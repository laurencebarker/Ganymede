/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// display.cpp
// this file holds the code to control a Nextion 2.8" display
// it is 320x240 pixels
/////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "display.h"
#include "stdio.h"
#include "globalinclude.h"
#include "analogueio.h"
#include "protect.h"



//
// define colours for  Nextion display
//
#define NEXBLACK 0L
#define NEXWHITE 65535L
#define NEXRED 63488L
#define NEXGREEN 2016L
#define NEXBLUE 31L

#define VHALFSECOND 50                        // 50 ticks per half second
#define VTENTHSECOND 10                       // 10 ticks per tenth of a second



EDisplayPage GDisplayPage;                    // global set to current display page number
byte GDisplayThrottleTicks;                   // number of clock ticks till next display object update
byte GDisplayData;                            // sets which object to update next


//
// declare pages:
//
NexPage page0 = NexPage(0, 0, "page0");       // creates touch event for "splash" page
NexPage page1 = NexPage(1, 0, "page1");       // creates touch event for "RX" page
NexPage page2 = NexPage(2, 0, "page2");       // creates touch event for "TX" page
NexPage page3 = NexPage(3, 0, "page3");       // creates touch event for "tripped" page
NexPage page4 = NexPage(4, 0, "page4");       // creates touch event for "about" page

//
// page 1 objects:
// 
NexText p1OnTime = NexText(1, 5, "p1t5");                   // on time
NexText p1HsTemp = NexText(1, 8, "p1t8");                   // heatsink temp

//
// page 2 objects:
// 
NexText p2HsTemp = NexText(2, 16, "p2t16");                 // heatsink temp
NexText p2Voltage = NexText(2, 17, "p2t17");                // PSU voltage
NexText p2Current = NexText(2, 18, "p2t18");                // drain current
NexText p2FwdPower = NexText(2, 19, "p2t19");               // forward power
NexText p2RevPower = NexText(2, 20, "p2t20");               // reverse power

//
// page 3 objects:
// 
NexText p3HsTemp = NexText(3, 13, "p3t13");                 // heatsink temp
NexText p3Voltage = NexText(3, 14, "p3t14");                // PSU voltage
NexText p3Current = NexText(3, 15, "p3t15");                // drain current
NexText p3FwdPower = NexText(3, 16, "p3t16");               // forward power
NexText p3RevPower = NexText(3, 17, "p3t17");               // reverse power
NexText p3HsTempLabel = NexText(3, 1, "p3t1");              // heatsink temp label
NexText p3VoltageLabel = NexText(3, 4, "p3t4");             // PSU voltage label
NexText p3CurrentLabel = NexText(3, 5, "p3t5");             // drain current label
NexText p3FwdPowerLabel = NexText(3, 6, "p3t6");            // forward power label
NexText p3RevPowerLabel = NexText(3, 7, "p3t7");            // reverse power label
NexButton p3Reset = NexButton(3, 2, "p3b2");                // RESET pushbutton

//
// page 4 objects:
// 
NexText p4SWVersion = NexText(4, 4, "p4t4");                // s/w version

//
// declare touch event objects to the touch event list
// this tells the code what touch events too look for
//
NexTouch *nex_listen_list[] = 
{
  &page0,                                     // page change 
  &page1,                                     // page change
  &page2,                                     // page change 
  &page3,                                     // page change
  &page4,                                     // page change 
  &p3Reset,                                   // RESET button press
  NULL                                        // terminates the list
};


