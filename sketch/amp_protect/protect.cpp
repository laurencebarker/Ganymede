/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// the code is written for an Arduino Nano Every module
//
// protect.cpp: amplifier protection logic
/////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "display.h"
#include "protect.h"
#include "iopins.h"
#include "cathandler.h"
#include "analogueio.h"


//
// this type enumerates the ptotection states:
//
enum EProtectionState
{
  eNotInitialised,                      // after power up
  eRX,                                  // "normal" RX 
  eTX,                                  // "normal" TX 
  eTripped,                             // tripped, not yet reset
  eTripResetPressed                     // after reset pressed, exiting trip
};



#define VINITCOUNT 550                  // 5.5 seconds count
#define VCOMPARATORPINS 3               // pins PE00 & PE01

//
// global variables
//
EProtectionState GProtectionState;      // sequencer state variable
int GInitialiseCounter;                 // counts down 5s after startup
ETripCause GTripCause;                  // reason for trip
bool GTempResettable;                   // if true can't reset until temp lowered
bool GPowerResettable;                  // if true can't reset until forward power lowered
bool GResettable;                       // true if radio will allow a RESET button press
bool GResetActivated;                   // true if reset button has been activated

#define VTRIPTEMPTHRESHOLD 900          // 90C to trip
#define VTTRIPTEMPREENABLE 500          // re-enable at 50C
#define VFANONTHRESHOLD 400             // fan on at 40C
#define VFANOFFTHRESHOLD 300            // off at 30C
#define VTRIPFWDPOWERTHRESHOLD 600      // forward power trip, in watts
#define VTTRIPPOWERREENABLE 40          // forward power re-enable level, in watts (should be 0 if tripped anyway)



void CurrentComparatorHandler(void)
{
  GTripCause = eTripCurrent;
// deassert enable outputs
  digitalWrite(VPINAMPENABLE, LOW);
  digitalWrite(VPINPSUENABLE, LOW);
}


void VoltageComparatorHandler(void)
{
  GTripCause = eTripPSUVoltage;
// deassert enable outputs
  digitalWrite(VPINAMPENABLE, LOW);
  digitalWrite(VPINPSUENABLE, LOW);
}

void SRFlipFlopTripHandler(void)
{
  GTripCause = eTripRevPower;
// deassert enable outputs
  digitalWrite(VPINAMPENABLE, LOW);
  digitalWrite(VPINPSUENABLE, LOW);
}



//
// protect initialise
// startup logic. This is called after the analogue thresholds are driven out.
//
void ProtectInit(void)
{
  GProtectionState = eNotInitialised;
  GInitialiseCounter = VINITCOUNT;
  GTripCause = eNoTrip;
  delay(500);                                 // 0.5s delay to give time for comparator thresholds to settle
//
// cycle the SR flip flop reset (it could start up in either state)
// then check the hardware trip conditions to see if OK to turn on interrupts
//
  digitalWrite(VPINSRRESET, HIGH);            // reset the flip flop
  digitalWrite(VPINSRRESET, LOW);
  if(digitalRead(VPINCURRENTCOMP)==HIGH)      // check if current already over limit
    GTripCause = eTripCurrent;
  if(digitalRead(VPINVOLTAGECOMP)==HIGH)      // check if PSU voltage already over limit
    GTripCause = eTripPSUVoltage;
  if(digitalRead(VPINREVPOWERSR)==LOW)        // check if flip flop for rev power already over limit
    GTripCause = eTripRevPower;
//
// while PSU still off, set the "zero" current reading (there is a deliberate 0.5V bias from ACS723)
// and attach interrupts
//
  SetZeroCurrent();                         // read zero while drain supply still off
  attachInterrupt(digitalPinToInterrupt(VPINCURRENTCOMP), CurrentComparatorHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(VPINVOLTAGECOMP), VoltageComparatorHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(VPINREVPOWERSR), SRFlipFlopTripHandler, FALLING);

//
// if there is a trip cause, just wait for now: the cause is set and will enter tripped in the sequencer
//
  if(GTripCause == eNoTrip)                   // OK to power up
  {
    digitalWrite(VPINPSUENABLE, HIGH);        // turn on PSU
  }
}


//
// handle "press" of the display reset button
//
void DisplayResetPressed(void)
{
  if (GResettable)                              // if we meet the conditions - begin reset
  {
    SetZeroCurrent();                           // re-read zero while drain supply still off
    GTripCause = eNoTrip;
    GProtectionState = eTripResetPressed;
    digitalWrite(VPINPSUENABLE, HIGH);          // turn PSU back on
  }
}


