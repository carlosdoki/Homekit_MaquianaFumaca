#include "extras/PwmPin.h"                          // library of various PWM functions

struct DEV_Switch : Service::Switch {       // Dimmable LED

  int pinMode;
  SpanCharacteristic *power;    

  DEV_Switch(int pin) : Service::Switch(){
    power=new Characteristic::On(0,true);     
    pinMode = pin;
  }

  boolean update(){                              // update() method

    LOG1(":  Current Power=");
    LOG1(power->getVal()?"true":"false");
    LOG1("\n");
  
    if(power->updated()){
      if (power->getNewVal())  {
        LOG1("LOW\n");
        digitalWrite(pinMode, LOW);
      } else {
        LOG1("HIGH\n");
        digitalWrite(pinMode, HIGH);
      }
    }
    return(true);                               // return true
  
  } // update

  void loop() {
    int pinRead = !digitalRead(pinMode);
    LOG1("pinRead: ");
    LOG1(pinRead);
    LOG1("\npower: ");
    LOG1(power->getVal());
    LOG1("\n");
    delay(500);
    if (power->getVal() != pinRead)
      power->setVal(pinRead);
      LOG1("\n******* Alteracao ******\n");
  }
};