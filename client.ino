#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>

//delay for sending information
#define DELAY 8000
//size of received/sent json structures
#define JSON_SIZE 60

IPAddress ip(192, 168, 1, 103);
const char * ssid = "ROSTELECOM_A2E6";
const char * password = "4N3R7K6D";

int pushtime = 0;
const int Port = 23;
short Status = 0;

WiFiClient client;

void setup() {
   delay(5000);
   while (!Serial) {
     ;
   }
   
   Serial.begin(115200);
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, LOW);
   
   WiFi.begin(ssid, password);
   
   Serial.print("\nConnecting to ");
   Serial.println(ssid);
   uint8_t i = 0;
   while (WiFi.status() != WL_CONNECTED && i++ < 20) {
      delay(500);
      Serial.print(".");
   }
   if (i == 21) {
      Serial.print("Could not connect to ");
      Serial.println(ssid);
      while (1) {
         delay(500);
      }
   }

   Serial.print("Connected, device IP: ");
   Serial.println(WiFi.localIP());
}

void loop() {
   if (Status == 0 && WiFi.status() == WL_CONNECTED) {
      Status = 1;
      client.connect(ip, Port);
      Serial.println("Connected to server");
   }

   //sends packed if enough time passed since previous packet
   if (millis() - pushtime >= DELAY) {
      pushtime = millis();


      //placeholder; should read data from connected sensor etc
      int num = rand() % 130;
      DynamicJsonDocument jsonBuffer(256);
      char json[JSON_SIZE];
      jsonBuffer["name"] = "test";
      jsonBuffer["rand"] = num;
      JsonArray data = jsonBuffer.createNestedArray("data");
      data.add(1.130);
      data.add(2.038);
      serializeJson(jsonBuffer, json);
      
      Serial.print("Sending data: ");
      Serial.println(json);
      client.write(json, sizeof(json));
   }

   //reads incomineg packets
   if (client.available()) {
      char json[JSON_SIZE];
      client.read(json, JSON_SIZE);
      StaticJsonDocument<200> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, json);
      if (error) {
         Serial.println("Deserialization error");
         return;
      }

      //placeholder for incoming data use
      const char * name = jsonBuffer["name"];
      int rand = jsonBuffer["rand"];
      double data = jsonBuffer["data"][0];
      Serial.print("Received data id: ");
      Serial.println(rand);
   }

   if (!client.status()) {
      Status = 0;
   }
}
