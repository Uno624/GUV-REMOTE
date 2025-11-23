#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <DHT22.h>
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <ModbusMaster.h>  
#include <RF24.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define pinDATA 14

#define uart1_rx 18
#define uart1_tx 17

#define RS485_DE_RE_PIN 6   // ขา Enable ส่ง/รับ ของ MAX485 (DE + RE)
#define RS485_RX_PIN    19  // RO จาก MAX485 → RX2
#define RS485_TX_PIN    20  // DI จาก ESP32 → TX2

#define SCK 12
#define MISO 13
#define MOSI 11

#define TXCSN  46
#define TXCE   10

#define RXCSN  42
#define RXCE   41

RF24 radioTX(TXCE, TXCSN);
RF24 radioRX(RXCE, RXCSN);

const byte addressTX[6] = "John";
const byte addressRX[6] = "Artur";


struct __attribute__((packed)) SoilPacket {
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

struct Payload {
  uint8_t  b1;
  uint8_t  b2;
  uint8_t  b3;
  uint8_t  trig;
  uint16_t joyX;
  uint16_t joyY;
};
Payload dataRX;

TinyGPSPlus gps;
HardwareSerial ss(1);

BH1750 lightMeter;

DHT22 dht22(pinDATA); 

HardwareSerial &RS485Serial = Serial2;
ModbusMaster node;

// ====== ควบคุม DE/RE สำหรับ half-duplex ======
void preTransmission()
{
  digitalWrite(RS485_DE_RE_PIN, HIGH); // เข้าโหมดส่ง
}

void postTransmission()
{
  digitalWrite(RS485_DE_RE_PIN, LOW);  // กลับมาโหมดรับ
}

void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, uart1_rx, uart1_tx, false);
  SPI.begin(SCK, MISO, MOSI);
  SPI.setBitOrder(MSBFIRST);
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(5, 4);
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use
  // Wire.begin(D2, D1);
  lightMeter.begin();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // เริ่มที่โหมดรับ

  // เริ่ม UART2 สำหรับ RS485
  // Baud rate: 4800, 8N1 ตามที่ Mart ให้มา
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
  radioTX.setChannel(102);          // ช่อง 0–125
  radioTX.setDataRate(RF24_1MBPS);  
  radioTX.setPALevel(RF24_PA_LOW);  
  radioTX.openWritingPipe(addressTX); 
  radioTX.stopListening();        //โหมดส่ง   

 /*   if (!radioRX.begin()) {
    Serial.println("RadioRX not responding!");
    while (1) { delay(1000); }
  }*/
  radioRX.setChannel(100);          // ช่อง 0–125
  radioRX.setDataRate(RF24_1MBPS);  
  radioRX.setPALevel(RF24_PA_LOW);  
  radioRX.openReadingPipe(1, addressRX); 
  radioRX.startListening();        //โหมดส่ง   

  Serial.println(F("Test begin"));
}
float lux ;
unsigned long lastModbus = 0;
unsigned long lastDHT    = 0;
unsigned long lastSend   = 0;

void loop() {
  unsigned long now = millis();

  // ===== 1) อ่าน Modbus ทุก 1000 ms =====
  if (now - lastModbus >= 1000) {
    lastModbus = now;

    node.clearResponseBuffer();
    uint8_t result = node.readHoldingRegisters(0x0000, 7);

    if (result == node.ku8MBSuccess) {
      dataTX.hum10 = (int16_t) (node.getResponseBuffer(0)*10);
      dataTX.tmp10 = (int16_t) (node.getResponseBuffer(1)*10);
      dataTX.ec    = node.getResponseBuffer(2);
      dataTX.ph10  = (int16_t) (node.getResponseBuffer(3)*10);
      dataTX.n     = node.getResponseBuffer(4);
      dataTX.p     = node.getResponseBuffer(5);
      dataTX.k     = node.getResponseBuffer(6);
    } else {
      // ถ้าอยาก debug error code ก็ print ตรงนี้ได้
      // Serial.print("Modbus err = "); Serial.println(result);
    }
  }

  // ===== 2) อ่าน DHT22 ทุก 2 วินาที =====
  if (now - lastDHT >= 2000) {
    lastDHT = now;
    dataTX.t10 = (int16_t)(dht22.getTemperature() * 10);
    dataTX.h10 = (int16_t)(dht22.getHumidity() * 10);
  }

  // ===== 3) อ่าน BH1750 ตลอด (ถ้า measurementReady) =====
  if (lightMeter.measurementReady()) {
    lux = lightMeter.readLightLevel();
  }

  // ===== 4) GPS: อ่านเท่าที่มี =====
  while (ss.available() > 0) {
    gps.encode(ss.read());
    dataTX.lat = gps.location.lat();
    dataTX.lon = gps.location.lng();
    dataTX.au  = gps.altitude.meters();
  }

  // ===== 5) รับจอยจาก RF24 ตัวอื่น =====
  if (radioRX.available()) {
    radioRX.read(&dataRX, sizeof(dataRX));

    Serial.print("RX <- ");
    Serial.print("B1:");   Serial.print(dataRX.b1);
    Serial.print(" B2:");  Serial.print(dataRX.b2);
    Serial.print(" B3:");  Serial.print(dataRX.b3);
    Serial.print(" TRG:"); Serial.print(dataRX.trig);
    Serial.print(" X:");   Serial.print(dataRX.joyX);
    Serial.print(" Y:");   Serial.println(dataRX.joyY);
  }

  // ===== 6) ส่ง RF24 ทุก 100 ms (10 Hz) =====
  if (now - lastSend >= 100) {
    lastSend = now;

    bool ok = radioTX.write(&dataTX, sizeof(dataTX));

    Serial.print("size ");
    Serial.print(sizeof(dataTX));
    Serial.print("  lux = ");
    Serial.print(lux);
    Serial.print("  status: ");
    Serial.println(ok ? "OK" : "FAIL");
  }
}


