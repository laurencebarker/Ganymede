/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// CAT handler.cpp
// this file holds the CAT handling code
// responds to parsed messages, and initiates message sends
// this is the main body of the program!
/////////////////////////////////////////////////////////////////////////

#include "globalinclude.h"
#include "cathandler.h"
#include <stdlib.h>


//
// clip to numerical limits allowed for a given message type
//
int ClipParameter(int Param, ECATCommands Cmd)
{
  SCATCommands* StructPtr;

  StructPtr = GCATCommands + (int)Cmd;
//
// clip the parameter to the allowed numeric range
//
  if (Param > StructPtr->MaxParamValue)
    Param = StructPtr->MaxParamValue;
  else if (Param < StructPtr->MinParamValue)
    Param = StructPtr->MinParamValue;
  return Param;  
}




//
// function to send back a software version message
//
void MakeSoftwareVersionMessage(void)
{
  long Version;
  Version = (PRODUCTID * 100000) + (HWVERSION*1000) + SWVERSION;
  
//  Version = SWVERSION;
  MakeCATMessageNumeric(eZZZS,Version);
}




//
// function to send back a product ID message
// Data holds the trip condition
// 0: no trip; 1: reverse power trip; 2: drain current trip; 4: PSU voltsge trip; 8: heatsink temperature trip
//
void MakeAmplifierTripMessage(ETripCause Data)
{
  byte Value;

  switch(Data)
  {
    case eNoTrip:                              // not tripped
      Value = 0; break;
    case eTripCurrent:                         // excess current
      Value = 2; break;
    case eTripPSUVoltage:                      // input PSU over threshold
      Value = 4; break;
    case eTripTemperature:                     // temp outside limits
      Value = 8; break;
    case eTripFwdPower:                        // excessive forward power
      Value = 16; break;
    case eTripRevPower:                        // excessive reverse power
      Value = 1; break;
  }
  
  MakeCATMessageNumeric(eZZZA,Value);
}





//
// handle a trip message from PC
// the only one we recognise is param=32 meaning "reset the trip condition"
//
HandleAmplifierTripMessage(int Param)
{
  if(Param == 32)
    DisplayResetPressed();
}





//
// handle CAT commands with numerical parameters
//
void HandleCATCommandNumParam(ECATCommands MatchedCAT, int ParsedParam)
{
  int Device;
  byte Param;
  bool State = false;
  
  switch(MatchedCAT)
  {
    case eZZZA:                                                       // amplifier trip reset
      HandleAmplifierTripMessage(ParsedParam);
      break;
  }
}


//
// handle CAT commands with no parameters
//
void HandleCATCommandNoParam(ECATCommands MatchedCAT)
{
  switch(MatchedCAT)
  {
    case eZZZS:                                                       // s/w version reply
      MakeSoftwareVersionMessage();
      break;
    case eZZZA:                                                       // amplifier trip request
      MakeAmplifierTripMessage(GTripCause);
    
  }
}


//
// handle CAT commands with boolean parameter
//
void HandleCATCommandBoolParam(ECATCommands MatchedCAT, bool ParsedBool)
{
  switch(MatchedCAT)
  {
  }
}


//
// handle CAT commands with string parameter
//
void HandleCATCommandStringParam(ECATCommands MatchedCAT, char* ParsedParam)
{
  switch(MatchedCAT)
  {
  }
}
