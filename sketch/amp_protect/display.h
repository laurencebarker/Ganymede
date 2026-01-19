/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// display.h
// this file holds the code to control a Nextion 2.8" display
// it is 320x240 pixels
/////////////////////////////////////////////////////////////////////////

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <Nextion.h>                        // uses the Nextion class library
#include <Arduino.h>


//
// this type enumerates the Nextion display pages:
//
enum EDisplayPage
{
  eSplashPage,
  eRXPage,                                  // "normal" RX page display
  eTXPage,                                  // "normal" TX page display
  eTrippedPage,                             // page when h/w tripped
  eAboutPage,                               // about page
  eEngineeringPage                          // engineering (PIN) page
};



//
// display initialise
//
void DisplayInit(void);


//
// display tick
//
void DisplayTick(void);


//
// set time
// called when time increments, to allow it to be displayed
//
void DisplaySetOnTime(char* Str);


//
// set display page
//
void SetDisplayPage(EDisplayPage NewPage);

//
// set reset button text when ready to reset
//
void ActivateResetButton(bool AllowReset);


#endif //#ifndef