//
// forward power check (called from h/w driver)
// parameter is forward power, integer
// note forward power trip condition removed temporarily. Radio was tripping on TX when drain current in spec
//
void CheckFwdPower(int Power)
{
//  if (Power > VTRIPFWDPOWERTHRESHOLD)
//  {
//    if (GTripCause == eNoTrip)
//      GTripCause = eTripFwdPower;
// deassert enable outputs
//      digitalWrite(VPINAMPENABLE, LOW);
//      digitalWrite(VPINPSUENABLE, LOW);
//      GPowerResettable = false;        // can't reset for temp
// }
//  else if (Power < VTTRIPPOWERREENABLE)
//    GPowerResettable = true;
  if (Power < VTTRIPPOWERREENABLE)
    GPowerResettable = true;
}



//
// temperature check (called from h/w driver)
// parameter is temperature, 1dp fixed point (ie 10x temp)
// check whether we have tripped, or can exit trip; then check fan setting
//
void CheckTemperature(int Temperature)
{
  if (Temperature > VTRIPTEMPTHRESHOLD)
  {
    if (GTripCause == eNoTrip)
      GTripCause = eTripTemperature;
// deassert enable outputs
      digitalWrite(VPINAMPENABLE, LOW);
      digitalWrite(VPINPSUENABLE, LOW);
      GTempResettable = false;        // can't reset for temp
  }
  else if (Temperature < VTTRIPTEMPREENABLE)
    GTempResettable = true;

  if (Temperature > VFANONTHRESHOLD)
    digitalWrite(VPINFAN, HIGH);
  else if (Temperature < VFANOFFTHRESHOLD)
    digitalWrite(VPINFAN, LOW);
}



//
// 10ms tick code
// execute sequencers for protection logic.
//   to relase from initialise;
//   to handle TX/RX;
//   to handle trip;
//   to handle trip reset.
// 
void ProtectTick(void)
{
  bool PTTPressed = false;                      // true if PTT pressed


  if (digitalRead(VPINPTT)==HIGH)               // first read the PTT
    PTTPressed = true;
//
// first see if it has been tripped (eg excessive temperature) - 
// 
  if ((GTripCause != eNoTrip) && (GProtectionState != eTripped))
  {
    GProtectionState = eTripped;                  // set new state
    MakeAmplifierTripMessage(GTripCause);         // send CAT message
    SetDisplayPage(eTrippedPage);
    GResetActivated = false;                      // reset button not activated
  }
//
// now step through the sequencer, noting transitions to "tripped" already done
//
  switch (GProtectionState)
  {
    case eNotInitialised:                         // after power up
      if (GInitialiseCounter != 0)                // decrement counter if not finished
        GInitialiseCounter--;

      if ((GInitialiseCounter==0) && !PTTPressed) // move to RX if timer expired and not PTT
      {
        GProtectionState = eRX;
        digitalWrite(VPINAMPENABLE, HIGH);
        SetDisplayPage(eRXPage);
      } 
      break;
      
    case eRX:                                     // "normal" RX. Check if TX started
      if (PTTPressed)
      {
        GProtectionState = eTX;
        SetDisplayPage(eTXPage);
      }
      break;
      
    case eTX:                                     // "normal" TX. Check if TX ended
      if (!PTTPressed)
      {
        GProtectionState = eRX;
        SetDisplayPage(eRXPage);
      }
      break;
      
    case eTripped:                                // tripped, not yet reset
//
// check conditions for "resettable"
// if "resettable" make the RESET button work. 
// temp, power and the 2 comparator inputs must all be OK. 
// Cycle the SR flip flop reset before testing it 
//
      GResettable = true;                           // assume we can re-enable
      if (GTempResettable == false)
        GResettable = false;
      if (GPowerResettable == false)
        GResettable = false;
      if(digitalRead(VPINCURRENTCOMP) == HIGH)
        GResettable = false;
      if(digitalRead(VPINVOLTAGECOMP) == HIGH)
        GResettable = false;
      digitalWrite(VPINSRRESET, HIGH);                // try SR flip flop reset
      digitalWrite(VPINSRRESET, LOW);
      if(digitalRead(VPINREVPOWERSR) == LOW)
        GResettable = false;
      if (PTTPressed)
        GResettable = false;
//
// we now know if it is OK to enable the "reset" button
//
      if ((GResettable == true) && (GResetActivated == false))
      {
        GResetActivated = true; 
        ActivateResetButton();                        // change text on reset button when ready
      }
      break;

    case eTripResetPressed:                       // complete the restoration of normal operation
      GProtectionState = eRX;
      MakeAmplifierTripMessage(GTripCause);         // send CAT message
      digitalWrite(VPINAMPENABLE, HIGH);          // turn amplifier back on
      SetDisplayPage(eRXPage);
      break;
  }
}