#define VASCII0 0x30                // zero character in ASCII
//
// local version of "sprintf like" function
// Adds a decimal point before last digit if 3rd parameter set
// note integer value is signed and may be negative!
//
unsigned char mysprintf(char *dest, int Value, bool AddDP)
{
  unsigned char Digit;              // calculated digit
  bool HadADigit = false;           // true when found a non zero digit
  unsigned char DigitCount = 0;     // number of returned digits
  unsigned int Divisor = 10000;     // power of 10 being calculated
//
// deal with negative values first
//
  if (Value < 0)
  {
    *dest++ = '-';    // add to output
    DigitCount++;
    Value = -Value;
  }
//
// now convert the (definitely posirive) number
//
  while (Divisor >= 10)
  {
    Digit = Value / Divisor;        // find digit: integer divide
    if (Digit != 0)
      HadADigit = true;             // flag if non zero so all trailing digits added
    if (HadADigit)                  // if 1st non zero or all subsequent
    {
      *dest++ = Digit + VASCII0;    // add to output
      DigitCount++;
    }
    Value -= (Digit * Divisor);     // get remainder from divide
    Divisor = Divisor / 10;         // ready for next digit
  }
//
// if we need a decimal point, add it now. Also if there hasn't been a preceiding digit
// (i.e. number was like 0.3) add the zero
//
  if (AddDP)
  {
    if (HadADigit == false)
    {
      *dest++ = '0';
      DigitCount++;
    }
    *dest++ = '.';
  DigitCount++;
  }
  *dest++ = Value + VASCII0;
  DigitCount++;
//
// finally terminate with a 0
//
  *dest++ = 0;

  return DigitCount;
}




////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: PAGE change
//

//
// page 0 - splash page callback
//
void page0PushCallback(void *ptr)             // called when page 0 loads (splash page)
{
  Serial.println("page 0");
  GDisplayPage = eSplashPage;
}


//
// page 1 - RX page callback
//
void page1PushCallback(void *ptr)             // called when page 1 loads (RX page)
{
  Serial.println("page 1");
  GDisplayPage = eRXPage;
}

//
// page 2 - TX page callback
//
void page2PushCallback(void *ptr)             // called when page 2 loads (TX page)
{
  Serial.println("page 2");
  GDisplayPage = eTXPage;
}

//
// page 3 - Tripped page callback
// set page, then change object background colour
//
void page3PushCallback(void *ptr)             // called when page 3 loads (tripped page)
{
  Serial.println("page 3");
  GDisplayPage = eTrippedPage;
  if(GTripCause == eTripCurrent)
  {
    sendCommand("p3t5.bco=RED");
    sendCommand("ref p3t5");
  }
  else if(GTripCause == eTripPSUVoltage)
  {
    sendCommand("p3t4.bco=RED");
    sendCommand("ref p3t4");
  }
  else if(GTripCause == eTripTemperature)
  {
    sendCommand("p3t1.bco=RED");
    sendCommand("ref p3t1");
  }
  else if(GTripCause == eTripFwdPower)
  {
    sendCommand("p3t6.bco=RED");
    sendCommand("ref p3t6");
  }
  else if(GTripCause == eTripRevPower)
  {
    sendCommand("p3t7.bco=RED");
    sendCommand("ref p3t7");
  }
}

//
// page 4 - about page callback
//
void page4PushCallback(void *ptr)             // called when page 4 loads (about page)
{
  char Str[10];
  Serial.println("page 4");
  GDisplayPage = eAboutPage;
  mysprintf(Str, SWVERSION, false);
  p4SWVersion.setText(Str);
}


//
// touch event - RESET pushbutton on page 3
//
void p3ResetPushCallback(void *ptr)              // reset trips pushbutton
{
  DisplayResetPressed();
  Serial.println("reset press");
}



//
// display initialise
//
// nexinit performs the following:
//    dbSerialBegin(9600);
//    nexSerial.begin(Speed);
//    sendCommand("");
//    sendCommand("bkcmd=1");
//    ret1 = recvRetCommandFinished();
//    sendCommand("page 0");
//    ret2 = recvRetCommandFinished();
//    return ret1 && ret2;
//

void DisplayInit(void)
{
//
// set baud rate & register event callback functions
//  
  nexInit(115200);
  page0.attachPush(page0PushCallback);
  page1.attachPush(page1PushCallback);
  page2.attachPush(page2PushCallback);
  page3.attachPush(page3PushCallback);
  page4.attachPush(page4PushCallback);
  p3Reset.attachPush(p3ResetPushCallback);
}



