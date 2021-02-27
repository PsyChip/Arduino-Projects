#include <avr/wdt.h>
#include <RCSwitch.h>

#define SENSCYCLE 5000
#define SYSCYCLE 10000
#define rcset 500
RCSwitch rc = RCSwitch();
RCSwitch rs = RCSwitch();
String clist[] = {"RF"};

unsigned long lpir, lsens, llight, lsys, lping;
int lightlevel, gasbuffer, ldrstate;
unsigned long pirtimer;
unsigned long ldrtimer;
unsigned long lastrc = 0;
int rcbuf, rcprev;
int fstep;
float rawtemp;
String buffer;
String command;
String param;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("$BOOT,0;"));
  pirtimer = 0;
  analogReference(INTERNAL);
  rc.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  rs.enableTransmit(3);
  rs.setRepeatTransmit(15);
  wdt_enable(WDTO_4S);
  Serial.println(F("$BOOT,1;"));
}

void pump() {
  Serial.print(F("$AIR,"));
  Serial.print(getHumidity());
  Serial.print(F(","));
  Serial.print(rawtemp);
  Serial.print(F(","));
  Serial.print(dewPoint(rawtemp, getHumidity()));
  Serial.print(F(","));
  Serial.print(gasbuffer);
  Serial.print(F(","));
  Serial.print(lightlevel);
  Serial.println(F(";"));
}

void pushpir() {
  Serial.print(F("$PIR,"));
  Serial.print(pirtimer);
  Serial.print(F(","));
  Serial.print(millis());
  Serial.println(F(";"));
}

void pushldr() {
  Serial.print(F("$LIGHT,"));
  Serial.print(lightlevel);
  Serial.print(F(","));
  Serial.print(ldrtimer);
  Serial.print(F(","));
  Serial.print(millis());
  Serial.println(F(";"));
}

void loop(void) {
  if (Serial.available() > 0) {
    char incomingByte = (char)Serial.read();
    if (incomingByte == ';') {
      fstep = 0;
      for (int i = 0; i < buffer.length(); i++) {
        if (buffer[i] == '=') {
          fstep = 1;
          continue;
        }

        if (fstep == 0) {
          command += buffer[i];
        } else {
          param += buffer[i];
        }
      }

      if (command.startsWith(clist[0])) {
        rs.send(param.toInt(), 24);
        Serial.print("Signal sent: ");
        Serial.println(param.toInt());
      }

      command = "";
      param = "";
      buffer = "";
    } else {
      buffer += incomingByte;
    }
    return ;
  }

  if (checkpir() == true) {
  //  pushpir();
  }

  if (checkldr() == true) {
  //  pushldr();
  }

  if (rc.available()) {
    rcbuf = rc.getReceivedValue();
    if ((rcbuf != rcprev) || ((millis() - lastrc) >= rcset)) {
      Serial.print(F("$RF,"));
      Serial.print(rcbuf);
      Serial.print(F(","));
      Serial.print(rc.getReceivedBitlength());
      Serial.print(F(","));
      Serial.print(rc.getReceivedProtocol());
      Serial.println(F(";"));
      rcprev = rcbuf;
      lastrc = millis();
    }
    rc.resetAvailable();
  }

  if ((millis() - lsens) > SENSCYCLE) {
    collect();
    pump();
    lsens = millis();
  }

  if ((millis() - lsys) > SYSCYCLE) {
    Serial.print(F("$SYST,"));
    Serial.print(PORTD);
    Serial.print(F(","));
    Serial.print((int)availableMemory);
    Serial.print(F(","));
    Serial.print(GetTemp());
    Serial.print(F(","));
    Serial.print(readVcc());
    Serial.print(F(","));
    Serial.print(millis());
    Serial.println(F(";"));
    lsys = millis();
  }
  wdt_reset();
}
