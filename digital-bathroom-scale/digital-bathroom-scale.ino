/* Wireless Bathroom Scale
 * Uses digispark pro (https://digistump.com)
 * 
 * depends on hx711 and rcswitch libraries
 * https://github.com/bogde/HX711
 * https://github.com/sui77/rc-switch
 * 
 */


#include "HX711.h"
#include <RCSwitch.h>

#define DOUT 0
#define CLK  2
HX711 scale(DOUT, CLK);
RCSwitch rc = RCSwitch();

const float gr = 453.59237;
const long calibration_factor = 11181;
const int numReadings = 8;

int lastval = 0;
boolean blink = LOW;
unsigned long blip = 0;

float readings[numReadings];      // the readings from the input
int readIndex = 0;                // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

bool active = false;
void setup() {
  delay(500);
  pinMode(1, OUTPUT);
  rc.enableTransmit(3);
  rc.setRepeatTransmit(5);
  scale.power_up();
  scale.set_scale();
  scale.tare();
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  rc.send(1003, 24);
}

void loop() {
  if (millis() - blip >= 1000) {
    if (!active) {
      digitalWrite(1, !digitalRead(1));
      blip = millis();
    }
  }
  float value = scale.get_units();
  if (value > 1) {
    digitalWrite(1, !digitalRead(1));
    active = true;
    long tempvalue = round(((value * gr) / 1000) * 100);
    rc.send(10000 + tempvalue, 24);

    /*
      readings[readIndex] = value;
      total = total + readings[readIndex];
      readIndex++;
      if (readIndex >= numReadings) {
      average = total / numReadings;
      readIndex = 0;
      total = 0;
      long tempvalue = round(((average * gr) / 1000) * 100);
      rc.send(10000 + tempvalue, 24);
      }
    */
  } else {
    if (active == true) {
      rc.send(10001, 24);
      active = false;
      total = 0;
      readIndex = 0;
      scale.set_scale();
      scale.tare();
      scale.set_scale(calibration_factor);
    }
  }
}