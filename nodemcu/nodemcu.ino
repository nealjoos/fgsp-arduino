/*  FGSP Testbaan
 *  Code: Neal Joos
    NodeMcu code voor het aansturen van de testbaan
    Dit programma zorgt ervoor dat de webserver wordt opgestart, de webbestanden van de micro SD kaart haalt en de communicatie verzorgt tussen de webclient en de Arduino Mega

    Opmerkingen over de code
    - namen van bestanden op de SD kaart mogen max. 8 tekens bevatten!
    - bestandextensies mogen niet langer dan 3 tekens zijn! (dus we gebruiken .htm ipv .html)
*/

#include <ESP8266WiFi.h> //de benodigde bibliotheken voor de wifi uit te zenden
#include <ESP8266WebServer.h> //om de webserver te laten draaien
#include <WiFiClient.h> //laat http get werken
#include <ESP8266mDNS.h> //zorgt ervoor dat je kunt verbinden via een http://host.local ipv het ip adres (standaard 192.168.4.1 trouwens)
#include <SPI.h> //zorgt voor communicatie met SD kaart
#include <SD.h> //de sd kaart bibliotheek

const char *ssid = "FGSP"; //SSID (netwerknaam)
const char *password = "wachtwoord"; //Wachtwoord van het netwerk
const char *host = "fgsp"; //MDNS host, zodat je kan verbinden naar host.local ipv het ip adres

ESP8266WebServer server(80); //start de webserver op poort 80

static bool hasSD = false;
bool noodstop = false;

void returnOK() {
  server.sendHeader("Location", "/");
  server.send(301);
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n"); //hierbij zend de server een error code (internal error, zie ook bovenstaande wiki link)
}

bool loadFromSdCard(String path) { //laad de bestanden van de sd kaart
  String dataType = "text/plain";
  if (noodstop) { //als noodstop is geactiveerd
    handleLocked();
  } else {
    if (path.endsWith("/")) { //als het ingetypte webadres eindigt met / dan zet hij er nog index.htm achter, zodat hij standaard naar de index pagina gaat als je op de server komt
      path += "index.htm";
    }

    if (path.endsWith(".src")) {
      path = path.substring(0, path.lastIndexOf("."));
    } else if (path.endsWith(".htm")) { //als de extensie .htm is is het een html bestand
      dataType = "text/html";
    } else if (path.endsWith(".css")) { //als de extensie .css is is het een webopmaak bestand
      dataType = "text/css";
    } else if (path.endsWith(".js")) { //als de extensie .js is is het een javascript bestand
      dataType = "application/javascript";
    } else if (path.endsWith(".png")) { //als de extensie .png is is het een afbeelding
      dataType = "image/png";
    } else if (path.endsWith(".gif")) { //als de extensie .gif is is het een afbeelding
      dataType = "image/gif";
    } else if (path.endsWith(".jpg")) { //als de extensie .jpg is is het een afbeelding
      dataType = "image/jpeg";
    } else if (path.endsWith(".ico")) { //als de extensie .ico is is het een icoontje
      dataType = "image/x-icon";
    } else if (path.endsWith(".xml")) { //als de extensie .xml is is het een xml bestand (zie https://nl.wikipedia.org/wiki/Extensible_Markup_Language)
      dataType = "text/xml";
    } else if (path.endsWith(".pdf")) { //als de extensie .pdf is is het een pdf bestand
      dataType = "application/pdf";
    } else if (path.endsWith(".zip")) { //als de extensie .zip is is het een gecomprimeerde map
      dataType = "application/zip";
    }

    File dataFile = SD.open(path.c_str()); //opent de sd kaart op de hoofdmap
    if (dataFile.isDirectory()) {
      path += "/index.htm";
      dataType = "text/html";
      dataFile = SD.open(path.c_str());
    }

    if (!dataFile) {
      return false;
    }

    if (server.hasArg("download")) {
      dataType = "application/octet-stream"; //laat de client weten wanneer er een download is
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size()) {
      Serial.println("ERROR: Server send less data"); //wanneer er iets misgaat tijdens het downloaden
    }

    dataFile.close();
    return true;
  }
}

void handleNotFound() { //handleNotFound is de bekende 404 pagina (zie ook terug https://nl.wikipedia.org/wiki/Lijst_van_HTTP-statuscodes)
  if (noodstop) { //als noodstop is geactiveerd
    handleLocked();
  } else {
    if (hasSD && loadFromSdCard(server.uri())) {
      return;
    }
    String content = "<!DOCTYPE html><html lang=\"nl-BE\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <meta name=\"author\" content=\"Leerlingen FGSP\"> <title>Oops...</title> <style>/* Styles of the 404 page of my website. */ body{background: #081421; color: #d3d7de; font-family: \"Courier new\"; font-size: 18px; line-height: 1.5em; cursor: default;}a{color: #fff;}.code-area{position: absolute;   width: 320px; min-width: 320px; top: 50%; left: 50%; -webkit-transform: translate(-50%, -50%); transform: translate(-50%, -50%);}.code-area > span{display: block;}@media screen and (max-width: 320px){.code-area{font-size: 5vw; min-width: auto; width: 95%; margin: auto; padding: 5px; padding-left: 10px; line-height: 6.5vw;}}</style></head><body><div class=\"code-area\"> <span style=\"color: #777;font-style:italic;\"> // 404 page not found. </span> <span> <span style=\"color:#d65562;\"> if </span> (<span style=\"color:#4ca8ef;\">!</span><span style=\"font-style: italic;color:#bdbdbd;\">found</span>){</span> <span> <span style=\"padding-left: 15px;color:#2796ec\"> <i style=\"width: 10px;display:inline-block\"></i>throw </span> <span> (<span style=\"color: #a6a61f\">\"(╯°□°)╯︵ ┻━┻\"</span>); </span> <span style=\"display:block\">}</span> <span style=\"color: #777;font-style:italic;\"> // <a href=\"/\">Go home!</a> </span> </span></div></body></html>";
    server.send(404, "text/html", content);
  }
}

