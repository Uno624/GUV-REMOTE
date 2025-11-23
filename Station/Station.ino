#include <SPI.h>
#include <RF24.h>

#define SCK 18
#define MISO 19
#define MOSI 23

#define TXCSN 33
#define TXCE 32

#define RXCSN 27
#define RXCE 14

#define AIX_Y 36
#define AIX_X 39

#define BUTTON1 34
#define BUTTON2 35
#define BUTTON3 26

#define TRIGGER 25

// RF24(CE, CSN)
RF24 radioTX(TXCE, TXCSN);
RF24 radioRX(RXCE, RXCSN);

// ที่อยู่ (ต้องตรงกับฝั่ง RX)
const byte addressTX[6] = "Artur";
const byte addressRX[6] = "John";

// payload ที่จะส่ง
struct Payload {
  uint8_t  b1;
  uint8_t  b2;
  uint8_t  b3;
  uint8_t  trig;
  uint16_t joyX;
  uint16_t joyY;
};
Payload dataTX;

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
};SoilPacket dataRX;

  char text[32] = {0};             // buffer ขนาด 32 bytes

void setup() {
  delay(150);
  Serial.begin(115200);

  // --- ปุ่ม ---
  pinMode(BUTTON1, INPUT_PULLDOWN);
  pinMode(BUTTON2, INPUT_PULLDOWN);
  pinMode(BUTTON3, INPUT_PULLDOWN);
  pinMode(TRIGGER, INPUT_PULLDOWN);

  // --- จอย ---
  pinMode(AIX_X, INPUT);
  pinMode(AIX_Y, INPUT);
  analogReadResolution(12);   // 0–4095

  // --- SPI ---
  SPI.begin(SCK, MISO, MOSI);
  SPI.setBitOrder(MSBFIRST);

  /*pinMode(RXCE, OUTPUT);  digitalWrite(RXCE, LOW);
  pinMode(RXCSN, OUTPUT); digitalWrite(RXCSN, HIGH);*/

  // --- RF24TX ---
  radioTX.begin();              // ใช้ SPI global ที่เรา begin แล้ว
  radioTX.setChannel(100);      // 0–125 ต้องเท่ากับฝั่งรับ
  radioTX.setDataRate(RF24_1MBPS);
  radioTX.setPALevel(RF24_PA_LOW);
  radioTX.openWritingPipe(addressTX);
  radioTX.stopListening();      // โหมดส่ง
    // --- RF24RX ---
  radioRX.begin();              // ใช้ SPI global ที่เรา begin แล้ว
  radioRX.setChannel(102);      // 0–125 ต้องเท่ากับฝั่งรับ
  radioRX.setDataRate(RF24_1MBPS);
  radioRX.setPALevel(RF24_PA_LOW);
  radioRX.openReadingPipe(1, addressRX);
  radioRX.startListening(); 
}

void loop() {
  // อ่านจอย
  int rawX = analogRead(AIX_X);   // 0–4095
  int rawY = analogRead(AIX_Y);

  // scale ลงเหลือ 10 บิต (0–1023)
  uint16_t joyX = map(rawX, 0, 4095, 0, 1023);
  uint16_t joyY = map(rawY, 0, 4095, 0, 1023);

  // อ่านปุ่ม
  dataTX.b1   = digitalRead(BUTTON1);
  dataTX.b2   = digitalRead(BUTTON2);
  dataTX.b3   = digitalRead(BUTTON3);
  dataTX.trig = digitalRead(TRIGGER);
  dataTX.joyX = joyX;
  dataTX.joyY = joyY;

  // ส่ง payload
  bool ok = radioTX.write(&dataTX, sizeof(dataTX));

  // debug
  Serial.print("TX -> ");
  Serial.print("B1:");   Serial.print(dataTX.b1);
  Serial.print(" B2:");  Serial.print(dataTX.b2);
  Serial.print(" B3:");  Serial.print(dataTX.b3);
  Serial.print(" TRG:"); Serial.print(dataTX.trig);
  Serial.print(" X:");   Serial.print(dataTX.joyX);
  Serial.print(" Y:");   Serial.print(dataTX.joyY);
  Serial.print("  status:");
  Serial.print(ok ? "OK" : "FAIL \n\t");

  delay(20); // ส่งประมาณ 50 Hz

if (radioRX.available()) {
    radioRX.read(&dataRX, sizeof(dataRX));

    float t   = dataRX.t10   / 10.0f;
    float h   = dataRX.h10   / 10.0f;
    float hum = dataRX.hum10 / 10.0f;
    float tmp = dataRX.tmp10 / 10.0f;
    float ph  = dataRX.ph10  / 10.0f;
    float au  = dataRX.au    / 10.0f;

    Serial.print("RX Packet");
    Serial.print("/t t  = "); Serial.print(t);
    Serial.print("/t h  = "); Serial.print(h);
    Serial.print("/t Hum= "); Serial.print(hum);
    Serial.print("/t Tmp= "); Serial.print(tmp);
    Serial.print("/t pH = "); Serial.print(ph);
    Serial.print("/t EC = "); Serial.print(dataRX.ec);
    Serial.print("/t N = "); Serial.print(dataRX.n);
    Serial.print("/t P  = "); Serial.print(dataRX.p);
    Serial.print("/t K  = "); Serial.print(dataRX.k);
    Serial.print("/t Lat= "); Serial.print(dataRX.lat, 6);
    Serial.print("/t Lon= "); Serial.print(dataRX.lon, 6);
    Serial.print("/t Au = "); Serial.print(au);
    Serial.println();
  }
}
