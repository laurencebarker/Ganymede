/////////////////////////////////////////////////////////////////////////
//
// Amplifier protection code by Laurence Barker G8NJJ
// copyright (c) Laurence Barker G8NJJ 2019
//
// this sketch provides a control mechanism for an LDMOS amplifier
//
// the code is written for an Arduino Nano Every module
//
// "main" file with setup() and loop()
/////////////////////////////////////////////////////////////////////////


#include "display.h"
#include "ontime.h"
#include "analogueio.h"
#include "tiger.h"
#include "cathandler.h"
#include "iopins.h"
#include "protect.h"
#include "configdata.h"


//
// global variables
//
bool GTickTriggered;                  // true if a 16ms tick has been triggered

bool ledOn = false;                   // for heartbeat LED:
byte Counter = 0;                     // for heartbeat LED:


//
// counter clocked by CK/64 (4us)
//
void SetupTimerForInterrupt(int Milliseconds)
{
  int Count;

  Count = 250* Milliseconds;                // temp value - not sure why not 250!
  TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Use timer compare mode  
  TCB0.CCMP = Count; // Value to compare with. This is 1/5th of the tick rate, so 5 Hz
  TCB0.INTCTRL = TCB_CAPT_bm; // Enable the interrupt
  TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm; // Use Timer A as clock, enable timer
}




void setup() 
{
//
// initialise timer to give 10ms tick interrupt
//
  SetupTimerForInterrupt(10);                                      // 10ms tick
  Serial.begin(9600);
  delay(1000);
  ConfigIOPins();
  LoadSettingsFromEEprom();

  AnalogueIOInit();
  DisplayInit();
  OnTimeInit();                                                   // initialise "on time" variables
//
// initialise CAT handler
//
  InitCAT();

  ProtectInit();
}


//
// periodic timer tick handler.
//
ISR(TCB0_INT_vect)
{
  GTickTriggered = true;
   // Clear interrupt flag
  TCB0.INTFLAGS = TCB_CAPT_bm;
}



//
// sits and waits for time tick to have happened
//
void loop() 
{
  while (GTickTriggered)
  {
    GTickTriggered = false;
// heartbeat LED
    if (Counter == 0)
    {
      Counter=49;
      ledOn = !ledOn;
      if (ledOn)
      {
        TimeSecondTick();
        digitalWrite(LED_BUILTIN, HIGH); // Led on, off, on, off...
      }
       else
        digitalWrite(LED_BUILTIN, LOW);
    }
    else
      Counter--;
//
// get analogue values
//
    AnalogueIOTick();
//
// look for any CAT commands in the serial input buffer and process them
//    
    ScanParseSerial();

//
// update protection logic
//
    ProtectTick();

//
// display update
//
    
    DisplayTick();

  }

}


//
// set pinmode for all I/O pins used
// and write any output initial states
//
void ConfigIOPins(void)
{
  pinMode(LED_BUILTIN, OUTPUT);                         // LED output
  pinMode(VPINPTT, INPUT_PULLUP);                       // PTT input
  pinMode(VPINCURRENTPWM, OUTPUT);                      // PWM output
  pinMode(VPINVOLTAGEPWM, OUTPUT);                      // PWM output
  pinMode(VPINREVPOWERPWM, OUTPUT);                     // PWM output
  pinMode(VPINCURRENTCOMP, INPUT_PULLUP);               // comparator
  pinMode(VPINVOLTAGECOMP, INPUT_PULLUP);               // comparator
  pinMode(VPINREVPOWERSR, INPUT_PULLUP);                // comparator
  pinMode(VPINSRRESET, OUTPUT);                         // reset output
//
// and set state BEFORE these are enabled as output
//
  digitalWrite(VPINFAN, LOW);
  digitalWrite(VPINAMPENABLE, LOW);
  digitalWrite(VPINPSUENABLE, LOW);
  pinMode(VPINFAN, OUTPUT);                             // FAN control output
  pinMode(VPINAMPENABLE, OUTPUT);                       // amplifier enable output (bias, PTT)
  pinMode(VPINPSUENABLE, OUTPUT);                       // PSU enable output
}
