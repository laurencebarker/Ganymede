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

#ifndef __CONFIGDATA_H
#define __CONFIGDATA_H



//
// RAM storage of loaded settings
// these are loaded from FLASH after boot up
//
extern unsigned int GPin;                                  // 4 diit stored PIN

//
// function to copy all config settings to EEprom
//
void CopySettingsToEEprom(void);


//
// function to load config settings from EEprom
// initialises EEPROM if it hadn't already been initialised. 
//
void LoadSettingsFromEEprom(void);


//
// function to check EEprom is initialised, and load it if not
// called on start-up BEFORE loading the settings
//
void CheckEEpromInitialised(void);


#endif  //not defined
