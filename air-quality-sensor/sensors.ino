#include <OneWire.h>
#include <dht11.h>

#define minipirhigh 300 // milivolt
#define adcdelay 8 // miliseconds
#define readinterval 6000 // ms
#define pirtime 250 // miliseconds
#define lightdiff 50
#define minvoltage 1
#define GASPreheat 3000

////////////////////////////////////////////////////////////////////////
#define DS18PIN 8
#define DHT11PIN 13
const int MQ9 = A0;      // MQ9 CO2,CO3,Butane,Propane Sensitive gas sensor.
const int LDR = A2;      // Photoresistor for measure environment light.
const int PIR = A1;      // 3v high mini pir sensor, waste of adc port
////////////////////////////////////////////////////////////////////////

OneWire ds(DS18PIN);
dht11 DHT11;

bool dhtcheck() {

  if (DHT11.read(DHT11PIN) == DHTLIB_OK) {
    return true;
  }
  return false;
}

int getHumidity() {
  return DHT11.humidity;
}

double dewPoint(double celsius, double humidity) {
  double A0 = 373.15 / (273.15 + celsius);
  double SUM = -7.90298 * (A0 - 1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / A0))) - 1) ;
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (A0 - 1))) - 1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}

float getTemp() {
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    // Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    //  Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  return TemperatureSum;
}

bool checkpir() {
  if ((analogRead(PIR) > minipirhigh) && ((millis() - pirtimer) > pirtime) ) {
    pirtimer = millis();
    return true;
  }
  return false;
}

bool checkldr() {
  analogReference(DEFAULT);
  if (abs((analogRead(LDR) - lightlevel)) > lightdiff && ((millis() - ldrtimer) > pirtime)) {
    lightlevel = analogRead(LDR);
    ldrtimer = millis();
    analogReference(INTERNAL);
    return true;
  }
  analogReference(INTERNAL);
  return false;
}

void collect() {
  if (millis() > GASPreheat) {
    analogRead(MQ9);
    delay(adcdelay);
    gasbuffer = analogRead(MQ9);
  }

  dhtcheck();
  rawtemp = getTemp();
  if (rawtemp == 85) {
    rawtemp = 0;
  }
}
