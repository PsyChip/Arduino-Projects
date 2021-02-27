#include <SoftwareSerial.h>
#include <OneWire.h>

const String uniqid = "180316-003";
const String serveraddr = "192.168.1.21";
const String serverport = "3535";
const String ssid = "your_ssid_here";
const String password = "your_ssid_pass";

int TMP = 13;
int LDR = A1;
int SOL = A0;

unsigned long lastreading = 0;
const int readinterval = 9000;
const int pumpinterval = 3000;
const int serdelay = 250;

OneWire ds(TMP);
SoftwareSerial esp(2,3); // RX, TX 

int light,soil; 
float tempraw;
   
unsigned long l1,latency;

void setup(void) {
 Serial.begin(9600); 
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  esp.begin(9600);
   while (!esp) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  delay(1000);
  esp.print("AT+RST\r\n");
  delay(6000);
  esp.print("AT+CIPMUX=0\r\n"); 
  delay(250);
  esp.print("AT+CWJAP=\""+ssid+"\",\""+password+"\""+"\r\n");
  delay(8000);
}

void pumpdata() {
  if((millis()-lastreading)>readinterval) {
  analogRead(LDR);
  light = analogRead(LDR);
  delay(100);
  analogRead(SOL);
  soil = analogRead(SOL);   
  lastreading=millis();  
  delay(100); 
  tempraw=getTemp();
  lastreading=millis();
  }

  char buffer[10];
  String tem = dtostrf(tempraw, 5, 2, buffer);
  String temper = (tem);

  float thrmraw;
  thrmraw=readThrm();
  char buffer2[10];
  String thm = dtostrf(thrmraw, 5, 2, buffer2);
  String thermal = (thm);

  String querystr = "/c/pushdata/";
  querystr += "id="+uniqid+"|";
  querystr += "uptime="+String(millis(), DEC)+"|";
  querystr += "light="+String(light, DEC)+"|";
  querystr += "temp="+temper+"|";
  querystr += "soil="+String((1023-soil), DEC)+"|";
  
  querystr += "vcc="+String(readVcc(), DEC)+"|";
  querystr += "thrm="+thermal+"|";
  querystr += "latc="+String(latency, DEC);
  httpGet(serveraddr,serverport,querystr);   
 }

void loop(void) {
  l1=millis();
  pumpdata();
  latency=millis()-l1;
  delay(pumpinterval);
}

void httpGet(String host,String port,String request) {
 String hdata = "GET "+request;
  hdata += " HTTP/1.1\r\n";
  hdata += "Host: "+host+"\r\n"; 
  hdata += "Connection: close\r\n"; 
  hdata += "\r\n";
  
  String cipSend = "AT+CIPSEND=";
  cipSend +=hdata.length();
  cipSend +="\r\n";
   
  esp.print("AT+CIPSTART=\"TCP\",\""+host+"\","+port+"\r\n"); 
  delay(serdelay);
  
  esp.print(cipSend); 
  delay(serdelay);
  
  esp.print(hdata);   
  delay(serdelay);
  
  esp.print("AT+CIPCLOSE\r\n");  
  delay(serdelay);
}

double readThrm(void) {
  unsigned int wADC;
  double t;
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  delay(8); 
  ADCSRA |= _BV(ADEN);  // enable the ADC
  ADCSRA |= _BV(ADSC);  // Start the ADC
  while (bit_is_set(ADCSRA,ADSC));
  wADC = ADCW;
  t = (wADC - 324.31 ) / 1.22;
  return (t);
}


long readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;
  return result;
}

float getTemp(){
  byte data[12];
  byte addr[8];

  if (!ds.search(addr)) {
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
  ds.write(0x44,1); // start conversion, with parasite power on at the end

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

