/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// configdata.h
// this file holds the code to save and load settings to/from EEPROM
/////////////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include "globalinclude.h"

#include <EEPROM.h>

#define VEEINITPATTERN 0x10                     // addr 0 set to this if configured

unsigned int GPin;                              // 4 diit stored PIN


//
// function to copy all config settings to EEprom
// this copies the current RAM vaiables to the persistent storage
// addr 0: defined pattern (to know EEPROM has been initialised)
// addr 1: normal encoder events per step
// addr 2: VFO encoder events per steo
// addr 3: display brightness
//
void CopySettingsToEEprom(void)
{
  int Addr=1;
  byte Setting;
  int Cntr;
  
//
// first set that we have initialised the EEprom
//
  EEPROM.write(0, VEEINITPATTERN);
//
// now copy settings from RAM data
//
  EEPROM.put(1, GPin);
}



//
// function to copy initial settings to EEprom
// this sets the factory defaults
// the settings here should match the fornt panel legend!
//
void InitialiseEEprom(void)
{
  int Cntr;
  
  GPin = 0;                           // initialise stored PIN to zero

// now copy them to FLASH
  CopySettingsToEEprom();
}



//
// function to load config settings from flash
//
void LoadSettingsFromEEprom(void)
{
  int Addr=1;
  byte Setting;
  int Cntr;

//
// first see if we have initialised the EEprom previously
// if not, copy settings to it
//
  Setting = EEPROM.read(0);
  if (Setting != VEEINITPATTERN)
    InitialiseEEprom();
//
// now copy out settings to RAM data
//
  EEPROM.get(Addr, GPin);
}


