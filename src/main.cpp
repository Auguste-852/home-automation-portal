#define SERIAL_PRINTF Serial.printf

#include <Arduino.h> 

#include <SPI.h>
#include <RF24.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LittleFS.h> 

#include <ESPAsyncWiFiManager.h>   // Permet de configurer le Wifi via un portail d'accès  https://github.com/tzapu/WiFiManager


WiFiUDP ntpUDP;

unsigned long timeSinceLastEat();

//----------------------------------------------------------------------------------------------------------------radio
#define pinCE   4           // On associe la broche "CE" du NRF24L01 à la sortie digitale D7 de l'arduino
#define pinCSN  5           // On associe la broche "CSN" du NRF24L01 à la sortie digitale D8 de l'arduino
RF24 radio(pinCE, pinCSN);    // Instanciation du NRF24L01
#define tunnel1 "PIPE1"
#define tunnel2 "PIPE2" 
const byte adresses[][6] = {tunnel1, tunnel2};    // Tableau des adresses de tunnel, au format "byte array"


#define SSID_AP        "MyPortal_conf"    // Nom du réseau wifi sur le quel se connecter pour configurer la connexion Wifi à un routeur via WifiManager
#define SSID_MDP_AP    "Portal1234"       // Mot de passe du réseau wifi de configuration. Peut être laissé vide



AsyncWebServer server(80);
DNSServer dns;

bool doorOpen = true;
String doorStatus = "close";
unsigned long lastEatTime = 0;

struct Settings {
  uint8_t doorInterval;
  uint8_t feedInterval;
  uint8_t foodQuantity;
  uint16_t photoresistorThreshold;
  uint8_t timeToEat;
};
Settings settings;


void setup() {
  settings.doorInterval = 2;
  Serial.begin(115200);


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


server.on("/open", HTTP_GET, [](AsyncWebServerRequest *request) {
  const char command = 'O';  // 'O' pour Open (Ouvrir)
  radio.stopListening();
  radio.write(&command, sizeof(command)); // Envoi de la commande 'O' à l'autre ESP
  request->send(204); 
});

server.on("/close", HTTP_GET, [](AsyncWebServerRequest *request) {
  const char command = 'C';  // 'C' pour Close (Fermer)
  radio.stopListening();
  radio.write(&command, sizeof(command)); // Envoi de la commande 'C' à l'autre ESP
  request->send(204);
});

server.on("/ReadDoorState", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(200, "text/plain", doorStatus);          
  });

server.on("/ReadTimeSinceLastEat", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    unsigned long elapsedTime = timeSinceLastEat();
    String elapsedTimeStr = String(elapsedTime / 60000); // Convertir en minutes et en chaîne de caractères
    request->send(200, "text/plain", elapsedTimeStr);
    });

server.on("/ReadDoorInterval", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String doorIntervalStr = String(settings.doorInterval);
    request->send(200, "text/plain", doorIntervalStr);
    });
  
server.on("/updateSettings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("doorInterval", true) && 
      request->hasParam("feedInterval", true) && 
      request->hasParam("foodQuantity", true) &&
      request->hasParam("photoresistorThreshold", true) &&
      request->hasParam("timeToEat", true)) {
      
      Settings settings;
      settings.doorInterval = request->getParam("doorInterval", true)->value().toInt();
      settings.feedInterval = request->getParam("feedInterval", true)->value().toInt();
      settings.foodQuantity = request->getParam("foodQuantity", true)->value().toInt();
      settings.photoresistorThreshold = request->getParam("photoresistorThreshold", true)->value().toInt();
      settings.timeToEat = request->getParam("timeToEat", true)->value().toInt();


      radio.stopListening();
      radio.write(&settings, sizeof(settings)); // Envoi des paramètres via RF24

      request->send(204);
    } else {
      request->send(400);
    }
  });

server.begin();

}
void loop() {
  if (radio.available()) {
    char status[20];
    radio.read(&status, sizeof(status));
    
    if (strcmp(status, "OPEN") == 0) {
        doorOpen = true;
        doorStatus = "open";
    } else if (strcmp(status, "CLOSED") == 0) {
        doorOpen = false;
        doorStatus = "close";
    } else if (strcmp(status, "ERROR_OPEN") == 0) {
        doorOpen = false;
        doorStatus = "error_open";
    } else if (strcmp(status, "ERROR_CLOSE") == 0) {
        doorOpen = false;
        doorStatus = "error_close";
    } else if (strcmp(status, "EAT") == 0) {
      lastEatTime = millis();
    }
    // Gérer les erreurs si nécessaire
    Serial.print("Door status received: ");
    Serial.println(status);
  }
}  


unsigned long timeSinceLastEat() {
  // Retourne le temps écoulé en millisecondes depuis le dernier "EAT"
  return millis() - lastEatTime;
}