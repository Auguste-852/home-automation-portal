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

#define SSID_AP        "MyPortal_conf"    // Nom du réseau wifi sur le quel se connecter pour configurer la connexion Wifi à un routeur via WifiManager
#define SSID_MDP_AP    "Portal1234"       // Mot de passe du réseau wifi de configuration. Peut être laissé vide


AsyncWebServer server(80);
DNSServer dns;


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

}  