/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// iopins.h
// this file lists all the I/O pins in the project
/////////////////////////////////////////////////////////////////////////

#ifndef __IOPINS_H
#define __IOPINS_H

#define VPINCURRENTADC A0         // current measurement input
#define VPINTEMPADC A1            // temp measurement input
#define VPINVOLTAGEADC A2         // PSU voltage measurement
#define VPINFWDPOWERADC A6        // analogue input
#define VPINREVPOWERADC A3        // analogue input

#define VPINCURRENTPWM 3          // threshold DAC
#define VPINVOLTAGEPWM 5          // threshold DAC
#define VPINREVPOWERPWM 9         // threshold DAC

#define VPINCURRENTCOMP 11        // comparator input (PE00) high if tripped
#define VPINVOLTAGECOMP 12        // comparator input for PSU voltage (PE01) high if tripped
#define VPINREVPOWERSR 8          // SR flip flop output triggered by reverse power (PE03) high when OK, low if tripped

#define VPINSRRESET  4            // reset output to flip flop
#define VPINAMPENABLE 7           // enable output for PTT and bias
#define VPINPSUENABLE 2           // power supply enable
#define VPINFAN 6                 // fan output (may not be needed)
#define VPINPTT 10                // PTT input. 1 = TX.


#endif //not defined
