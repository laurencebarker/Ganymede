/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// CAT handler.h
// this file holds the CAT handling code
/////////////////////////////////////////////////////////////////////////
#ifndef __cathandler_h
#define __cathandler_h
#include <Arduino.h>
#include "tiger.h"
#include "protect.h"



extern long Gp2appVersion;                     // radio s/w version for Saturn
extern long GFirmwareVersion;                  // radio firmware version




#define VREVPOWERTRIP 1
#define VCURRENTTRIP 2
#define VVOLTSGETRIP 4
#define VTEMPTRIP 8
//
// function to send back a product ID message
// Data holds the trip condition
// 0: no trip; 1: reverse power trip; 2: drain current trip; 4: PSU voltsge trip; 8: heatsink temperature trip
// 16: high forward power trip
// 64: can be reset
//
void MakeAmplifierTripMessage(ETripCause Data, bool CanReset);




//
// handlers for received CAT commands
//
void HandleCATCommandNumParam(ECATCommands MatchedCAT, long ParsedParam);
void HandleCATCommandNoParam(ECATCommands MatchedCAT);
void HandleCATCommandBoolParam(ECATCommands MatchedCAT, bool ParsedBool);
void HandleCATCommandStringParam(ECATCommands MatchedCAT, char* ParsedParam);


#endif //not defined
