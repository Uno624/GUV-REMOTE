#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <DHT22.h>
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <ModbusMaster.h>  
#include <RF24.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>


#define pinDATA 3

#define uart1_rx 17
#define uart1_tx 18

#define RS485_DE_RE_PIN 6   // ขา Enable ส่ง/รับ ของ MAX485 (DE + RE)
#define RS485_RX_PIN    19  // RO จาก MAX485 → RX2
#define RS485_TX_PIN    20  // DI จาก ESP32 → TX2

#define SCK 12
#define MISO 13
#define MOSI 11

#define TXCSN  46
#define TXCE   10

#define RXCSN  14
#define RXCE   21

RF24 radioTX(TXCE, TXCSN);
RF24 radioRX(RXCE, RXCSN);

bool ARM, Resrc;
const int CENTER_X = 1772;
const int CENTER_Y = 1772;
const int DEAD     = 20;
uint8_t speed = 0;
int stae = 0;

const byte addressTX[6] = "John";
const byte addressTXPos[6] = "JohnP";

const byte addressRX[6] = "Artur";

/*การกำหนดค่าเริ่มต้นให้กับโครงสร้าง (structure) ที่จัดเรียงสมาชิกในหน่วยความจำให้ ชิดติดกันโดยไม่มีการเว้นช่องว่าง ซึ่งแตกต่างจากค่าเริ่มต้นปกติที่คอมไพเลอร์อาจเพิ่มช่องว่าง
(padding) เพื่อเพิ่มประสิทธิภาพในการเข้าถึงข้อมูล โดยมีจุดประสงค์หลักเพื่อลดขนาดของหน่วยความจำที่ใช้และเพื่อให้มั่นใจว่าข้อมูลจะมีรูปแบบตรงตามที่กำหนดไว้ 
int8_t → 1 byte
int16_t × 5 → 10 bytes
uint16_t × 4 → 8 bytes
float × 2 → 8 bytes
int16_t → 2 bytes
รวม = 1 + 10 + 8 + 8 + 2 = 29 bytes
*/
struct __attribute__((packed)) SoilPacket {
  int8_t   lux;
  int16_t  t10;
  int16_t  h10;
  int16_t  hum10;
  int16_t  tmp10;
  int16_t  ph10;
  uint16_t ec;
  uint16_t n;
  uint16_t p;
  uint16_t k;
  float    lat;
  float    lon;
  int16_t  au;
};
SoilPacket dataTX;
/*
struct GNSSPayload {
  float    lat;
  float    lon;
  int16_t  au;
};
GNSSPayload dataTXPosition;*/

struct Payload {
  uint8_t  b1;
  uint8_t  b2;
  uint8_t  b3;
  uint8_t  trig;
  uint16_t joyX;
  uint16_t joyY;
  bool ARMstatus;
  bool Resrcstatus;
};
Payload dataRX;

TinyGPSPlus gps;
HardwareSerial ss(1);

BH1750 lightMeter;

DHT22 dht22(pinDATA); 

HardwareSerial &RS485Serial = Serial2;
ModbusMaster node;

Servo Servo1;

// ====== ควบคุม DE/RE สำหรับ half-duplex ======
void preTransmission()
{
  digitalWrite(RS485_DE_RE_PIN, HIGH); // เข้าโหมดส่ง
}

void postTransmission()
{
  digitalWrite(RS485_DE_RE_PIN, LOW);  // กลับมาโหมดรับ
}

void idle(){
  digitalWrite(36, 0);
  digitalWrite(37, 0);
  digitalWrite(40, 0);
  digitalWrite(41, 0);
  analogWrite(35, 0);
  analogWrite(38, 0);
  analogWrite(39, 0);
  analogWrite(42, 0);
  stae = 0;
}

void backward(){
  digitalWrite(36, 1);
  digitalWrite(37, 0);
  digitalWrite(40, 1);
  digitalWrite(41, 0);
  analogWrite(35, updateSpeedXY());
  analogWrite(38, updateSpeedXY());
  analogWrite(39, updateSpeedXY());
  analogWrite(42, updateSpeedXY());
}

void forward(){
  digitalWrite(36, 0);
  digitalWrite(37, 1);
  digitalWrite(40, 0);
  digitalWrite(41, 1);
  analogWrite(35, updateSpeedXY());
  analogWrite(38, updateSpeedXY());
  analogWrite(39, updateSpeedXY());
  analogWrite(42, updateSpeedXY());
}
void spinright(){
  digitalWrite(36, 0);
  digitalWrite(37, 1);
  digitalWrite(40, 1);
  digitalWrite(41, 0);
  analogWrite(35, updateSpeedXY());
  analogWrite(38, updateSpeedXY());
  analogWrite(39, updateSpeedXY());
  analogWrite(42, updateSpeedXY());
}

