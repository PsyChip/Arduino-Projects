/*

  Serial Relay Controller
  root@psychip.net
  April 2015
  
*/

const byte NUMBER_OF_PINS = 4;                     // set number of relays
byte relayPin[NUMBER_OF_PINS] = {7,6,5,4};         // define pin numbers of relays connected to 
String clist[]={"SET","ON","OFF","ALLON","ALLOFF"}; 

/*
  command format: command=param;
 
  SET=x; allows direct manipulation PORTD of arduino board 
  ON=x; toggle relay on (x: relay number 1 to 4) 
  OFF=x; toggle relay off
  ALLON; turn all relays on
  ALLOFF; turn all relays off
  
*/

String buffer; 
String command;
String param;
int recv;
int fstep;

void setup(void) {
  recv=0;
  buffer = "";
  delay(250);
  while(!Serial);
  Serial.begin(19200);
  Serial.println("Initializing..");
}

void loop(void) {
    
  // parse commands start
  recv=0;
  
  if (Serial.available() > 0) {
   recv=1; 
    char incomingByte = (char)Serial.read();
    if (incomingByte == ';') {
    
    fstep=0;  
    for(int i=0;i<buffer.length();i++) {
    if(buffer[i]=='=') {
    fstep=1;
    continue;
    }
    
    if(fstep==0) {
    command +=buffer[i];
    } else {
    param +=buffer[i];
    }   
    }    
   
    if(command.startsWith(clist[0])) {
      PORTD = param.toInt();
    }  
    
    if(command.startsWith(clist[1])) {
    digitalWrite(relayPin[param.toInt()-1],LOW);
    }
    
    if(command.startsWith(clist[2])) {
    digitalWrite(relayPin[param.toInt()-1],HIGH);
    }
     
    if(command.startsWith(clist[3])) {
     PORTD=255;
    }
    
    if(command.startsWith(clist[4])) {
    PORTD=0;
    }    
       
    command = "";
    param = "";
    buffer = "";
    } else {
        buffer += incomingByte;
  } 
}

if(recv==1) {
  return ;
}

 // parse commands end 
 
 // send relay status to pc software
  Serial.print("+RELAY,");
  for(int i=0;i<NUMBER_OF_PINS;i++) {
  Serial.print(!digitalRead(relayPin[i]));
  Serial.print(",");
  }
  Serial.println("0;");
 // relay status end

  Serial.flush();
  delay(50);
}
