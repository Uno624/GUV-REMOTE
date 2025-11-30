#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


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
};SoilPacket dataRX;

    float lux,t,h,hum,tmp,ph,au;   
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

  display.clearDisplay();
}
unsigned long PoRX   = 0;
unsigned long datasoil   = 0;
unsigned long datamonitor   = 0;
unsigned long status   = 0;

byte pipeNum;

void loop() {

  unsigned long now = millis();

  screemod();

  dataTX.b1   = digitalRead(BUTTON1);
  dataTX.b2   = digitalRead(BUTTON2);
  dataTX.b3   = digitalRead(BUTTON3);
  dataTX.trig = digitalRead(TRIGGER);
  dataTX.joyX = analogRead(AIX_X);
  dataTX.joyY = analogRead(AIX_Y);

  // ส่ง payload
  bool ok = radioTX.write(&dataTX, sizeof(dataTX));
  // debug
 /* Serial.print("TX -> ");
  Serial.print("B1:");   Serial.print(dataTX.b1);
  Serial.print(" B2:");  Serial.print(dataTX.b2);
  Serial.print(" B3:");  Serial.print(dataTX.b3);
  Serial.print(" TRG:"); Serial.print(dataTX.trig);
  Serial.print(" X:");   Serial.print(dataTX.joyX);
  Serial.print(" Y:");   Serial.print(dataTX.joyY);
  Serial.print("  status:");
  Serial.print(ok ? "OK" : "FAIL \n\t");*/

 if (now - PoRX >= 50) {
  PoRX = now;
   radioRX.openReadingPipe(1, addressRX);  // GNSS
  if (radioRX.available(&pipeNum)) {
    if (pipeNum == 1) {
      radioRX.read(&dataRX, sizeof(dataRX));
    }
   }
 }

 if (now - status >= 50){
   status = now;
        static bool lastTrigARM = 0;
        static bool lastTrigResrc = 0;

        bool pressedARM = (dataTX.joyY == 0 && dataTX.trig == 1);
        bool pressedResrc = (dataTX.b2 == 1 && dataTX.trig == 1);

        if (pressedARM && !lastTrigARM) {
            dataTX.ARMstatus = !dataTX.ARMstatus;     
        }
        if (pressedResrc && !lastTrigResrc) {
            dataTX.Resrcstatus = !dataTX.Resrcstatus;     
        }
       lastTrigARM = pressedARM; // เก็บสถานะปุ่มไว้เทียบรอบหน้า
      lastTrigResrc = pressedResrc;

      if(dataTX.ARMstatus) dataTX.Resrcstatus = 0;
      if(dataTX.Resrcstatus) dataTX.ARMstatus = 0;
 }



   if (now - datamonitor >= 100){
     datamonitor = now;
     lux   = dataRX.lux   / 10.0f;
     t   = dataRX.t10   / 10.0f;
     h   = dataRX.h10   / 10.0f;
     hum = dataRX.hum10 / 10.0f;
     tmp = dataRX.tmp10 / 10.0f;
     ph  = dataRX.ph10  / 10.0f;
     au  = dataRX.au    / 10.0f;

  /*display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  //display.print(" Lat = ");  display.println(dataRX.lat);
    display.print(" Hum= "); display.println(hum);
    display.print(" Tmp= "); display.println(tmp);
    display.print(" pH = "); display.println(ph);
    display.print(" EC = "); display.println(dataRX.ec);
    display.print(" N = ");  display.println(dataRX.n);
    display.print(" P  = "); display.println(dataRX.p);
    display.print(" K  = "); display.println(dataRX.k);
    display.display(); 
    display.clearDisplay();*/

   /* float plat = dataRXPosition.lat;
    float plon = dataRXPosition.lon;
    float pau = dataRXPosition.au;
    Serial.print("RX Packet");
    Serial.print("/t lux  = "); Serial.print(lux);
    Serial.print("/t t  = "); Serial.print(t);
    Serial.print("/t h  = "); Serial.print(h);
    Serial.print("/t Hum= "); Serial.print(hum);
    Serial.print("/t Tmp= "); Serial.print(tmp);
    Serial.print("/t pH = "); Serial.print(ph);
    Serial.print("/t EC = "); Serial.print(dataRX.ec);
    Serial.print("/t N = "); Serial.print(dataRX.n);
    Serial.print("/t P  = "); Serial.print(dataRX.p);
    Serial.print("/t K  = "); Serial.print(dataRX.k);
    Serial.print("  Lat= "); Serial.print(dataRX.lat);
    Serial.print("  Lon= "); Serial.print(dataRX.lon);
    Serial.print("  Au = "); Serial.print(dataRX.au);
    Serial.print(" pipeNum = "); Serial.print(pipeNum);

    Serial.println();*/
   }
}

 void screemod(){
  display.setTextSize(1.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(" mode :");
    if(dataTX.ARMstatus){
    display.println(" ARM ");
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.print(" turn : ");
     if     (dataTX.joyY >= 1772+20){display.println(" forward ");}
     else if(dataTX.joyY <= 1772-20){display.println(" backward ");} 
     else if(dataTX.joyX >= 1772+20){display.println(" spinright ");}
     else if(dataTX.joyX <= 1772-20){display.println(" spinleft ");}
     else{display.println(" stop ");}  
    display.display(); 
    display.clearDisplay();
      }
    else if(dataTX.Resrcstatus){
    display.println(" Soilsearch ");
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.print(" Lat = ");  display.println(dataRX.lat);
    display.print(" Hum= "); display.println(hum);
    display.print(" Tmp= "); display.println(tmp);
    display.print(" pH = "); display.println(ph);
    display.print(" EC = "); display.println(dataRX.ec);
    display.print(" N = ");  display.println(dataRX.n);
    display.print(" P  = "); display.println(dataRX.p);
    display.print(" K  = "); display.println(dataRX.k);
    display.display(); 
    display.clearDisplay();
      }
    else{display.println(" idle ");
        display.setTextSize(1);
        display.setCursor(0, 15);
        display.print(" Lat = ");  display.println(dataRX.lat);
        display.print(" lon = ");  display.println(dataRX.lon);
        display.display(); 
        display.clearDisplay();
    }

 }
