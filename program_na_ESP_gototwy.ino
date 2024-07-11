#include "Wire.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Dane do połączenia WiFi
const char* ssid = "808";
const char* password = "leadropesolo";

// URL do API
const char* serverNameCreatePath = "http://MuodyGri.pythonanywhere.com/post_path/1";
const char* serverNamePostPoint = "http://MuodyGri.pythonanywhere.com/post_point";

// Zmienna do przechowywania ID utworzonej ścieżki
int pathID = 0;

// Okres czasu w milisekundach do wysyłania punktów (np. 10 sekund)
const long interval = 500;


#define TCAADDR 0x70

#define AS5600_ADDR 0x36
#define ZPOS_HIGH 0x01
#define ZPOS_LOW 0x02
#define MPOS_HIGH 0x03
#define MPOS_LOW 0x04
#define MANG_HIGH 0x05
#define MANG_LOW 0x06

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  //Serial.begin(115200);
  delay(1000);

  //Serial.println("\nConfiguring AS5600 encoders through TCA9548A...");

  for (uint8_t channel = 0; channel < 8; channel++) {
    tcaselect(channel);
    //Serial.print("Configuring TCA Port #"); Serial.println(channel);

    // Set zero position
    writeAS5600Register(ZPOS_HIGH, 0x00);
    writeAS5600Register(ZPOS_LOW, 0x00);

    // Set maximum position (for full range)
    writeAS5600Register(MPOS_HIGH, 0x0F); // High byte of 4095
    writeAS5600Register(MPOS_LOW, 0xFF);  // Low byte of 4095

    // Set maximum angle (for 360 degrees)
    writeAS5600Register(MANG_HIGH, 0x0F); // High byte of 4095
    writeAS5600Register(MANG_LOW, 0xFF);  // Low byte of 4095

    //Serial.println("Configured AS5600 for 360-degree range.");
  }
  //Serial.println("\nConfiguration done.");
  delay(1000);
  // Połączenie z WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    //Serial.println("Łączenie z WiFi...");
  }
  //Serial.println("Połączono z WiFi!");

  // Utworzenie nowej ścieżki
  createNewPath();
}

void loop() {
  
  uint8_t channel = 3;
  tcaselect(channel);
  //Serial.print("Reading TCA Port #"); Serial.println(channel);
  int iangle4 = readAS5600Angle();
  double dangle4 = iangle4;
  double angle4 = dangle4/4096*360-215;
  if (iangle4 != -1) {
    //Serial.print("AS5600 Angle: ");
    //Serial.println(angle4);
  } else {
    //Serial.println("Error reading AS5600 angle");
  }


  channel = 2;
  tcaselect(channel);
  //Serial.print("Reading TCA Port #"); Serial.println(channel);
  int iangle3 = readAS5600Angle();
  double dangle3 = iangle3;
  double angle3 = -dangle3/4096*360-120+247.17;
  if (iangle3 != -1) {
    //Serial.print("AS5600 Angle: ");
    //Serial.println(angle3);
  } else {
    //Serial.println("Error reading AS5600 angle");
  }


  channel = 1;
  tcaselect(channel);
  //Serial.print("Reading TCA Port #"); Serial.println(channel);
  int iangle2 = readAS5600Angle();
  double dangle2 = iangle2;
  double angle2 = dangle2/4096*360-320;
  if (iangle2 != -1) {
    //Serial.print("AS5600 Angle: ");
    //Serial.println(angle2);
  } else {
    //Serial.println("Error reading AS5600 angle");
  }


  channel = 0;
  tcaselect(channel);
  //Serial.print("Reading TCA Port #"); Serial.println(channel);
  int iangle1 = readAS5600Angle();
  double dangle1 = iangle1;
  double angle1 = dangle1/4096*360;
  if (iangle1 != -1) {
    //Serial.print("AS5600 Angle: ");
    //Serial.println(angle1);
  } else {
    //Serial.println("Error reading AS5600 angle");
  }


  postPoint(angle1,angle2,angle3,angle4);
  delay(10); // Adjust the delay as needed
}

void writeAS5600Register(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

int readAS5600Angle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0E); // High byte of the raw angle
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 2);

  int angle = -1;
  if (Wire.available() == 2) {
    angle = Wire.read() << 8;
    angle |= Wire.read();
  }

  return angle;
}

void createNewPath() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverNameCreatePath);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST("");

    if (httpResponseCode > 0) {
      String response = http.getString();
      //Serial.println("Odpowiedź serwera: " + response);

      // Spróbuj przekonwertować odpowiedź na liczbę całkowitą
      int pathId = response.toInt();

      // Sprawdź, czy udało się przekonwertować na liczbę
      if (pathId != 0) {
        pathID = pathId;
        //Serial.println("Utworzona ścieżka o ID: " + String(pathID));
      } else {
        //Serial.println("Błąd: Odpowiedź serwera nie zawiera poprawnego ID ścieżki");
      }
    } else {
      //Serial.println("Błąd podczas tworzenia ścieżki, kod: " + String(httpResponseCode));
      String response = http.getString();
      //Serial.println("Odpowiedź serwera: " + response);
    }
    http.end();
  } else {
    //Serial.println("Brak połączenia z WiFi");
  }
}

void postPoint(double a1, double a2, double a3, double a4) {
  if (WiFi.status() == WL_CONNECTED && pathID > 0) {
    HTTPClient http;
    String serverPath = String(serverNamePostPoint) + "/" + String(pathID);
    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");

    // Tworzenie obiektu JSON
    DynamicJsonDocument doc(256);
    doc["a1"] = a1;
    doc["a2"] = a2;
    doc["a3"] = a3;
    doc["a4"] = a4;

    // Serializacja do formatu JSON
    String jsonData;
    serializeJson(doc, jsonData);

    // Wysłanie danych JSON na serwer
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      //Serial.println("Odpowiedź serwera: " + response);
    } else {
      //Serial.println("Błąd podczas wysyłania punktu, kod: " + String(httpResponseCode));
    }
    http.end();
  } else {
    //Serial.println("Brak połączenia z WiFi lub nie utworzono ścieżki");
  }
}
