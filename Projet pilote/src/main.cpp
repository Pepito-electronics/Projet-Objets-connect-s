#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Littlefs.h>

#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>

const char* mqtt_server ="IP_server";    //MQTT

AsyncWebServer server(80);
WiFiClient espClient;                //MQTT
PubSubClient client(espClient);      //MQTT

const char* ssid = "LARAS";
const char* password = "wifi4guest";

const char* PARAM_MESSAGE = "message";

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
      client.subscribe("test");
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

const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html> 
<html>
  <head>
    <title>Projet Objet connectés</title>  
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
   <meta name="viewport" content="width=device-width,initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <style>
      h1 {font-family:monospace;text-align:center}
      h4 {font-family:cursive;font-style:italic}
      ul {font-family:tahoma;font-weight:normal;font-style:italic;font-size:14px;line-height:20px}
      hr {border: 3px solid green;}
      p {font-family:tahoma}
      span {font-family:tahoma;line-height:35px}
      div {text-align:center}
      button {font-size:20px}
      .mySlides {display:none;}
    </style>
  </head>
  <body class="w3-animate-opacity">
    <!-- Sidebar -->
<div class="w3-sidebar w3-bar-block w3-card w3-animate-left" style="display:none" id="mySidebar">
  <button onclick="w3_close()" class="w3-bar-item w3-large w3-hover-grey">Close &times;</button>
  <a href="#infos" class="w3-bar-item w3-button w3-hover-grey">infos pratiques</a>
  <a href="#data" class="w3-bar-item w3-button w3-hover-grey">DATA</a>
  <a href="#ill" class="w3-bar-item w3-button w3-hover-grey">Illustrations</a>
</div>
<!-- Page Content -->
 <div id="main" style="text-align:left">
<div class="w3-blue-grey" style = "text-align:left">
  <button class="w3-button w3-blue-grey w3-xlarge" onclick="w3_open()">☰</button>
    <header class="w3-container w3-blue-grey">
       <h1 class="w3-animate-top"> Cocktail des électroniciens </h1>
    </header>
  </div>
<script>
function w3_open() {
  document.getElementById("main").style.marginLeft = "25%";
  document.getElementById("mySidebar").style.width = "25%";
  document.getElementById("mySidebar").style.display = "block";
  document.getElementById("openNav").style.display = 'none';
}
function w3_close() {
  document.getElementById("main").style.marginLeft = "0%";
  document.getElementById("mySidebar").style.display = "none";
  document.getElementById("openNav").style.display = "inline-block";
}
</script>

    <h4 class="w3-animate-fading"> Résultats du concours </h4>
    <h2 id="infos"> Règles du jeu :</h2>
    <ul>
    	<p>Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.<p/>
        <p>Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.<p/>
      <li>Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.
      <li> Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.
    </ul>
    <br>
    <fieldset class="w3-pale-yellow">
      <legend id="data" style = "text-align:center;font-size:25px"> SCORES </legend>
      <span style="size:20"> Equipe 1 </span><span id="capteur_t";style="color:green;size:20;"></span>
      <br>
      <span style="size:20;"> Equipe 2: </span><span id="capteur_h";style="color:green;size:20;"></span>
      <br>
      <span style="size:20;"> Equipe 3: </span><span id="capteur_h";style="color:green;size:20;"></span>
      <br>
      <span style="size:20;"> Equipe 4: </span><span id="capteur_h";style="color:green;size:20;"></span>
      <br>
    <br>
    <br>
    
	<h2 style = "text-align:center;font-size:25px"> DATA DISPLAY </h2>
  <div id="chart-temperature" class="container"></div>
  <div id="chart-humidity" class="container"></div>
    <script>
var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { text: 'lorem ipsum' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#059e8a' }
  },
  xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: 'points' }
    //title: { text: 'Temperature (Fahrenheit)' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartT.series[0].data.length > 40) {
        chartT.series[0].addPoint([x, y], true, true, true);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 1000 ) ;
    </script>
    <br>
    <br>
        <h2 id="ill"class="w3-center">Illustrations</h2>
    <div class="w3-content w3-display-container">
 	  <img class="mySlides" src="https://urlz.fr/jKSr"⁩ style="width:50%;height:60%">
	  <img class="mySlides" src="https://urlz.fr/jKSu"⁩ style="width:20%;height:25%">
	</div>
    <script>
var myIndex = 0;
carousel();

function carousel() {
  var i;
  var x = document.getElementsByClassName("mySlides");
  for (i = 0; i < x.length; i++) {
    x[i].style.display = "none";  
  }
  myIndex++;
  if (myIndex > x.length) {myIndex = 1}    
  x[myIndex-1].style.display = "block";  
  setTimeout(carousel, 2000); // Change image every 2 seconds
}
</script>
    </div>
    <footer class="w3-container w3-blue-grey w3-margin-top">
        <h5>Docker hosted webserver </h5>
        <p>launched from NodeRed</p>
    </footer>
  </body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

   //----------------------------------------------------SPIFFS

  if(!LittleFS.begin())
  {
    Serial.println("Erreur LittleFS...");
    return;
  }

  File root = LittleFS.open("/","r");
  File file = root.openNextFile();

  while(file.available())
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file.openNextFile();
  }

  Serial.println("Successfully Ended");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
      // Route for root / web page
    //request->send(LittleFS, "/index.html","text/html");
    //request->send(200, "text/plain","pepito");
    request->send(200, "text/html",html_page);
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}