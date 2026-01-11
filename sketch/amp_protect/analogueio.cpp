/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// analogueio.cpp
// this file holds the code to read the analogue inputs
/////////////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include "iopins.h"
#include "analogueio.h"
#include "protect.h"


//
// scaling factors to convert from ADC reading to sensed value
// note these are for true ISO units, not scaled by a factor of 10 for 1 decimapl place
// see spreadsheet for derivation
//
#define VFWDPOWERSCALE 0.002968131F         // convert ADC reading squared to forward watts
#define VREVPOWERSCALE 0.000745561F         // convert ADC reading squared to reverse watts
#define VCURRENTSCALE 0.069225749F          // convert ADC reading to current (includes potential divider)
#define VVOLTAGESCALE 0.076311849F          // convert ADC reading to PSU voltage

//
// comparator threshold outputs. See spreadsheet for derivation.
//
#define VCURRENTPWMTHRESHOLD  163           // 45A true current  
#define VVOLTAGEPWMTHRESHOLD  174           // 3.391V
#define VREVPOWERPWMTHRESHOLD 177           // 3.46V


//
// global variables
// sensor values, as 1DP fixed point integers
// (ie voltage 480 means 48.0 volts)
//
unsigned int GSensorTemperature;
unsigned int GSensorPSUVolts;             // 1DP
unsigned int GSensorCurrent;              // 1DP
unsigned int GSensorFwdPower;             // watts (not 1DP)
unsigned int GSensorRevPower;             // watts (not 1DP)
unsigned int GSensorFwdPowerPeak;
unsigned int GSensorRevPowerPeak;
int GSensorZeroCurrentRaw;               // ADC reading

//
// interpolate temperature measurements
// we have an array of point pairs fully covering the ADC input range, including 0 and 1023
//

struct STempData
{
  int ADCPoint;
  int Temp;
};



//
// lookup data for thermistor calibration
// derived from a spreadsheet
//
#define VNUMTEMPDATAPOINTS 11
STempData GTempDataArray[VNUMTEMPDATAPOINTS] =
{
  {0,-200},
  {100, 50},
  {182, 200},
  {298, 350},
  {435, 500},
  {573, 650},
  {693, 800},
  {814, 1000},
  {906, 1250},
  {949, 1450},
  {1023,1800}
};



//
// find a temperature for a given ADC reading
// this needs to read the table and find the points either side of the ADC reading
// result is 1DP fixed point (ie integer, 10x temp value in C)
//

int FindTemp(int SensorReading)
{
  byte Index;    // array index
  int FoundTemp;
  int Temp1, Temp2; // temps above and below the data point
  int ADC1, ADC2;       // ADC samples above and below the data point
  int DeltaTemp;
// check the extremes; if not an extreme, find points either side in the array
  if (SensorReading == 0)
    FoundTemp = GTempDataArray[0].Temp;
  else if (SensorReading == 1023)
    FoundTemp = GTempDataArray[VNUMTEMPDATAPOINTS -1].Temp;
  else          // find points and interpolate
  {
    for(Index=1; Index < VNUMTEMPDATAPOINTS; Index++)
      if(SensorReading < GTempDataArray[Index].ADCPoint)    // this will happen once!
      { 
        Temp2 = GTempDataArray[Index].Temp;
        Temp1 = GTempDataArray[Index-1].Temp;
        ADC2 = GTempDataArray[Index].ADCPoint;
        ADC1 = GTempDataArray[Index-1].ADCPoint;
        DeltaTemp = (Temp2-Temp1)*((128*(SensorReading-ADC1))/(ADC2-ADC1));
        FoundTemp = Temp1 + (DeltaTemp >> 7);
        break;
      }
  }
  return FoundTemp;
}


//
// set PWM comparamtor thresholds
// if paramter is true, set to max allowed (5V)
//
void SetPWMThresholds(bool SetToMax)
{
  if(SetToMax)
  {
    analogWrite(VPINCURRENTPWM, 255);
    analogWrite(VPINVOLTAGEPWM, 255);
    analogWrite(VPINREVPOWERPWM, 255);
  }
  else
  {
    analogWrite(VPINCURRENTPWM, VCURRENTPWMTHRESHOLD);
    analogWrite(VPINVOLTAGEPWM, VVOLTAGEPWMTHRESHOLD);
    analogWrite(VPINREVPOWERPWM, VREVPOWERPWMTHRESHOLD);
  }
}


