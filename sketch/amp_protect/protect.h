/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// the code is written for an Arduino Nano Every module
//
// protect.h: amplifier protection logic
/////////////////////////////////////////////////////////////////////////

#ifndef __protect_h
#define __protect_h

#include <Arduino.h>
#include "display.h"


//
// enum for the cause of amplifier trip
//
enum ETripCause
{
  eNoTrip,                              // not tripped
  eTripCurrent,                         // excess current
  eTripPSUVoltage,                      // input PSU over threshold
  eTripTemperature,                     // temp outside limits
  eTripFwdPower,                        // excessive forward power
  eTripRevPower                         // excessive reverse power
};

extern ETripCause GTripCause;                  // reason for trip
extern bool GProtectionEnforced;               // true if protection is enforced
extern bool GResetActivated;                   // true if reset button has been activated



//
// protect initialise
// startup logic
//
void ProtectInit(void);


//
// temperature check (called from h/w driver)
// parameter is temperature, 1dp fixed point (ie 10x temp)
//
void CheckTemperature(int Temp);


//
// forward power check (called from h/w driver)
// parameter is forward power, integer
//
void CheckFwdPower(int Power);


//
// handle "press" of the display reset button
//
void DisplayResetPressed(void);




//
// 10ms tick code
// execute sequencers for protection logic.
//   to relase from initialise;
//   to handle TX/RX;
//   to handle trip;
//   to handle trip reset.
// 
void ProtectTick(void);

//
// enable/disable protection enforcement
//
void EnforceProtection(bool IsEnforced);


#endif      // file sentry
