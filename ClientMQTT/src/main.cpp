#include <Arduino.h>

//#define BLYNK_PRINT Serial

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>

const char* mqtt_server ="IP_server";    //NODERED

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

WiFiClient espClient;       //NODERED
PubSubClient client(espClient);      //NODERED
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

void callback(char* topic, byte* payload, unsigned int length){

}


void reconnect() {     //NODERED mqqt connexion 
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("_");
      client.subscribe("__");
      client.subscribe("___");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  client.setServer(mqtt_server,1883);    //NODERED
  client.setCallback(callback);         //NODERED 
}

void loop() {
  // put your main code here, to run repeatedly:

  if (!client.connected()) {   //NODERED
    reconnect();
  }
  client.loop();  
}