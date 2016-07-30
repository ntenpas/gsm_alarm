#include <SoftwareSerial.h>
#include <EEPROM.h>
SoftwareSerial sim(7,8);

char c;
char pnum[11];
char sendcmd[23] = "AT+CMGS=\"+1xxxxxxxxxx\"";
char newalert[13];
int addr;

int newmessage(char *cp);

void setup() {
  sim.begin(9600);
  Serial.begin(9600);
  for (addr = 0; addr <= 10; ++addr)
    pnum[addr] = EEPROM.read(addr);
  Serial.println("okay");
  Serial.print("phone number: ");
  Serial.println(pnum);
  for (addr = 0; addr < 10; ++addr)
    sendcmd[addr + 11] = pnum[addr];
  for (addr = 0; addr <= 12; ++addr)
    newalert[addr] = '\0';
}

void loop() {
  // +CMTI: "SM",1
  // means that there is a new message
  if (Serial.available()) {
    c = Serial.read();
    if (c == 'd') {
      sim.println("AT+CMGD=1,4");
    }
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
    for (addr = 1; addr <= 12; ++addr)
      newalert[addr - 1] = newalert[addr];
    newalert[12] = c;
  }
  if (newmessage(newalert)) {
    Serial.println("new message!");
  }
}

int newmessage(char *cp) {
  int i;
  char cmp[] = "+CMTI: \"SM\",1";
  for (i = 0; i <= 12; ++i) {
    if (newalert[i] != cmp[i])
      return 0;
  }
  return 1;
}

