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


#define VLINEVOLTSCALE 0.1953F              // convert ADC reading to volts
#define VTEMPSCALE 0.1172F                  // convert ADC reading to temperature
#define VCURRENTSCALE 0.0195F               // convert ADC reading to current
#define VVOLTAGESCALE 0.0586                // convert ADC reading to PSU voltage

//
// comparator threshold outputs. See spreadsheet for derivation.
//
#define VCURRENTPWMTHRESHOLD  102           // DEBUG 2V  
#define VVOLTAGEPWMTHRESHOLD  156           // DEBUG 3V
#define VREVPOWERPWMTHRESHOLD 156           // DEBUG 3V


//
// global variables
// sensor values, as 1DP fixed point integers
//
unsigned int GSensorTemperature;
unsigned int GSensorPSUVolts;
unsigned int GSensorCurrent;
unsigned int GSensorFwdPower;
unsigned int GSensorRevPower;

//
// AnalogueIO initialise
// set the comparator threshold values
//
void AnalogueIOInit(void)
{
  analogWrite(VPINCURRENTPWM, VCURRENTPWMTHRESHOLD);
  analogWrite(VPINVOLTAGEPWM, VVOLTAGEPWMTHRESHOLD);
  analogWrite(VPINREVPOWERPWM, VREVPOWERPWMTHRESHOLD);
}




//
// AnalogueIO tick
// read the aDC values
//
void AnalogueIOTick(void)
{
  int SensorReading;
  float ScaledReading;

  SensorReading = analogRead(VPINCURRENTADC);                       // get ADC reading for current
  ScaledReading = (float)SensorReading * VCURRENTSCALE * 10.0;      // 10x current
  GSensorCurrent = (unsigned int)ScaledReading;                     // store 10x current to variable

  SensorReading = analogRead(VPINTEMPADC);                        // get ADC reading for temperature
  ScaledReading = (float)SensorReading * VTEMPSCALE * 10.0;       // 10x temp
  GSensorTemperature = (unsigned int)ScaledReading;               // store 10x temp to variable

  SensorReading = analogRead(VPINVOLTAGEADC);                       // get ADC reading for PSU voltage
  ScaledReading = (float)SensorReading * VVOLTAGESCALE * 10.0;      // 10x voltage
  GSensorPSUVolts = (unsigned int)ScaledReading;                    // store 10x voltage to variable

  SensorReading = analogRead(VPINFWDPOWERADC);                      // get ADC reading for forward RF voltage
  ScaledReading = (float)SensorReading * VLINEVOLTSCALE;            // line voltage
  ScaledReading = ScaledReading * ScaledReading * 0.02;             // V2/R for power
  GSensorFwdPower = (unsigned int)ScaledReading;                    // store forward power to variable (not fixed point!)

  SensorReading = analogRead(VPINREVPOWERADC);                      // get ADC reading for reverse RF voltage
  ScaledReading = (float)SensorReading * VLINEVOLTSCALE;            // line voltage
  ScaledReading = ScaledReading * ScaledReading * 0.02;             // V2/R for power
  GSensorRevPower = (unsigned int)ScaledReading;                    // store reverse power to variable (not fixed point!)

  CheckTemperature(GSensorTemperature);
  CheckFwdPower(GSensorFwdPower);
}


//
// get temperature, as 1 dp fixed point integer
//
unsigned int GetTemperature(void)
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