void handleLocked() {
  if (noodstop) { //als noodstop is geactiveerd
    server.send(200, "text/html", "<!DOCTYPE html><html lang='nl-BE'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Vergrendeld</title><style>*{margin:0;padding:0;box-sizing:border-box;--locked-color:#5fadbf;--unlocked-color:#ff5153;font-family:Arial,Helvetica,sans-serif;text-align:center}.container{display:flex;align-items:center;justify-content:center;min-height:400px}.lock{width:24px;height:21px;border:3px solid var(--locked-color);border-radius:5px;position:relative;cursor:pointer;-webkit-transition:all 0.1s ease-in-out;transition:all 0.1s ease-in-out}.lock:after{content:'';display:block;background:var(--locked-color);width:3px;height:7px;position:absolute;top:50%;left:50%;margin:-3.5px 0 0 -2px;-webkit-transition:all 0.1s ease-in-out;transition:all 0.1s ease-in-out}.lock:before{content:'';display:block;width:10px;height:10px;bottom:100%;position:absolute;left:50%;margin-left:-8px;border:3px solid var(--locked-color);border-top-right-radius:50%;border-top-left-radius:50%;border-bottom:0;-webkit-transition:all 0.1s ease-in-out;transition:all 0.1s ease-in-out}</style></head><body><div class='container'> <span class='lock'></span></div><p>Hardware reset vereist om te ontgrendelen</p></body></html>");
  }
  else if (!noodstop) { //als noodstop niet is geactiveerd
    returnOK();
  }
}

void handleSubmit() {
  if (server.hasArg("emergencystop")) { //als de noodstop is ingedrukt
    Serial.println("");
    Serial.println("ES:1");
    noodstop = true;
    handleLocked();
  }
  else if (server.hasArg("start") && server.hasArg("stop") && server.hasArg("richting")) { //als start, stop en richting zijn ingegeven
    Serial.print("Start:");
    Serial.print(server.arg("start"));
    Serial.print(" ");
    Serial.print("Stop:");
    Serial.print(server.arg("stop"));
    Serial.print(" ");
    Serial.print("Richting:");
    Serial.print(server.arg("richting"));
    Serial.print(" ");
    Serial.print("SS1:");
    Serial.print(server.arg("SS1"));
    Serial.print(" ");
    Serial.print("SS2:");
    Serial.print(server.arg("SS2"));
    Serial.print(" ");
    Serial.print("SS3:");
    Serial.print(server.arg("SS3"));
    Serial.print(" ");
    Serial.print("SS4:");
    Serial.print(server.arg("SS4"));
    Serial.print(" ");
    Serial.print("SS5:");
    Serial.print(server.arg("SS5"));
    Serial.print(" ");
    Serial.print("SS6:");
    Serial.println(server.arg("SS6"));
    returnOK();
  }
  else { //in elk ander geval, deze worden altijd meegegeven
    Serial.print("SS1:");
    Serial.print(server.arg("SS1"));
    Serial.print(" ");
    Serial.print("SS2:");
    Serial.print(server.arg("SS2"));
    Serial.print(" ");
    Serial.print("SS3:");
    Serial.print(server.arg("SS3"));
    Serial.print(" ");
    Serial.print("SS4:");
    Serial.print(server.arg("SS4"));
    Serial.print(" ");
    Serial.print("SS5:");
    Serial.print(server.arg("SS5"));
    Serial.print(" ");
    Serial.print("SS6:");
    Serial.println(server.arg("SS6"));
    returnOK();
  }
}

void setup() { //en de setup, deze zet je in codes met meerdere loops best onderaan, alle loops moeten eerst gedefinieerd zijn voordat iets werkt
  Serial.begin(115200); //opent de debug poort met een bepaalde snelheid
  WiFi.mode(WIFI_AP_STA); //belangrijk: dit laat de Nodemcu weten hoe hij moet werken: als Access Point (transmitter, WIFI_AP), Client (de bezoeker/station, WIFI_STA) of een combinatie (WIFI_AP_STA)
  WiFi.softAP(ssid); //maakt een Access point met de ssid, hier is de password char verwijderd zodat er geen beveiligin is op het netwerk
  IPAddress ip(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);

  MDNS.begin(host);//start de MDNS service (meer info: https://nl.wikipedia.org/wiki/Domain_Name_System)

  server.on("/submit", handleSubmit);
  server.on("/locked", handleLocked);
  server.onNotFound(handleNotFound); //laat de server weten wat hij moet doen als er een url is ingetypt die niet bestaat op de server, in dit geval naar de NotFound pagina gaan
  server.begin(); //start de HTTP server

  if (SD.begin(SS)) { //verbind met de SD kaart via de SS (ChipSelect, op de nodemcu pin D8)
    hasSD = true;
  } else {
    hasSD = false;
    Serial.println("ERROR: SD verbonden?");
  }
}

void loop() {
  server.handleClient(); //de webclient laten draaien
  MDNS.update();
}

//hopelijk is alles nu wat duidelijker :)
