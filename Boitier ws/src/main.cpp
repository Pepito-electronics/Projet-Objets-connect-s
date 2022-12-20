#include <Arduino.h>


#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>   
#include <SPIFFS.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>

const char* mqtt_server ="172.30.40.52";    //NODERED

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);


/* Variables programme */
char ASCII_table_numbers [9] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
int score [9];
int sauvegarde [9];
int cocktails[9];
char send_buffer [25]; 

/*-------------------------------------------------------------------------------*/

/* Callback for MQTT messages */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print('\n');
  switch (payload[0])
  {
    case 'C':
      for(int j = 0; j < 9 ; j++){
        if(payload[1] == ASCII_table_numbers[j]){
          Serial.println(j);
          cocktails[j]++;
          break;
        }
      }
      break;
    case 'W':
      for(int j = 0; j < 6 ; j++){
        if(payload[1] == ASCII_table_numbers[j]){
          Serial.println(j);
          score[j]++;
          sauvegarde[j]++;
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

/* Reconnect function for WIFI */
void reconnect() {     //NODERED mqqt connexion 
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += "Master";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and (re)subscribe
      client.subscribe("Game1");
      client.subscribe("Commande");
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

void setup(){
  int send_time = millis();
 //----------------------------------------------------Serial
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  /*----------------------SPIFFS-------------------------------*/
  if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }
  Serial.println("Successfully Ended SPIFFS reading");

  /* ---------------------------------------------------------------- */

  Serial.println(WiFi.localIP()); // print IP address

  client.setServer(mqtt_server,1883); // Connect as client to the MQTT broker
  client.setCallback(callback);       // Set function to call when a message arrives

  /* ------------ Sever functions ------------------------------------ */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      // Route for root / web page
      request->send(SPIFFS, "/index.html","text/html");
    });
  
  server.on("/commande", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      // Route for barman page
      request->send(SPIFFS, "/commande.html","text/html");
    });

  server.on("/actu", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      char sending_char[2];
      for(int i = 0; i < 9 ; i++){
        sprintf(sending_char,"%i",cocktails[i]);
        strcat(send_buffer,sending_char);
        strcat(send_buffer,";");
      }
      Serial.println(send_buffer);
      for(int j = 0;j<9;j++){
        cocktails[j]=0;
      }
      // actualisations orders
      request->send(200, "text/plain", send_buffer);
      memset(send_buffer,0,sizeof(send_buffer));
    });

    server.on("/scores", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      char sending_char[2];
      for(int i = 0; i < 9 ; i++){
        sprintf(sending_char,"%i",score[i]);
        strcat(send_buffer,sending_char);
        strcat(send_buffer,";");
      }
      Serial.println(send_buffer);
      for(int j = 0;j<9;j++){
        score[j]=0;
      }
      // actualisations orders
      request->send(200, "text/plain", send_buffer);
      memset(send_buffer,0,sizeof(send_buffer));
    });

  /*------------------- Launch WS -------------------------------- */
  server.begin();
}
 
void loop(){
  //Blynk.run();
  if (!client.connected()){
    reconnect();
  }
  client.loop();
}