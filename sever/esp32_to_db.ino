// ESP32 send data to XAMPP (PHP + MySQL)
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* serverName = ""; // เช่น http://localhost/TestInput_db.php

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);  // เชื่อมต่อไปยังไฟล์ PHP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    /*
    float lux = 2.00; // ตัวอย่างข้อมูล sensor
    float t = 60.00;
    float h = 60.00;
    float tmp = 60.00;
    float hum = 60.00;
    float ph = 60.00;
    float ec = 60.00;
    float n = 60.00;
    float p = 60.00;
    float k = 60.00;
    float lat = 60.00;
    float lon = 60.00;
    float au = 60.00;*/

    String postData = "lux=" + String(lux) + 
                      "&temperature=" + String(t) +
                      "&humidity=" + String(h) +
                      "&soil_temp=" + String(tmp) +
                      "&soil_hum=" + String(hum) +
                      "&ph=" + String(ph) +
                      "&soil_ec=" + String(ec) +
                      "&nitrogen=" + String(p) +
                      "&phosphorus=" + String(k) +
                      "&potassium=" + String(t) +
                      "&latitude=" + String(lat) +
                      "&longitude=" + String(lon) +
                      "&altitude=" + String(au);


    int httpResponseCode = http.POST(postData); // ส่งข้อมูลไปที่ php

    Serial.print("POST Data: ");
    Serial.println(postData);

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

  delay(10000); // ส่งทุก 10 วินาที
}
