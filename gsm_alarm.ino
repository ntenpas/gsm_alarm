#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
SoftwareSerial sim(7,8);

char c;
char pnum[11];
char sendcmd[23] = "AT+CMGS=\"+1xxxxxxxxxx\"";
char newalert[13];
char newmsg[4];
char msgalert[8];
int addr;
int msglen;
int endofmsg;
int nl;
// nl == 1 is look for newline before message
// nl == 2 is during message
// reset to zero after message's nl
//const int MPU_addr=0x68;
//int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
//int16_t oldAcX,oldAcY,oldAcZ;
int pulseX, pulseY;
long int aX, aY;
//int old_aX, old_aY;
long int avg_aX, avg_aY;
long int old_aX[20];
long int old_aY[20];
int count;
int moved;
int horn;
int grab;
long int horncount;
int horntime;
int simavail;

int checkfornew(char *cp);
int checkformsg(char *cp);
int checkforhorn(char *cp);
int checkforon(char *cp);
int checkforoff(char *cp);

void setup() {
  sim.begin(9600);
  Serial.begin(9600);
  for (addr = 0; addr <= 10; ++addr)
    pnum[addr] = EEPROM.read(addr);
  for (addr = 0; addr < 10; ++addr)
    sendcmd[addr + 11] = pnum[addr];
  Serial.println("okay");
  Serial.print("phone number: ");
  Serial.println(pnum);
  Serial.flush();
  for (addr = 0; addr <= 12; ++addr)
    newalert[addr] = '\0';
  for (addr = 0; addr <= 3; ++addr)
    newmsg[addr] = '\0';
  for (addr = 0; addr <= 7; ++addr)
    msgalert[addr] = '\0';

  /*
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU_6050)
  Wire.endTransmission(true);
  */

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  pinMode(9, OUTPUT); 
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
  delay(2000);
  digitalWrite(9, LOW);
  delay(3000);
  
  Serial.begin(9600);
  /*
  AcX = 0;
  AcY = 0;
  AcZ = 0;
  oldAcX = 0;
  oldAcY = 0;
  oldAcZ = 0;
  */
  aX = 0;
  aY = 0;
  //old_aX = 0;
  //old_aY = 0;
  avg_aX = 0;
  avg_aY = 0;
  count = -20;
  moved = 0;
  horn = 0;
  grab = 0;
  horncount = 0;
  horntime = 0;
  simavail = 0;
  for (int i = 0; i < 20; ++i) {
    old_aX[i] = 0;
    old_aY[i] = 0;
  }
}

