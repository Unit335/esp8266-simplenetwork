#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

//max clients
#define MAX_SRV_CLIENTS 2
//size of received/sent json structures
#define JSON_SIZE 60

//static ip for hub
IPAddress ip(192, 168, 1,103 );
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

const char * ssid = "SSID";
const char * password = "WIFI_PASS";

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];
long pushtime = 0;

void setup() {
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, LOW);
   Serial.begin(115200);

   WiFi.config(ip, gateway, subnet); 
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   Serial.print("\nConnecting to ");
   Serial.println(ssid);
   uint8_t i = 0;
   while (WiFi.status() != WL_CONNECTED && i++ < 30) {
      Serial.print(".");
      delay(500);
   }
   if (i == 21) {
      Serial.print("Could not connect to");
      Serial.println(ssid);
      while (1) {
         delay(500);
      }
   }
   server.begin();
   
   //disable small packets merge
   server.setNoDelay(true);

   Serial.print("Cuurent IP: ");
   Serial.print(WiFi.localIP());
   Serial.println(" 23' to connect");
   pushtime = millis();
}

void loop() {
   uint8_t i;
   //check for new clients
   if (server.hasClient()) {
      for (i = 0; i < MAX_SRV_CLIENTS; i++) {
         if (!serverClients[i] || !serverClients[i].connected()) {
            if (serverClients[i]) {
               serverClients[i].stop();
            }
            serverClients[i] = server.available();
            Serial.print("New client: ");
            Serial.println(i);
            break;
         }
      }

      if (i == MAX_SRV_CLIENTS) {
         WiFiClient serverClient = server.available();
         serverClient.stop();
         Serial.println("Connection rejected ");
      }
   }

   //check for new data from clients
   for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
         if (serverClients[i].available()) {
            char json[JSON_SIZE], resend_json[JSON_SIZE];
            serverClients[i].read(json, JSON_SIZE);
            strncpy(resend_json, json, JSON_SIZE);
            StaticJsonDocument < 200 > jsonBuffer;
            DeserializationError error = deserializeJson(jsonBuffer, json);
            if (error) {
               Serial.println("Deserialization error");
               return;
            }

            const char * name = jsonBuffer["name"];
            int rand = jsonBuffer["rand"];
            double data = jsonBuffer["data"][0];

            Serial.print("Read id: ");
            Serial.println(rand);


            //if destination ip is in document sends message to this ip
            /*
            JsonVariant error = jsonBuffer["ip"];
            if (error.isNull()){
               message(URL, json);
            }
            else {}
            */

            //resends data to other client
            //not 1 => 0; not 0 -> 1
            //only for network with 2 clients
            if (serverClients[not i] && serverClients[not i].connected()) {
               Serial.print("Resending data to client ");
               Serial.println(not i);
               Serial.println(resend_json);

               int len = sizeof(resend_json);
               serverClients[not i].write(resend_json, len);
            }
         }
      }
   }
}


//accepts url and message to send to URL
void message(char url[JSON_SIZE], char json[JSON_SIZE]){
            HTTPClient http;   
            WiFiClient client;
            http.begin(client, url);     
            http.addHeader("Content-Type", "application/json");  
         
            int httpCode = http.POST(json);  
            String payload = http.getString();                 
         
            Serial.println(httpCode);   
            Serial.println(payload);   
            http.end(); 
}