//
// AnalogueIO initialise
// set the comparator threshold values
//
void AnalogueIOInit(void)
{
  SetPWMThresholds(false);
}




//
// AnalogueIO tick
// read the aDC values
//
void AnalogueIOTick(void)
{
  int SensorReading;
  float ScaledReading;

//
// current needs to have the zero offset removed, but noise could make it dip below 0A. Clip at 0A.
//
  SensorReading = analogRead(VPINCURRENTADC);                       // get ADC reading for current
  ScaledReading = (float)(SensorReading - GSensorZeroCurrentRaw) * VCURRENTSCALE * 10.0;      // 10x current
  if(ScaledReading > 0)
    GSensorCurrent = (unsigned int)ScaledReading;                   // store 10x current to variable
  else
    GSensorCurrent = 0;
    
  SensorReading = analogRead(VPINTEMPADC);                          // get ADC reading for temperature
  GSensorTemperature = (unsigned int)FindTemp(SensorReading);       // store 10x temp to variable

  SensorReading = analogRead(VPINVOLTAGEADC);                       // get ADC reading for PSU voltage
  ScaledReading = (float)SensorReading * VVOLTAGESCALE * 10.0;      // 10x voltage
  GSensorPSUVolts = (unsigned int)ScaledReading;                    // store 10x voltage to variable

  SensorReading = analogRead(VPINFWDPOWERADC);                      // get ADC reading for forward RF voltage
  ScaledReading = (float)SensorReading;                             // ADC
  ScaledReading = ScaledReading * ScaledReading * VFWDPOWERSCALE;   // V2/R for power
  GSensorFwdPower = (unsigned int)ScaledReading;                    // store forward power to variable (not fixed point!)
  if(GSensorFwdPower > GSensorFwdPowerPeak)                         // peak hold the value
    GSensorFwdPowerPeak = GSensorFwdPower;
    
  SensorReading = analogRead(VPINREVPOWERADC);                      // get ADC reading for reverse RF voltage
  ScaledReading = (float)SensorReading;                             // ADC
  ScaledReading = ScaledReading * ScaledReading * VREVPOWERSCALE;   // V2/R for power
  GSensorRevPower = (unsigned int)ScaledReading;                    // store reverse power to variable (not fixed point!)
  if(GSensorRevPower > GSensorRevPowerPeak)
    GSensorRevPowerPeak = GSensorRevPower;

  CheckTemperature(GSensorTemperature);
  CheckFwdPower(GSensorFwdPower);
}


//
// get temperature, as 1 dp fixed point integer
//
int GetTemperature(void)
{
  return GSensorTemperature;
}


//
// get PSU voltage, as 1 dp fixed point integer
//
unsigned int GetPSUVoltage(void)
{
  return GSensorPSUVolts;
}


//
// get drain current, as 1 dp fixed point integer
//
unsigned int GetCurrent(void)
{
  return GSensorCurrent;
}

//
// get forward power, as integer
//
unsigned int GetForwardPower(void)
{
  return GSensorFwdPower;
}

//
// get reverse power, as integer
//
unsigned int GetReversePower(void)
{
  return GSensorRevPower;
}


//
// get forward peak power, as integer
//
unsigned int GetForwardPeakPower(bool Clear)
{
  unsigned int Value;

  Value = GSensorFwdPowerPeak; 
  if(Clear)
    GSensorFwdPowerPeak = 0;
  return Value;
}

//
// get reverse power, as integer
//
unsigned int GetReversePeakPower(bool Clear)
{
  unsigned int Value;

  Value = GSensorRevPowerPeak; 
  if(Clear)
    GSensorRevPowerPeak = 0;
  return Value;  
}


//
// clear peak hold settings. Called when PTT pressed
//
void ClearPeakHolds(void)
{
    GSensorRevPowerPeak = 0;
    GSensorFwdPowerPeak = 0;  
}


//
// set zero current
// this is used to null out offset current: the ACS723 has a delivberate offset
// must be called when the current is zero!
//
void SetZeroCurrent(void)
{
  int SensorReading;
  float ScaledReading;

  GSensorZeroCurrentRaw = analogRead(VPINCURRENTADC);                       // get ADC reading for current
}
