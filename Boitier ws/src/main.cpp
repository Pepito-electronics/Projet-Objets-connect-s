#include <Arduino.h>

//#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>   //NODERED
// #include <BlynkSimpleEsp32.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>


WiFiClientSecure gsclient;
String GAS_ID = "AKfycbwTrsV34Yi9KSkZO5fO7-K6k12g29kLfqCCe3LR2T7K27YACbl343E8z0e9Z3brdKH-EA";    //getactivespreadsheetID 
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";  //génération d'un fingerprint
const char* host = "script.google.com";  // configuration de l'hôte
const int httpsPort = 443;  // configuration du port https



const char* mqtt_server ="172.30.40.52";    //NODERED

#define WIFI_SSID "LARAS"
#define WIFI_PASS "wifi4guest"
//#define WIFI_SSID "juanito pepito"
//#define WIFI_PASS "aaaa1230"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

WiFiClient espClient;       //NODERED
PubSubClient client(espClient);      //NODERED

char ASCII_table_numbers [9] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
int score [9];
int cocktails[9];

char send_buffer [25]; 



void callback(char* topic, byte* payload, unsigned int length) {     //NODERED que faire quand un msg arrive
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
        break;
      }
    }
    break;
  
  default:
    Serial.print("Unknown Payload");
    break;
  }
  char sending_char[2];
  
  for(int i = 0; i < 9 ; i++){
    //std::string formated_string;
    sprintf(sending_char,"%i",cocktails[i]);
    strcat(send_buffer,sending_char);
    strcat(send_buffer,";");
  }
  Serial.println(send_buffer);

  for(int j = 0;j<9;j++){
    cocktails[j]=0;
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

void sendData(int x, int y, int z)  // prend comme paramètre les données que l'on souhaite envoyer, ici température, humidité et luminosité
{
  gsclient.setInsecure();   
  if (!gsclient.connect("script.google.com", httpsPort))  // on vérifie le port et le fingerprint
  {
    Serial.println("connection failed");
    return;
  }
  if (gsclient.verify(fingerprint, host))   // on vérifie le port et le fingerprint
  {
    Serial.println("certificate matches");
  } 
  else 
  {
    Serial.println("certificate doesn't match");
  }
  // x =  température, y = humidité et z = luminosité
  String string_x     =  String(x, DEC);  // conversion de x en string
  String string_y     =  String(y, DEC);  // conversion de y en string
  String string_z     =  String(z, DEC);  // conversion de z en string

  Serial.println(string_x);
  Serial.println(string_y);
  Serial.println(string_z);

  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_x + "&humidity="+ string_y + "&luminosity="+ string_z;   //création de l'url qui enverra les données sur le google sheet
  Serial.print("requesting URL: "); 
  Serial.println(url); 


  gsclient.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

  Serial.println("request sent");

  while (gsclient.connected()) {
    String line = gsclient.readStringUntil('\n');
    if (line == "\r") 
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = gsclient.readStringUntil('\n');
  Serial.println(line);
  if (line.startsWith("{\"state\":\"success\"")) 
  {
    Serial.println("CI successfull!");
  } 
  else 
  {
    Serial.println("CI has failed");
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

  
  server.on("/commande", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        // Route for root / web page
      request->send(SPIFFS, "/commande.html","text/html");
      //request->send(200, "text/html",index_html);
    });

  server.on("/actu", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        // Route for root / web page
      //request->send(SPIFFS, "/commande.html","text/html");
    //memset(send_buffer,'0',sizeof(send_buffer));
    //sprintf(send_buffer,"%f",rand_int);
    //Serial.print("test"); 
    request->send(200, "text/plain", send_buffer);
    memset(send_buffer,0,sizeof(send_buffer));
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