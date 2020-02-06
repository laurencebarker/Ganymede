/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// analogueio.h
// this file holds the code to read the analogue inputs
/////////////////////////////////////////////////////////////////////////

#ifndef __ANALOGUEIO_H
#define __ANALOGUEIO_H



//
// AnalogueIO tick
// set comparator thresholds
//
void AnalogueIOTick(void);



//
// AnalogueIO initialise
// read the aDC values
//
void AnalogueIOInit(void);



//
// get temperature, as 1 dp fixed point integer
//
int GetTemperature(void);



//
// get PSU voltage, as 1 dp fixed point integer
//
unsigned int GetPSUVoltage(void);


//
// get drain current, as 1 dp fixed point integer
// zero should hasve been nulled out, but clips at zero
//
unsigned int GetCurrent(void);



//
// get forward power, as integer
//
unsigned int GetForwardPower(void);


//
// get reverse power, as integer
//
unsigned int GetReversePower(void);


//
// set zero current
// this is used to null out offset current: the ACS723 has a delivberate offset
//
void SetZeroCurrent(void);



#endif      // file sentry
