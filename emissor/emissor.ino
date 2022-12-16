#include <Manchester.h>
#include "Arduino.h"

#define TX_PIN  2
#define led 0
#define bed 2
#define t 500

bool envoi = true;

const byte interruptPin = 4;

void setup() {
  pinMode(interruptPin, INPUT_PULLUP); // set to input with an internal pullup resistor (HIGH when switch is open)
  pinMode(3, INPUT_PULLUP);
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
  if(envoi==true){
    int m = bed;

    for(int i=0;i<20;i++)
    {
      digitalWrite(led,HIGH);
      man.transmit(m);
      delay(t);
      digitalWrite(led,LOW);
      delay(t);
    }


    int count = 0;
    int count10000 = 0;

    
    while((digitalRead(3)>0) && count10000 < 90) // count10000 < 90 --> tempo 20 min para desligar
    {
      count++;
      delay(1);
      if(count >= 10000)
      {
        count = 0;
        count10000++;
      }
      
      if(digitalRead(4)==0){

        for(int i=0;i<20;i++)
        {
          digitalWrite(led,HIGH);
          man.transmit(m);
          delay(t);
          digitalWrite(led,LOW);
          delay(t);
        }
      }
    }
    
    
    m = m + 50;// messagem para desligar

    for(int i=0;i<20;i++)
    {
      digitalWrite(led,HIGH);
      man.transmit(m);
      delay(t);
      digitalWrite(led,LOW);
      delay(t);
    }
  }
  envoi = !envoi;
}