/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// the code is written for an Arduino Nano Every module
//
// ontime.h: header for "on time" recording
/////////////////////////////////////////////////////////////////////////

#ifndef __ONTIME_H
#define __ONTIME_H




//
// initialise - simply initislise the structures
//
void OnTimeInit(void);


//
// once per second tick - increment time value
//
void TimeSecondTick(void);




#endif //__ONTIME_H
