#include <Arduino.h>

//#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>   //NODERED
// #include <BlynkSimpleEsp32.h>
#include <SPIFFS.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>



const char* mqtt_server ="172.30.40.60";    //NODERED

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"
//#define WIFI_SSID "juanito pepito"
//#define WIFI_PASS "aaaa1230"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

WiFiClient espClient;       //NODERED
PubSubClient client(espClient);      //NODERED


void callback(char* topic, byte* payload, unsigned int length) {     //NODERED que faire quand un msg arrive
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == 'W') {    //victoire jeu
    for(int j = 0; j < 12 ;j++){
      if(payload[0])
    }
  }
  //------------------------ commandes ----------------------------
  if ((char)payload[0] == 'M') {    

  }
  if ((char)payload[0] == 'F') {

  }
  if ((char)payload[0] == 'C') {

  } 
  //ajouter autres
  //---------------------------------------------------------------
  else
  {
    Serial.print("Unknown Payload");
  }
}

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
      // ... and resubscribe
      client.subscribe("Game1");
      client.subscribe("Commande");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){
  int send_time=millis();

 //----------------------------------------------------Serial
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

   //----------------------------------------------------SPIFFS
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

  Serial.println("Successfully Ended");

  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server,1883);    //NODERED
  client.setCallback(callback);         //NODERED

  //----------------------------------------------------SERVER

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
      // Route for root / web page
    request->send(SPIFFS, "/index.html","text/html");
    //request->send(200, "text/html",index_html);
  });

  server.begin();
}
 
void loop(){
  //Blynk.run();
  if (!client.connected()) {   //NODERED
    reconnect();
  }
  client.loop();      //NODERED
 
}