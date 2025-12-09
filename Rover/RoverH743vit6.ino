#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <DHT22.h>
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <ModbusMaster.h>  
#include <RF24.h>
#include <Servo.h>
#include <Arduino.h>
#include <HardwareSerial.h>


#define pinDATA PD3

#define uart1_rx PA9
#define uart1_tx PA10

#define RS485_DE_RE_PIN PA0   // ขา Enable ส่ง/รับ ของ MAX485 (DE + RE)
#define uart2_tx    PA2  // RO จาก MAX485 → RX2
#define uart2_rx    PA3  // DI จาก ESP32 → TX2

#define SCK PA5
#define MISO PA6
#define MOSI PA7

#define TXCSN  PC0
#define TXCE   PC1

#define RXCSN  PC3_C
#define RXCE   PC2_C

RF24 radioTX(TXCE, TXCSN);
RF24 radioRX(RXCE, RXCSN);

bool ARM, Resrc;
const int CENTER_X = 1772;
const int CENTER_Y = 1772;
const int DEAD     = 20;
uint8_t speed = 0;
int stae = 0;

const byte addressTX[6] = "John";
//const byte addressTXPos[6] = "JohnPo";

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
  int16_t  speed;
};
SoilPacket dataTX;

/*
struct GNSSPayload {
  float    lat;
  float    lon;
  int16_t  au;
  int16_t  speed;
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
HardwareSerial Serial1(uart1_rx, uart1_tx);  // USART1: RX=PA10, TX=PA9

BH1750 lightMeter;

DHT22 dht22(pinDATA); 

ModbusMaster node;
HardwareSerial Serial2(uart2_rx,  uart2_tx);  // USART2: RX=PA3,  TX=PA2

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
  digitalWrite(PB14, 0);
  digitalWrite(PB12, 0);
  digitalWrite(PB10, 0);
  digitalWrite(PE14, 0);

  analogWrite(PB15, 0);
  analogWrite(PB13, 0);
  analogWrite(PB11, 0);
  analogWrite(PE11, 0);

  Motoersoildstop();

}

void backward(){
  digitalWrite(PB14, 1);
  digitalWrite(PB12, 0);
  digitalWrite(PB10, 1);
  digitalWrite(PE14, 0);
  analogWrite(PB15, updateSpeedXY());
  analogWrite(PB13, updateSpeedXY());
  analogWrite(PB11, updateSpeedXY());
  analogWrite(PE11, updateSpeedXY());

  Motoersoildstop();
}

void forward(){
  digitalWrite(PB14, 0);
  digitalWrite(PB12, 1);
  digitalWrite(PB10, 0);
  digitalWrite(PE14, 1);
  analogWrite(PB15, updateSpeedXY());
  analogWrite(PB13, updateSpeedXY());
  analogWrite(PB11, updateSpeedXY());
  analogWrite(PE11, updateSpeedXY());

  Motoersoildstop();
}
void spinright(){
  digitalWrite(PB14, 1);
  digitalWrite(PB12, 0);
  digitalWrite(PB10, 0);
  digitalWrite(PE14, 1);
  
  analogWrite(PB15, updateSpeedXY());
  analogWrite(PB13, updateSpeedXY());
  analogWrite(PB11, updateSpeedXY());
  analogWrite(PE11, updateSpeedXY());

  Motoersoildstop();

}

void spinleft(){
  digitalWrite(PB14, 0);
  digitalWrite(PB12, 1);
  digitalWrite(PB10, 1);
  digitalWrite(PE14, 0);

  analogWrite(PB15, updateSpeedXY());
  analogWrite(PB13, updateSpeedXY());
  analogWrite(PB11, updateSpeedXY());
  analogWrite(PE11, updateSpeedXY());

  Motoersoildstop();
}

void Motoersoildstop(){
  digitalWrite(PE12, 0);
  digitalWrite(PE10, 0);
  analogWrite(PE5, 0);
}

void Motoersoildforward(){
  digitalWrite(PE12, 1);
  digitalWrite(PE10, 0);


  analogWrite(PE5,  updateSpeedXY()*2);
}


void Motoersoildbackward(){
  digitalWrite(PE12, 0);
  digitalWrite(PE10, 1);

  analogWrite(PE5,  updateSpeedXY()*2);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  SPI.setSCLK(SCK);
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.begin();
  SPISettings settingsSensor(1000000,  MSBFIRST, SPI_MODE1);
  pinMode(TXCSN, OUTPUT); digitalWrite(TXCSN, 1); 
  pinMode(RXCSN, OUTPUT); digitalWrite(RXCSN, 1);
  pinMode(TXCE,  OUTPUT); digitalWrite(TXCE, 0);   
  pinMode(RXCE,  OUTPUT); digitalWrite(RXCE, 0);
  pinMode(PB15, OUTPUT);
  pinMode(PB14, OUTPUT);
  pinMode(PB13, OUTPUT);
  pinMode(PB12, OUTPUT);
  pinMode(PB11, OUTPUT);
  pinMode(PB10, OUTPUT);
  pinMode(PE11, OUTPUT);
  pinMode(PE13, OUTPUT);
  pinMode(PE12, OUTPUT);
  pinMode(PE10, OUTPUT);
  pinMode(PE14, OUTPUT);
  pinMode(PE5, OUTPUT);


  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.setSCL(PB8);
  Wire.setSDA(PB9);
  Wire.begin();
     
  Servo1.detach();
  lightMeter.begin();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // เริ่มที่โหมดรับ

  // เริ่ม UART2 สำหรับ RS485
  Serial2.begin(4800);

  // ตั้งค่า ModbusMaster
  uint8_t slaveID = 1;   // Modbus Device ID = 1
  node.begin(slaveID, Serial2);

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
    Serial.print(" lat:");   Serial.print(dataTX.lat,6);
    Serial.print(" lon:");   Serial.print(dataTX.lon,6);
    Serial.print(" au:");   Serial.print(dataTX.au);
    Serial.print(" airtmp:");   Serial.print((dataTX.t10));
    Serial.print(" lux:");   Serial.println(lightMeter.readLightLevel());


    if (now - lastDATARXPOSEAD >= 100){
      lastDATARXPOSEAD = now;
     while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
    dataTX.lat = gps.location.lat();
    dataTX.lon = gps.location.lng();
    dataTX.au  = gps.altitude.meters();
    dataTX.speed = gps.speed.kmph();
  }
    bool ok = radioTX.write(&dataTX, sizeof(dataTX));
    }

 if(ARM == 0 && Resrc == 0){
   idle();
 }

 ////////////////MODE สำรวจ//////////////////////
  else if (ARM == 1) {
    Resrc = 0;
    if (stae == 4) {forward();}
    else if (stae == 3) {backward();}
    else if (stae == 2) {spinleft();}
    else if (stae == 1) {spinright();}
    else {idle();}

  }
 ////////////////MODE ตวรจ//////////////////////
  else if (Resrc == 1){
    Servo1.attach(PC7, 400, 2500);
    Servo1.write(AngleResrcControll());
    if (stae == 0) {Motoersoildstop();}
    else if (stae == 4) {Motoersoildforward();}
    else if (stae == 3) {Motoersoildbackward();}
    if (dataRX.b2 == 1){
       Motoersoildstop();
        node.clearResponseBuffer();
        result = node.readHoldingRegisters(0x0000, 7);
        dataTX.t10 = (int16_t)(dht22.getTemperature()*10);
        dataTX.h10 = (int16_t)(dht22.getHumidity()*10);

        if (lightMeter.measurementReady()) {
        dataTX.lux = (int16_t)(lightMeter.readLightLevel()*10);
        }//else{ dataTX.lux = 999*10; }

        if (result == node.ku8MBSuccess) {
        dataTX.hum10 = (int16_t) (node.getResponseBuffer(0)*100.0f);
        dataTX.tmp10 = (int16_t) (node.getResponseBuffer(1)*100.0f);
        dataTX.ec    = (int16_t) (node.getResponseBuffer(2)*10.0f);
        dataTX.ph10  = (int16_t) (node.getResponseBuffer(3)*100.0f);
        dataTX.n     = (int16_t) (node.getResponseBuffer(4)*10.0f);
        dataTX.p     = (int16_t) (node.getResponseBuffer(5)*10.0f);
        dataTX.k     = (int16_t) (node.getResponseBuffer(6)*10.0f);} 
      else {
      // ถ้าอยาก debug error code ก็ print ตรงนี้ได้
      //Serial.print("Modbus err = "); Serial.println(result);
       }
      while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
      dataTX.lat = gps.location.lat();
      dataTX.lon = gps.location.lng();
      dataTX.au  = gps.altitude.meters();
       }
       
      radioTX.openWritingPipe(addressTX); 
      bool ok = radioTX.write(&dataTX, sizeof(dataTX));
    }
  }else{
  Servo1.detach();
  Clearsoildata();}
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
  stae = 0;

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
    static uint8_t lastB3   = 0;   
    static uint8_t lastTrig = 0;
    
    static int targetAngle = 0;    
    static int currentAngle = 0;   

    static unsigned long lastStepTime = 0;  

    unsigned long now = millis();

    if (now - lastSend >= 10) {
        lastSend = now;

        // ตรวจขอบขาขึ้น (กดใหม่)
        if (dataRX.b3 == 1 && lastB3 == 0) {
            targetAngle += 10;     
        }
        if (dataRX.trig == 1 && lastTrig == 0) {
            targetAngle -= 10;     
        }
        targetAngle = constrain(targetAngle, 0, 90);

        lastB3   = dataRX.b3;
        lastTrig = dataRX.trig;
    }

    if (now - lastStepTime >= 100) {
        lastStepTime = now;

        if (currentAngle < targetAngle) {
            currentAngle++;        // ไต่ขึ้นทีละ 1
        } else if (currentAngle > targetAngle) {
            currentAngle--;        // ไต่ลงทีละ 1
        }
    }

    return (uint8_t)currentAngle;
}

