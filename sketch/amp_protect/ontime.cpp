/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// the code is written for an Arduino Nano Every module
//
// ontime.c: "on time" recording
/////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <stdlib.h>
#include "ontime.h"
#include "display.h"


//
// global variables
//
byte GSeconds;
byte GMinutes;
int GHours;


//
// initialise - simply initislise the structures
//
void OnTimeInit(void)
{
  GSeconds = 0;
  GMinutes = 0;
  GHours = 0;
}


//
// once per second tick - increment time value
//
void TimeSecondTick(void)
{
  char Str[20];
  byte Pos;
  GSeconds ++;
  if (GSeconds >= 60)
  {
    GSeconds = 0;
    GMinutes++;
    if (GMinutes >= 60)
    {
      GMinutes = 0;
      GHours++;
    }
  }
//
// convert to text and send to display
//
  Str[0] = 0;
  itoa(GHours, Str, 10);                  // send the hours to string
  Pos=strlen(Str);                        // find out where we are
  Str[Pos++] = ':';
  Str[Pos++] = (GMinutes / 10)+0x30;      // add tens digit of minutes
  Str[Pos++] = (GMinutes % 10)+0x30;      // add units digit of minutes
  Str[Pos++] = ':';
  Str[Pos++] = (GSeconds / 10)+0x30;      // add tens digit of seconds
  Str[Pos++] = (GSeconds % 10)+0x30;      // add units digit of seconds
  Str[Pos++] = 0;
  DisplaySetOnTime(Str);
}
