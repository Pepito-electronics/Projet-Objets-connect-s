/*
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      ESP8266      
 *             Reader/PCD        
 * Signal      Pin          Pin           
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          D1 /            
 * SPI SS      SDA(SS)      D2 /         
 * SPI MOSI    MOSI         D7 /    
 * SPI MISO    MISO         D6 /
 * SPI SCK     SCK          D5 / 
 *
 */
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <WiFiClient.h>
#include <PubSubClient.h>   
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <FastLED.h>

#define SS_PIN 4
#define RST_PIN 5

#define LED_PIN 15// GPIO4 esp8266 d2   tester autre pin
#define NB_LEDS 4

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"

#define NB_LEDS 4
#define BRIGHTNESS 50

const char* mqtt_server ="172.30.40.46";    //MQTT broker

WiFiClient espClient;
PubSubClient client(espClient);
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

String EquipeRouge = "206FB789";// 1
String EquipeBleue = "9352A52E";// 2
String EquipeVerte = "698948B8";// 3
String EquipeJaune = "2A51FE29";// 4
String EquipeOrange = "539CAD2E";  // 5
String EquipeMauve = "6F18A628";  // 6
String EquipeMarron = "8BBC459";  // 7
String EquipeRose = "9ED94F20";  // 8
String tagID = "";

// var
bool G_Started;

char ID[2] = {0x4D,0x31};
char ASCII_table_numbers [9] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
char ID_winner[2] = {0x00,0x00};

char send_buffer [25]; 

CRGB leds[NB_LEDS];

int cpt=0;
unsigned long currentMillis=0;

//Prototypes
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

void showProgramCleanUp(long delayTime);
void led(int commande);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print('\n');
  switch (payload[0])
  {
    case 'M':
      for(int j = 0; j < 9 ; j++){
        if(payload[1] == ID[1]){
          G_Started = true;
          break;
        }
      }
      break;
    default:
      Serial.print("Unknown Payload");
      break;
  }
}
/*-------------------------------------------------------------------------*/

void reconnect() {     
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += ID[1];
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and (re)subscribe
      client.subscribe("Game1");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/* ---------------------------------------------------------------------- */


void setup() { 
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP()); // print IP address

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NB_LEDS);  
  //FastLED.setMaxPowerInVoltsAndMilliamps(3, 100);
  //FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  //led(10);

  client.setServer(mqtt_server,1883); // Connect as client to the MQTT broker
  client.setCallback(callback);       // Set function to call when a message arrives

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
}
 
void loop() {

  if (!client.connected()){
    reconnect();
  }
  
  if(G_Started){
    if(currentMillis<millis()){
      FastLED.show();
      leds[cpt] = CRGB::Blue;
      Serial.print("test");
      FastLED.show();
      //delay(500);
      leds[cpt] = CRGB::Black;
      cpt+=1;
      currentMillis=millis()+1000;
      if(cpt==4){
        cpt=0;
      }
      Serial.print(cpt);
    }
    if ( ! rfid.PICC_IsNewCardPresent())
    return;
    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;

    if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3] ) 
    {
      Serial.println(F("A new card has been detected."));
      // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
    
      Serial.println(F("The NUID tag is:"));
      Serial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      Serial.println();
      tagID = "";
      for ( uint8_t i = 0; i < 4; i++) {                  // The MIFARE tag in use has a 4 byte UID
        tagID.concat(String(rfid.uid.uidByte[i], HEX));
        // Adds the 4 bytes in a single string variable
      }
      tagID.toUpperCase();

      if(tagID==EquipeRouge){
        client.publish("Game1","W1");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID == EquipeVerte){
        client.publish("Game1","W2");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID == EquipeBleue){
        client.publish("Game1","W3");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID==EquipeJaune){
        client.publish("Game1","W4");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID==EquipeOrange){
        client.publish("Game1","W5");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID==EquipeMauve){
        client.publish("Game1","W6");
        led(3);
        //led(3);
        G_Started = false;
      }
      else if(tagID==EquipeMarron){
        client.publish("Game1","W7");
        led(3);
        //led(3);
        G_Started = false;
      } 
    }
  }
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  client.loop();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}


void led(int commande){
 
  switch(commande){
     
    case 1://ledRed
      for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::Red;
      }
      FastLED.show();
      break;
   
    case 2://ledBlue
       for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::Blue;
      }
      FastLED.show();
      break;
     
    case 3://ledgreen
    FastLED.show();
       for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::Green;
      }
      FastLED.show();
      break;
     
    case 4://ledYellow
       for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::Yellow;
      }
      FastLED.show();
      break;
     
    case 6://ledOrange
      for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::Orange;
      }
      FastLED.show();
      break;
    case 7://ledWhite
      for (int i = 0; i < NB_LEDS; ++i) {
        leds[i] = CRGB::White;
      }
      FastLED.show();
      break;
           
    default:
      for (int i=0; i<NB_LEDS; i++ )
        leds[i] = CRGB(0, 0, 0 );
      FastLED.show();
      break;
  }
}

void showProgramCleanUp(long delayTime) {
  for (int i = 0; i < NB_LEDS; ++i) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}