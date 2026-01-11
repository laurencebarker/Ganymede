/////////////////////////////////////////////////////////////////////////
//
// Aries ATU controller sketch by Laurence Barker G8NJJ
// this sketch controls an L-match ATU network
// with a CAT interface to connect to an HPSDR control program
// copyright (c) Laurence Barker G8NJJ 2019
//
// the code is written for an Arduino Nano Every module
//
// globalinclude.h: global include definitions
/////////////////////////////////////////////////////////////////////////
#ifndef __globalinclude_h
#define __globalinclude_h


//
// hardware and software version: send back to console on request
// product iD: send back to console on request
// 1=Andromeda front panel
// 2 = Aries ATU
// 3 = Ganymede
// 4 = G2V1 panel (no Arduino though)
// 5 = G2V2 panel
// 6 = p2app
// 7 = Saturn f/w
////



#define SWVERSION 5
#define HWVERSION 1
#define PRODUCTID 3                 // Ganymede



#endif      // file sentry
