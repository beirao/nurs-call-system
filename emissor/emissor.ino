#include <Manchester.h>
#include "Arduino.h"

const byte interruptPin = 4;


#define TX_PIN  2
#define led 0


void setup() {
  pinMode(interruptPin, INPUT_PULLUP); // set to input with an internal pullup resistor (HIGH when switch is open)
  pinMode(led,OUTPUT);
  
  man.setupTransmit(TX_PIN, MAN_1200);

  // interrupts
  PCMSK  |= bit (PCINT4);  // want pin D4 / pin 3
  GIFR   |= bit (PCIF);    // clear any outstanding interrupts
  GIMSK  |= bit (PCIE);    // enable pin change interrupts 
}

void loop() {

}

ISR (PCINT0_vect) // this is the Interrupt Service Routine
{
  int m=3;
  for(int i=0;i<10;i++)
  {
    digitalWrite(led,HIGH);
    man.transmit(m);
    delay(1000);
    digitalWrite(led,LOW);
    delay(1000);
  }
}