void spinleft(){
  digitalWrite(36, 1);
  digitalWrite(37, 0);
  digitalWrite(40, 0);
  digitalWrite(41, 1);
  analogWrite(35, updateSpeedXY());
  analogWrite(38, updateSpeedXY());
  analogWrite(39, updateSpeedXY());
  analogWrite(42, updateSpeedXY());
}

void Motoersoild(){
  digitalWrite(36, 1);
  digitalWrite(37, 0);
  digitalWrite(40, 0);
  digitalWrite(41, 1);
  analogWrite(35, updateSpeedXY());
  analogWrite(38, updateSpeedXY());
  analogWrite(39, updateSpeedXY());
  analogWrite(42, updateSpeedXY());
}


void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, uart1_rx, uart1_tx, false);
  SPI.begin(SCK, MISO, MOSI);
  SPI.setBitOrder(MSBFIRST);
  pinMode(TXCSN, OUTPUT); digitalWrite(TXCSN, 1); 
  pinMode(RXCSN, OUTPUT); digitalWrite(RXCSN, 1);
  pinMode(TXCE,  OUTPUT); digitalWrite(TXCE, 0);   
  pinMode(RXCE,  OUTPUT); digitalWrite(RXCE, 0);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(42, OUTPUT);
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(5, 4);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
     
  Servo1.setPeriodHertz(50);    // Standard 50Hz servo
  Servo1.attach(2, 500, 2400); 
  lightMeter.begin();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // เริ่มที่โหมดรับ

  // เริ่ม UART2 สำหรับ RS485
  RS485Serial.begin(4800, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // ตั้งค่า ModbusMaster
  uint8_t slaveID = 1;   // Modbus Device ID = 1
  node.begin(slaveID, RS485Serial);

  // Callback เปิด/ปิด DE/RE
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  if (!radioTX.begin()) {
    Serial.println("RadioTX not responding!");
    while (1) { delay(1000); }
  }
  radioTX.begin();
  radioTX.setChannel(102);
  radioTX.setDataRate(RF24_1MBPS);  
  radioTX.setPALevel(RF24_PA_LOW);  
  radioTX.stopListening();        //โหมดส่ง   
  

    if (!radioRX.begin()) {
    Serial.println("RadioRX not responding!");
    while (1) { delay(1000); }
  }
  radioRX.begin();
  radioRX.setChannel(100);          // ช่อง 0–125
  radioRX.setDataRate(RF24_1MBPS);  
  radioRX.setPALevel(RF24_PA_LOW);  
  radioRX.openReadingPipe(1, addressRX); 
  radioRX.startListening();        //โหมดรับ  
  delay(500);
  Servo1.write(0);

  Serial.println(F("Test begin"));
}
float lux ;
unsigned long lastModbus = 0;
unsigned long lastDHT    = 0;
unsigned long lastSend   = 0;
unsigned long lastpressed   = 0;
unsigned long lastDATARXPOSEAD   = 0;


int x;
bool inaction;
bool lastB2;