//
// display tick
//
void DisplayTick(void)
{
  char Str[20];
//
// handle touch display events
//  
  nexLoop(nex_listen_list);

//
// display dependent processing
//
  switch(GDisplayPage)
  {
    case  eSplashPage:                              // startup splash - nothing to add to display
      break;

      
    case eRXPage:                                   // "normal" RX page display
      if(GDisplayThrottleTicks == 0)                // update display if timed out
      {
        mysprintf(Str, GetTemperature()/10, false);   // temp in C
        p1HsTemp.setText(Str);
        GDisplayThrottleTicks = VHALFSECOND;
      }
      else
        GDisplayThrottleTicks--;
      break;


    case eTXPage:                                         // "normal" TX page display
      if(GDisplayThrottleTicks == 0)                      // update display if timed out
      {
        switch(GDisplayData)
        {
          case 0:                                         // display temperature
            mysprintf(Str, GetTemperature()/10, false);   // temp in C
            p2HsTemp.setText(Str);
            break;
          case 1:                                         // display PSU voltage   
            mysprintf(Str, GetPSUVoltage()/10, false);    // voltage in whole volts
            p2Voltage.setText(Str);
            break;
          case 2:                                         // display current
            mysprintf(Str, GetCurrent(), false);          // current in fractional A
            p2Current.setText(Str);
            break;
          case 3:                                         // display forward power
            mysprintf(Str, GetForwardPower(), false);     // forward power in W
            p2FwdPower.setText(Str);
            break;
          case 4:                                         // display reverse power
            mysprintf(Str, GetReversePower(), false);     // reverse power in W
            p2RevPower.setText(Str);
            break;
        }
        GDisplayThrottleTicks = VTENTHSECOND;
        if (GDisplayData == 4)                      // and set up to display next object
          GDisplayData=0;
        else
          GDisplayData++;
      }
      else
        GDisplayThrottleTicks--;
      break;


    case eTrippedPage:                             // page when h/w tripped
      if(GDisplayThrottleTicks == 0)                      // update display if timed out
      {
        switch(GDisplayData)
        {
          case 0:                                         // display temperature
            mysprintf(Str, GetTemperature()/10, false);   // temp in C
            p3HsTemp.setText(Str);
            break;
          case 1:                                         // display PSU voltage   
            mysprintf(Str, GetPSUVoltage()/10, false);    // voltage in whole volts
            p3Voltage.setText(Str);
            break;
          case 2:                                         // display current
            mysprintf(Str, GetCurrent(), false);          // current in fractional A
            p3Current.setText(Str);
            break;
          case 3:                                         // display forward power
            mysprintf(Str, GetForwardPower(), false);     // forward power in W
            p3FwdPower.setText(Str);
            break;
          case 4:                                         // display reverse power
            mysprintf(Str, GetReversePower(), false);     // reverse power in W
            p3RevPower.setText(Str);
            break;
        }
        GDisplayThrottleTicks = VTENTHSECOND;
        if (GDisplayData == 4)                      // and set up to display next object
          GDisplayData=0;
        else
          GDisplayData++;
      }
      else
        GDisplayThrottleTicks--;
      break;

    
    case eAboutPage:                                // about page

      break;

      
  }

}


//
// set time
// called when time increments, to allow it to be displayed
//
void DisplaySetOnTime(char* Str)
{
  if(GDisplayPage == eRXPage)
    p1OnTime.setText(Str);
}


//
// set display page
//
void SetDisplayPage(EDisplayPage NewPage)
{
  GDisplayPage = NewPage;
  GDisplayThrottleTicks = VHALFSECOND;
  GDisplayData = 0;
  switch (NewPage)
  {
    case  eSplashPage:
      page0.show();
      break;

    case eRXPage:                                  // "normal" RX page display
      page1.show();
      break;

    case eTXPage:                                  // "normal" TX page display
      page2.show();
      break;
      
    case eTrippedPage:                             // page when h/w tripped
      page3.show();
      break;

    case eAboutPage:                                // about page
      page4.show();
      break;
  }
}

//
// set reset button text whwn ready to reset
//
void ActivateResetButton(void)
{
  p3Reset.setText("RESET");
}
