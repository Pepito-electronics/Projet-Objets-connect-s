#include <Arduino.h>

//#define BLYNK_PRINT Serial

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

const char* mqtt_server ="IP_server";    //NODERED

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"


#define SS_PIN 15//D8
#define RST_PIN 16//D0
#define ledIndicator 4
#define ledDetectCard 2
#define ledDetectWrongCard 5 

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

WiFiClient espClient;       //NODERED
PubSubClient client(espClient);      //NODERED
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;

byte nuidPICC[4];

const char iDEsp = 4;
String MasterTag = "D34958A3";
String tagID = "";
bool flag = 0;

void StopReading();

void callback(char* topic, byte* payload, unsigned int length){
    for (int i = 0; i < length; i++) {
        msg[i] = payload[i];
        if ((char)msg[i] == iDEsp) {
            flag = 1;
        }        
    }
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
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    rfid.PCD_DumpVersionToSerial();
    pinMode(ledDetectCard, OUTPUT);
    pinMode(ledDetectWrongCard, OUTPUT);
    pinMode(ledIndicator, OUTPUT);

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

    if (flag == 1) {

        digitalWrite(ledIndicator, HIGH);

        // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
        if (!rfid.PICC_IsNewCardPresent())
            return;

        // Verify if the NUID has been readed
        if (!rfid.PICC_ReadCardSerial())
            return;


        if (rfid.uid.uidByte[0] != nuidPICC[0] ||
            rfid.uid.uidByte[1] != nuidPICC[1] ||
            rfid.uid.uidByte[2] != nuidPICC[2] ||
            rfid.uid.uidByte[3] != nuidPICC[3]) {
            Serial.println(F("A new card has been detected."));

            for (byte i = 0; i < 4; i++) {
                nuidPICC[i] = rfid.uid.uidByte[i];
            }

            tagID = "";
            for (uint8_t i = 0; i < 4; i++) {
                tagID.concat(String(rfid.uid.uidByte[i], HEX));
            }
            tagID.toUpperCase();

            if (tagID == MasterTag)
            {
                Serial.println(" Access Granted!");
                digitalWrite(ledDetectCard, HIGH);
                delay(1000);
                digitalWrite(ledDetectCard, LOW);
                flag = 0;
                
                snprintf(msg, MSG_BUFFER_SIZE, "%s", tagID);
                Serial.print("Publish message: ");
                Serial.println(msg);
                client.publish("ID :", msg);

            }
            else
            {
                Serial.println(" Access Denied!");
                digitalWrite(ledDetectCard, LOW);
                digitalWrite(ledDetectWrongCard, HIGH);
                delay(1000);
                digitalWrite(ledDetectWrongCard, LOW);
            }

        }
        else
        {
            Serial.println(F("Card read previously."));
            StopReading();
        }
    }
    else
    {
        digitalWrite(ledIndicator, LOW);

    }

  if (!client.connected()) {   //NODERED
    reconnect();
  }
  client.loop();  
}

void StopReading() {
    rfid.PICC_HaltA();  // Halt PICC
    rfid.PCD_StopCrypto1();     // Stop encryption on PCD
}