void loop() {
  unsigned long now = millis();
  uint8_t result;
  ARM = dataRX.ARMstatus;
  Resrc = dataRX.Resrcstatus;
  idle();

 /* while (ss.available() > 0) {
    gps.encode(ss.read());
    dataTXPosition.lat = gps.location.lat();
    dataTXPosition.lon = gps.location.lng();
    dataTXPosition.au  = gps.altitude.meters();
  }*/

  // ===== 5) รับจอยจาก RF24 ตัวอื่น =====
  if (radioRX.available()) {
    radioRX.read(&dataRX, sizeof(dataRX));
  }
    Serial.print("RX <- ");
    Serial.print("B1:");   Serial.print(dataRX.b1);
    Serial.print(" B2:");  Serial.print(dataRX.b2);
    Serial.print(" B3:");  Serial.print(dataRX.b3);
    Serial.print(" TRG:"); Serial.print(dataRX.trig);
    Serial.print(" X:");   Serial.print(dataRX.joyX);
    Serial.print(" Y:");   Serial.print(dataRX.joyY);
    Serial.print(" speed:"); Serial.print(updateSpeedXY());
    Serial.print(" turn:"); Serial.print(stae);
    Serial.print(" inaction:"); Serial.print(inaction);
    Serial.print(" ARM:");   Serial.print(ARM);
    Serial.print(" Resrc:");   Serial.print(Resrc);
    Serial.print(" servo:");   Serial.print(AngleResrcControll());
    Serial.print(" tmp:");   Serial.print((dataTX.tmp10));
    Serial.print(" lat:");   Serial.print(dataTX.lat);
    Serial.print(" lon:");   Serial.print(dataTX.lon);
    Serial.print(" au:");   Serial.print(dataTX.au);
    Serial.print(" airtmp:");   Serial.println((dataTX.t10));

    if (now - lastDATARXPOSEAD >= 500){
      lastDATARXPOSEAD = now;
      radioTX.openWritingPipe(addressTX);
      bool ok = radioTX.write(&dataTX, sizeof(dataTX));
    }
 ////////////////MODE สำรวจ//////////////////////
  if (ARM == 1) {
    Resrc = 0;
    if (stae == 4) {forward();}
    else if (stae == 3) {backward();}
    else if (stae == 2) {spinleft();}
    else if (stae == 1) {spinright();}
    else {idle();}
  }
 ////////////////MODE ตวรจ//////////////////////
  else if (Resrc == 1){
    ARM = 0;
    Servo1.write(AngleResrcControll());
    delay(100);
    if (dataRX.b2 == 1){
        node.clearResponseBuffer();
        result = node.readHoldingRegisters(0x0000, 7);
        dataTX.t10 = (int16_t)(dht22.getTemperature()*10);
        dataTX.h10 = (int16_t)(dht22.getHumidity()*10);

        if (lightMeter.measurementReady()) {
        dataTX.lux = (int16_t)(lightMeter.readLightLevel()*10);
        }else{ dataTX.lux = 999*10; }

        if (result == node.ku8MBSuccess) {
        dataTX.hum10 = (int16_t) (node.getResponseBuffer(0)*10.0f);
        dataTX.tmp10 = (int16_t) (node.getResponseBuffer(1)*10.0f);
        dataTX.ec    = (int16_t) (node.getResponseBuffer(2)*10.0f);
        dataTX.ph10  = (int16_t) (node.getResponseBuffer(3)*10.0f);
        dataTX.n     = (int16_t) (node.getResponseBuffer(4)*10.0f);
        dataTX.p     = (int16_t) (node.getResponseBuffer(5)*10.0f);
        dataTX.k     = (int16_t) (node.getResponseBuffer(6)*10.0f);} 
      else {
      // ถ้าอยาก debug error code ก็ print ตรงนี้ได้
      Serial.print("Modbus err = "); Serial.println(result);
       }
      while (ss.available() > 0) {
      gps.encode(ss.read());
      dataTX.lat = gps.location.lat();
      dataTX.lon = gps.location.lng();
      dataTX.au  = gps.altitude.meters();
       }
       
      radioTX.openWritingPipe(addressTX); 
      bool ok = radioTX.write(&dataTX, sizeof(dataTX));
    }
    else {
    idle();
    Servo1.write(0);
    }
  }else{Clearsoildata();}
}

 void Clearsoildata(){
    dataTX.lux = 0;
  dataTX.t10 = 0;
  dataTX.h10 = 0;
  dataTX.hum10 = 0;
  dataTX.tmp10 = 0;
  dataTX.ph10 = 0;
  dataTX.ec = 0;
  dataTX.n = 0;
  dataTX.p = 0;
  dataTX.k = 0;
}

uint8_t updateSpeedXY() {
  int speedX = 0;
  int speedY = 0;

  if (dataRX.joyX >= CENTER_X + DEAD) {
    speedX = map(dataRX.joyX, CENTER_X + DEAD, 4095, 0, 255);
    stae = 2;
  }
   else if (dataRX.joyX <= CENTER_X - DEAD) {
    speedX = map(dataRX.joyX, CENTER_X - DEAD, 0,   0, 255);
    stae = 1;
  } 
  else {
    speedX = 0; 
  }

  if (dataRX.joyY >= CENTER_Y + DEAD) {
    speedY = map(dataRX.joyY, CENTER_Y + DEAD, 4095, 0, 255);
    stae = 3;
  } else if (dataRX.joyY <= CENTER_Y - DEAD) {
    speedY = map(dataRX.joyY, CENTER_Y - DEAD, 0,   0, 255);
    stae = 4;
  } else {
    speedY = 0;
  }

  int s = (speedX > speedY) ? speedX : speedY;
  int i = (dataRX.b2 == 0) ? s/2 : s;
  return (uint8_t)constrain(i, 0, 255);
}

uint8_t AngleResrcControll() {
    static uint8_t lastB3   = 0;   // สถานะปุ่มรอบก่อน
    static uint8_t lastTrig = 0;
    static int i;
    unsigned long now = millis();
    if (now - lastSend >= 10) {    // เช็คถี่หน่อย เช่นทุก 10 ms
        lastSend = now;
        int delta =
            (dataRX.b3 == 1  && lastB3   == 0) ?  10 :      // เพิ่งกด b3      → +1
            (dataRX.trig == 1 && lastTrig == 0) ? -10 :     // เพิ่งกด trig    → -1
                                                  0;       // ไม่ได้กดใหม่   → 0
        i += delta;
        // เก็บสถานะล่าสุดไว้เทียบรอบหน้า
        lastB3   = dataRX.b3;
        lastTrig = dataRX.trig;
    }
    return (uint8_t)constrain(i, 0, 255);;
}
