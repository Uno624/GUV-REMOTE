#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "UMA";
const char* password = "0836068832";
const char* serverName = "http://10.179.254.128/datasensor.php"; // เช่น http://localhost/TestInput_db.php


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// RF24(CE, CSN)
RF24 radioTX(TXCE, TXCSN);
RF24 radioRX(RXCE, RXCSN);

// ที่อยู่ (ต้องตรงกับฝั่ง RX)
const byte addressTX[6] = "Artur";

const byte addressRX[6] = "John";
const byte addressRXPos[6] = "JohnP";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool ResrcAir;

// payload ที่จะส่ง
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
Payload dataTX;

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

};SoilPacket dataRX;

    float lux,t,h,hum,tmp,ph,ec,n,k,p,au;   
/*
struct GNSSPayload {
  float    lat;
  float    lon;
  int16_t  au;
};
GNSSPayload dataRXPosition;*/

void setup() {
  delay(150);
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // --- ปุ่ม ---
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT_PULLDOWN);
  pinMode(TRIGGER, INPUT_PULLDOWN);

  // --- จอย ---
  pinMode(AIX_X, INPUT);
  pinMode(AIX_Y, INPUT);
  analogReadResolution(12);   // 0–4095

  // --- SPI ---
  SPI.begin(SCK, MISO, MOSI);
  SPI.setBitOrder(MSBFIRST);

  pinMode(TXCSN, OUTPUT); digitalWrite(TXCSN, HIGH); 
  pinMode(RXCSN, OUTPUT); digitalWrite(RXCSN, HIGH);
  pinMode(TXCE,  OUTPUT); digitalWrite(TXCE, LOW);   
  pinMode(RXCE,  OUTPUT); digitalWrite(RXCE, LOW);

  // --- RF24TX ---
  radioTX.begin();              // ใช้ SPI global ที่เรา begin แล้ว
  radioTX.setChannel(100);      // 0–125 ต้องเท่ากับฝั่งรับ
  radioTX.setDataRate(RF24_1MBPS);
  radioTX.setPALevel(RF24_PA_LOW);
  radioTX.openWritingPipe(addressTX);
  radioTX.stopListening();      // โหมดส่ง
    // --- RF24RX ---
  radioRX.begin();              // ใช้ SPI global ที่เรา begin แล้ว
  //radioRX.enableDynamicPayloads();
  radioRX.setChannel(102);      // 0–125 ต้องเท่ากับฝั่งรับ
  radioRX.setDataRate(RF24_1MBPS);
  radioRX.setPALevel(RF24_PA_LOW);
  radioRX.openReadingPipe(1, addressRX);
  radioRX.startListening(); 

 initWiFi();
  display.clearDisplay();
}

unsigned long PoRX   = 0;
unsigned long DTSED   = 0;
unsigned long datasoil   = 0;
unsigned long datamonitor   = 0;
unsigned long status   = 0;

byte pipeNum;
bool TXok;

void loop() {

  unsigned long now = millis();
  
  screemod();

  dataTX.b1   = digitalRead(BUTTON1);
  dataTX.b2   = digitalRead(BUTTON2);
  dataTX.b3   = digitalRead(BUTTON3);
  dataTX.trig = digitalRead(TRIGGER);
  dataTX.joyX = analogRead(AIX_X);
  dataTX.joyY = analogRead(AIX_Y);

  Serial.print("b1 "); Serial.print(dataTX.b1);
  Serial.print(" b2 "); Serial.print(dataTX.b2);
  Serial.print(" b3 "); Serial.print(dataTX.b3);
  Serial.print(" trig "); Serial.print(dataTX.trig);
  Serial.print(" joyX "); Serial.print(dataTX.joyX);
  Serial.print(" njoyY "); Serial.println(dataTX.joyY);


  // ส่ง payload
  TXok = radioTX.write(&dataTX, sizeof(dataTX));

 if (now - PoRX >= 50) {
  PoRX = now;
   radioRX.openReadingPipe(1, addressRX);  // GNSS
  if (radioRX.available(&pipeNum)) {
    if (pipeNum == 1) {
      radioRX.read(&dataRX, sizeof(dataRX));
     lux   = dataRX.lux   / 10.0f;
     t   = dataRX.t10   / 10.0f;
     h   = dataRX.h10   / 10.0f;
     hum = dataRX.hum10 / 100.0f;
     tmp = dataRX.tmp10 / 100.0f;
     ph  = dataRX.ph10  / 100.0f;
     ec  = dataRX.ec    / 10.0f;
     n   = dataRX.n     / 10.0f;
     p   = dataRX.p     / 10.0f;
     k   = dataRX.k     / 10.0f;
     au  = dataRX.au;
    }
   }

 }

 if (now - status >= 50){
   status = now;
        static bool lastTrigARM = 0;
        static bool lastTrigResrc = 0;
        static bool lastTrigResrcAir = 0;

        bool pressedARM = (dataTX.joyY == 0 && dataTX.trig == 1);
        bool pressedResrc = (dataTX.b2 == 1 && dataTX.trig == 1);
         bool pressedResrcAir;

          if(dataTX.Resrcstatus == 1) {
          pressedResrcAir = dataTX.trig;
              if (pressedResrcAir && !lastTrigResrcAir) {
            ResrcAir = !ResrcAir;
           }
          }

        if (pressedARM && !lastTrigARM) {
            dataTX.ARMstatus = !dataTX.ARMstatus;     
        }
        if (pressedResrc && !lastTrigResrc) {
            dataTX.Resrcstatus = !dataTX.Resrcstatus;     
        }
       lastTrigARM = pressedARM; // เก็บสถานะปุ่มไว้เทียบรอบหน้า
      lastTrigResrc = pressedResrc;
      lastTrigResrcAir = pressedResrcAir;

      if(dataTX.ARMstatus) dataTX.Resrcstatus = 0;
      if(dataTX.Resrcstatus) dataTX.ARMstatus = 0;
 }
}