void loop() {
  // +CMTI: "SM",1
  // means that there is a new message
  /*
  if (Serial.available()) {
    c = Serial.read();
    if (c == 'd') {
      sim.println("AT+CMGD=1,4");
      Serial.println("deleting");
    }
    if (c == 's') {
      sim.println("AT+CBC");
    }
    else if (c == 'r') {
      sim.println("AT+CMGF=1");
      delay(130);
      sim.println("AT+CMGL=\"ALL\"");
      nl = 0;
      msglen = 0;
      endofmsg = 0;
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
  */
  
  if (horn == 1 && horntime < 10) {
    if (horncount == 0) {
      digitalWrite(4, HIGH);
      //Serial.println("beep high");
      ++horntime;
    }
    else if (horncount == 10000) {
      digitalWrite(4, LOW);
      //Serial.println("beep low");
    }
    //Serial.println(horncount);
    horncount = (horncount > 20000) ? 0 : (horncount + 1);
  }
  else if (horntime >= 10) {
    horn = 0;
    horntime = 0;
    moved = 0;
    horn = 0;
    count = 0;
    for (int i = 0; i < 20; ++i) {
      old_aX[i] = 0;
      old_aY[i] = 0;
    }
    avg_aX = 0;
    avg_aY = 0;
    digitalWrite(4, LOW);
  }
  if (grab == 0 && simavail == 0) {
    // movement
    /*
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
    oldAcX = AcX;
    oldAcY = AcY;
    oldAcZ = AcZ;
    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    Serial.print("x: ");
    Serial.println(AcX);
    Serial.print("y: ");
    Serial.println(AcY);
    Serial.print("z: ");
    Serial.println(AcZ);
    if (count < 2)
      ++count;
    if (count == 2 && !moved) {
      if (AcX < (oldAcX - 1000) || AcX > (oldAcX + 1000)) {
        Serial.println("X moved");
        moved = 1;
      }
      if (AcY < (oldAcY - 1000) || AcY > (oldAcY + 1000)) {
        Serial.println("Y moved");
        moved = 1;
      }
      if (AcZ < (oldAcZ - 1000) || AcZ > (oldAcZ + 1000)) {
        Serial.println("Z moved");
        moved = 1;
      }
    }
    */

    //old_aX = aX;
    //old_aY = aY;
    for (int i = 0; i < 19; ++i) {
      old_aX[i] = old_aX[i + 1];
      old_aY[i] = old_aY[i + 1];
    }
    old_aX[19] = aX;
    old_aY[19] = aY;
    avg_aX = 0;
    avg_aY = 0;
    for (int i = 0; i < 20; ++i) {
      avg_aX += old_aX[i];
      avg_aY += old_aY[i];
    }
    avg_aX /= 20;
    avg_aY /= 20;
    pulseX = pulseIn(2, HIGH);
    pulseY = pulseIn(3, HIGH);
    aX = ((pulseX / 10) - 500) * 8;
    aY = ((pulseY / 10) - 500) * 8;
    /*
    Serial.print(aX);
    Serial.print("\t");
    Serial.println(aY);
    */
    //if (count < 2)
    //  ++count;
    //if (count == 2 && !moved) {
    if (count < 21) {
      ++count; 
    }
    //if (count == 21)
    //  Serial.println("count == 21");
    if (count == 21 && !moved) {
      if (aX < (avg_aX - 30) || aX > (avg_aX + 30)) {
        /*
        Serial.print("count: ");
        Serial.println(count);
        Serial.println("X moved");
        Serial.print("avg_aX: ");
        Serial.println(avg_aX);
        for (int i = 0; i < 19; ++i) {
          Serial.println(old_aX[i]);
        }
        Serial.print("aX: ");
        Serial.println(aX);
        Serial.print("diff: ");
        Serial.println(aX - avg_aX);
        */
        moved = 1;
      }
      if (aY < (avg_aY - 30) || aY > (avg_aY + 30)) {
        /*
        Serial.print("count: ");
        Serial.println(count);
        Serial.println("Y moved");
        Serial.print("avg_aY: ");
        Serial.println(avg_aY);
        for (int i = 0; i < 19; ++i) {
          Serial.println(old_aY[i]);
        }
        Serial.print("aY: ");
        Serial.println(aY);
        Serial.print("diff: ");
        Serial.println(aY - avg_aY);
        */
        moved = 1;
      }
    }
    
    
    //Serial.print("AcX = "); Serial.print(AcX);
    //Serial.print(" | AcY = "); Serial.print(AcY);
    //Serial.print(" | AcZ = "); Serial.print(AcZ);
    //Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
    //Serial.print(" | GyX = "); Serial.print(GyX);
    //Serial.print(" | GyY = "); Serial.print(GyY);
    //Serial.print(" | GyZ = "); Serial.println(GyZ);
  }
  grab = (grab > 10000) ? 0 : (grab + 1);
  
  if (sim.available()) {
    simavail = 1;
    c = sim.read();
    //Serial.print(c);
    for (addr = 1; addr <= 12; ++addr)
      newalert[addr - 1] = newalert[addr];
    newalert[12] = c;
    for (addr = 1; addr <= 7; ++addr)
      msgalert[addr - 1] = msgalert[addr];
    msgalert[7] = c;
    if (nl == 2 && msglen < 4) {
      ++msglen;
      for (addr = 1; addr <= 3; ++addr)
        newmsg[addr - 1] = newmsg[addr];
      newmsg[3] = c;
    }
    if (c == '\n' && nl > 0) {
      //Serial.println("newline!");
      if (nl == 2) {
        nl = 0;
        endofmsg = 1;
      }
      else {
        ++nl;
      }
    }
    if (checkfornew(newalert)) {
      Serial.println("new message!");
      //
      sim.println("AT+CMGF=1");
      delay(130);
      sim.println("AT+CMGL=\"ALL\"");
      nl = 0;
      msglen = 0;
      endofmsg = 0;
      //
    }
    if (checkformsg(msgalert)) {
      //Serial.println("list message");
      // look for the next newline - then read message
      nl = 1;
    }
  }
  else {
    simavail = 0;
  }
  if (endofmsg == 1) {
    if (checkforhorn(newmsg)) {
      Serial.println("horn!");
      horn = 1;
    }
    if (checkforon(newmsg)) {
      Serial.println("on!");
      moved = 0;
      horn = 0;
      count = 0;
      for (int i = 0; i < 20; ++i) {
        old_aX[i] = 0;
        old_aY[i] = 0;
      }
      avg_aX = 0;
      avg_aY = 0;
      digitalWrite(4, LOW);
    }
    if (checkforoff(newmsg)) {
      Serial.println("off!");
      moved = 2;
      horn = 0;
      digitalWrite(4, LOW);
    }
    delay(130);
    sim.println("AT+CMGD=1,4");
    endofmsg = 0;
    for (addr = 0; addr <= 3; ++addr)
      newmsg[addr] = '\0';
  }
  if (moved == 1) {
    Serial.println("sending a text");
    sim.println("AT+CMGF=1");
    delay(130);
    sim.println(sendcmd);
    delay(130);
    sim.print("moved");
    delay(130);
    sim.print(char(26));
    horn = 1;
    moved = 2;
  }
}

int checkfornew(char *cp) {
  int i;
  char cmp[] = "+CMTI: \"SM\",1";
  for (i = 0; i <= 12; ++i) {
    if (newalert[i] != cmp[i])
      return 0;
  }
  return 1;
}

int checkformsg(char *cp) {
  int i;
  char cmp[] = "+CMGL: 1";
  for (i = 0; i <= 7; ++i) {
    if (msgalert[i] != cmp[i])
      return 0;
  }
  return 1;
}

int checkforhorn(char *cp) {
  int i;
  char cmp[] = "Horn";
  for (i = 0; i <= 3; ++i) {
    if (newmsg[i] != cmp[i])
      return 0;
  }
  return 1;
}

int checkforon(char *cp) {
  int i;
  char cmp[] = "On";
  for (i = 0; i <= 1; ++i) {
    //Serial.print(newmsg[i]);
    if (newmsg[i] != cmp[i])
      return 0;
  }
  return 1;
}

int checkforoff(char *cp) {
  int i;
  char cmp[] = "Off";
  for (i = 0; i <= 2; ++i) {
    if (newmsg[i] != cmp[i])
      return 0;
  }
  return 1;
}

