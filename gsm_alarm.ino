#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial sim(7,8);

char c;
char pnum[11];
char sendcmd[23] = "AT+CMGS=\"+1xxxxxxxxxx\"";
int addr;

void setup() {
  sim.begin(9600);
  Serial.begin(9600);
  for (addr = 0; addr <= 10; ++addr)
    pnum[addr] = EEPROM.read(addr);
  Serial.println("okay");
  Serial.print("phone number: ");
  Serial.println(pnum);
  for (addr = 0; addr < 10; ++addr) {
    sendcmd[addr + 11] = pnum[addr];
  }
}

void loop() {
  if (Serial.available()) {
    c = Serial.read();
    if (c == 's') {
      sim.println("AT+CBC");
    }
    else if (c == 'r') {
      sim.println("AT+CMGF=1");
      delay(130);
      sim.println("AT+CMGL=\"ALL\"");
    }
    else if (c == 'w') {
      sim.println("AT+CMGF=1");
      delay(130);
      sim.println(sendcmd);
      delay(130);
      sim.print("this is another msg");
      delay(130);
      sim.print(char(26));
    }
  }
  if (sim.available()) {
    c = sim.read();
    Serial.print(c);
  }
}