void drawArmMode() {
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(" turn : ");

  if      (dataTX.joyY >= 1772 + 20) display.println(" forward ");
  else if (dataTX.joyY <= 1772 - 20) display.println(" backward ");
  else if (dataTX.joyX >= 1772 + 20) display.println(" spinleft ");
  else if (dataTX.joyX <= 1772 - 20) display.println(" spinright ");
  else                               display.println(" stop ");

  display.print(" speed : ");
  display.print(dataRX.speed);
  display.println(" km/h ");

  display.setCursor(0, 55);
  display.print(" TX ");
  display.println(TXok ? "LIVE" : "LOST");
}

void drawSoilMode() {
  display.setTextSize(1);
  display.setCursor(0, 15);
  //display.print(" Lat = ");  display.println(dataRX.lat);
  display.print(" Hum= "); display.println(hum);
  display.print(" Tmp= "); display.println(tmp);
  display.print(" pH = "); display.println(ph);
  display.print(" EC = "); display.println(dataRX.ec);

  display.setCursor(70, 15);
  display.print(" N = "); display.println(dataRX.n);
  display.setCursor(70, 23);
  display.print(" P = "); display.println(dataRX.p);
  display.setCursor(70, 31);
  display.print(" K = "); display.println(dataRX.k);

  display.setCursor(0, 55);
  display.print(" TX ");
  display.println(TXok ? "LIVE" : "LOST");
}

void drawAirMode() {
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(" LUX = "); display.println(lux);
  display.print(" Hum = "); display.println(h);
  display.print(" Tmp = "); display.println(t);

  display.setCursor(0, 55);
  display.print(" TX ");
  display.println(TXok ? "LIVE" : "LOST");
}

void drawIdleMode() {
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(" Lat : ");  display.println(dataRX.lat, 6);
  display.print(" Lon : ");  display.println(dataRX.lon, 6);

  display.setCursor(0, 55);
  display.print(" TX ");
  display.println(TXok ? "LIVE" : "LOST");
}

// ฟังก์ชันหลัก
void screemod() {
  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);              // เดิม 1.8 → ใช้ 2 แทน
  display.setCursor(0, 0);
  display.print(" mode :");

  if (dataTX.ARMstatus) {
    display.println(" ARM ");
    drawArmMode();

  } else if (dataTX.Resrcstatus && ResrcAir == 0) {
    display.println(" ResrcSoil ");
        unsigned long now = millis();
            if(now - DTSED >= 100){
       if (dataTX.b1 == 1){
    DTS();
   }
  }
    drawSoilMode();

  } else if (ResrcAir == 1) {
    display.println(" ResrcAir ");
    unsigned long now = millis();
    if(now - DTSED >= 100){
      DTSED = now;
       if (dataTX.b1 == 1){
    DTS();
     }
    }
    drawAirMode();

  } else {
    // idle
    display.println(" idle ");
    drawIdleMode();
  }

  display.display();
}
 
 void DTS(){
   if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;

  // สร้าง URL แบบ GET
  String url = String(serverName) + "?lux=" + String(lux) +
               "&airtmp="  + String(t) +
               "&airhum="  + String(h) +
               "&soilhum=" + String(hum) +
               "&soiltmp=" + String(tmp) +
               "&soilph="  + String(ph) +
               "&soilec="  + String(ec) +
               "&soiln="   + String(n) +
               "&soilp="   + String(p) +
               "&soilk="   + String(k) +
               "&lat="     + String(dataRX.lat, 6) +
               "&lon="     + String(dataRX.lon, 6) +
               "&au="      + String(au);

  Serial.print("GET URL: ");
  Serial.println(url);

  http.begin(url);

  // ส่ง GET
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Server response: " + response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
} else {
  Serial.println("WiFi Disconnected");
}

 }

 void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}
