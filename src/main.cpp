#include <Arduino.h>

#include <SPI.h>
#include <RF24.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LittleFS.h>

#include <ESPAsyncWiFiManager.h>   // Permet de configurer le Wifi via un portail d'accès  https://github.com/tzapu/WiFiManager
#include <NTPClient.h>             // Client NTP pour récupérer l'heure d'internet


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//----------------------------------------------------------------------------------------------------------------radio
#define pinCE   4           // On associe la broche "CE" du NRF24L01 à la sortie digitale D7 de l'arduino
#define pinCSN  5           // On associe la broche "CSN" du NRF24L01 à la sortie digitale D8 de l'arduino
RF24 radio(pinCE, pinCSN);    // Instanciation du NRF24L01
#define tunnel1 "PIPE1"
#define tunnel2 "PIPE2" 
const byte adresses[][6] = {tunnel1, tunnel2};    // Tableau des adresses de tunnel, au format "byte array"

#define SSID_AP        "Monportail_conf"    // Nom du réseau wifi sur le quel se connecter pour configurer la connexion Wifi à un routeur via WifiManager
#define SSID_MDP_AP    "maserre1234"       // Mot de passe du réseau wifi de configuration. Peut être laissé vide

struct __attribute__((__packed__)) setting {//valeur a envoyer a l'autre UC
  float temp_heat;//seuil declanchement chauffe
  short precision_heat;//precision de la chauffe
  short soil_moisture_trigger;//seuil declanchement electrovanne
  short precision_soil_moisture;//precision arrosage
  unsigned long interval;
  };
setting set = {22, 2, 500, 100, 5000};

struct donnees { //valeur a afficher
  float temp;
  float humidity;
  short soil_moisture;
};
donnees data;

unsigned long previousMillis = 0;


AsyncWebServer server(80);
DNSServer dns;

String processor(const String& _token)
{
  if(_token == "DEF_TEMP_HEAT")   return String(set.temp_heat); 
  if(_token == "DEF_PRECISION_HEAT")   return String(set.precision_heat); 
  if(_token == "DEF_SOIL_MOISTURE_TRIGGER")    return String(set.soil_moisture_trigger); 
  if(_token == "DEF_PRECISION_SOIL_MOISTURE")  return String(set.precision_soil_moisture); 
  if(_token == "DEF_INTERVAL")  return String(set.interval); 

  return String();
}

void setup() {
  Serial.begin(115200);

  timeClient.begin();

  delay(3000);

  //---------------------------------------------------------------------------------------------------------------------radio
  radio.begin();
  radio.openWritingPipe(adresses[0]);      // Ouverture du "tunnel1" en ÉCRITURE
  radio.openReadingPipe(1, adresses[1]);   // Ouverture du "tunnel2" en LECTURE
  radio.setPALevel(RF24_PA_MIN);
  Serial.print("Radio connected: ");
  Serial.println(radio.isChipConnected() ? "YES" : "NO !!!");


  
//---------------------------------------------------------------------------------------------------------------------------wifi 
  WiFi.hostname("Portail");
  AsyncWiFiManager wm(&server,&dns);
  wm.setConfigPortalTimeout(300);
  bool res = wm.autoConnect(SSID_AP, SSID_MDP_AP); // SSID et Mot de passe du point d'accès (portail) de configuration WiFi



if(!res) 
  {
  Serial.println("\n\nImpossible de se connecter au réseau local !");
  // ESP.restart(); // ==> Décommentez cette ligne pour rebooter le microcontrôlleur
  } 
else 
  {
  Serial.println("\n\nConnecté au réseau local !");
  }  


  //---------------------- Teste et monte le système de fichiers LittleFS -------------------------
Serial.print("\n- Liste les fichiers présents dans la mémoire flash LittleFS... ");

LittleFS.begin();      // Démarage du gestionnaire de fichier du microcontrôlleur

// LittleFS.format();  // ==> Décommentez cette ligne si il y a besoin de formater le système de fichier LittleFS du microcontrolleur

// Liste des fichiers (et affiche leur taille) présents dans la mémoire flash du microcontrolleur
String str = "\n";
Dir dir = LittleFS.openDir("/");
while (dir.next())  
  {
  str += dir.fileName();
  str += "\t";
  str += dir.fileSize();
  str += " octets\n";
  }
Serial.print(str);


server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  Serial.println("Appelle page index.html");
  request->send(LittleFS, "/index.html", String(), false);  // Envoie index.html
  });

server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  Serial.println("Appelle page config.html");
  request->send(LittleFS, "/config.html", String(), false, processor);  // Envoie config.html
  });



server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(LittleFS, "/w3.css", "text/css");                      // Demande au serveur d'envoyer w3.css
  });



server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(LittleFS, "/script.js", "text/javascript");            // Demande au serveur d'envoyer script.js
  });

server.on("/jquery-3.6.3.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(LittleFS, "/jquery-3.6.3.min.js", "text/javascript");            // Demande au serveur d'envoyer script.js
  });



server.on("/readtempIN", HTTP_GET, [](AsyncWebServerRequest *request) {
  String val = String(data.temp);
  request->send(200, "text/plain", val);
});

server.on("/readhumIN", HTTP_GET, [](AsyncWebServerRequest *request) {
  String val = String(data.humidity);
  request->send(200, "text/plain", val);
});

server.on("/readsoilmoisture", HTTP_GET, [](AsyncWebServerRequest *request) {
  String val = String(data.soil_moisture);
  request->send(200, "text/plain", val);
});




server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
  String message;
  if(request->hasParam("valeurInterval", true))
  {
    message = request->getParam("valeurInterval", true)->value();
    set.interval = message.toInt();
  }

  if(request->hasParam("valeurTempHeat", true))
  {
    message = request->getParam("valeurTempHeat", true)->value();
    set.temp_heat = message.toFloat();
  }

  if(request->hasParam("valeurPrecisionHeat", true))
  {
    message = request->getParam("valeurPrecisionHeat", true)->value();
    set.precision_heat = message.toInt();
  }
  
  if(request->hasParam("valeurMoistureTrigger", true))
  {
    message = request->getParam("valeurMoistureTrigger", true)->value();
    set.soil_moisture_trigger = message.toInt();
  }
  
  if(request->hasParam("valeurPrecisionSoilMoisture", true))
  {
    message = request->getParam("valeurPrecisionSoilMoisture", true)->value();
    set.precision_soil_moisture = message.toInt();
  }
  request->send(204);
});




server.begin();

}
void loop() {
  
  unsigned long currentMillis = millis();
  
  radio.startListening();                                                 // On commence par arrêter le mode envoi, pour pouvoir réceptionner des données
  if(radio.available()) {                                                 // On regarde si une donnée a été reçue
    while (radio.available()) {                                           // Si une donné est en attente de lecture, on va la lire
      radio.read(&data, sizeof(data));  // Lecture des données reçues, une par une
    }
     delay(5);
  } 
  
  if (currentMillis - previousMillis >= set.interval) {
    
    previousMillis = currentMillis;
    
    Serial.println(data.temp);
    Serial.println(data.humidity);
    Serial.println(data.soil_moisture);
  
    radio.stopListening();                                                  // On commence par arrêter le mode écoute, pour pouvoir émettre les données
    radio.write(&set, sizeof(set)); // … et on envoi cette valeur à l'autre arduino, via le NRF24
    delay(5);      
 }
}  