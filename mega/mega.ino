/*  FGSP - Testbaan - mega.ino
    Code: Jorik Wittevrongel, Neal Joos
    Deze code valt onder de MIT licentie (https://github.com/nealjoos/fgsp-arduino/blob/main/LICENSE.md)
! BOVENSTAANDE INFO MAG NIET GEWIJZIGD OF VERWIJDERD WORDEN !
*/

/*
    De begin en einde bestemming wordt voorlopig ingegeven via de seriele monitior

    INSTELLINGEN SERIËLE COMMUNICATIE: 115200 baud
      Per doorvoer vanaf NodeMcu (of pc) staat alles op één regel. Bij het einde van die regel begint het aanpassen van de waarden opnieuw.

      Voorlopig is enkel het rijden van / naar SpoorOnder en SpoorBoven mogelijk

      We gaan er van uit dat alle sporen in wijzersin gepolariseerd zijn. Dus van SpoorOnder naar Rechts naar Spoor 2 naar Links en terug naar SpoorOnder

      De gevoeligheid van de reedcontacten kun je instellen onder de variable reedContactTrigger > (Extra variablen, juist boven setup())
      Voorlopig staat het op de heft van het analoge gebied (512).

      TODO: 
        - !! Pinouts controleren !!
        - Integratie wissels (zijsporen)
        - Rijden in tegenwijzerzin
        Optioneel: 
            - Resetfunctie
            - Servo's (incl. sleutelschakelaar en LED strips)
            - Bezette sporen status (bool)
*/

//Bibliotheken
#include <Arduino.h> //Dit moet je toevoegen bij platformio in de arduino ide is dit niet nodig maar kan geen kwaad

// INGANGEN
// Reedcontacten
#define inputReedSpoorOnderLinks A0
#define inputReedSpoorOnderRechts A1
#define inputReedZijSpoorOnderLinks A2
#define inputReedZijSpoorOnderRechts A3
#define inputReedSpoorBovenLinks A4
#define inputReedSpoorBovenRechts A5
#define inputReedZijSpoorBovenLinks A6
#define inputReedZijSpoorBovenRechts A7
#define inputReedLinksSpoorOnder A8
#define inputReedLinksSpoorBoven A9
#define inputReedRechtsSpoorOnder A10
#define inputReedRechtsSpoorBoven A11

// Sleutelschakelaar
#define inputSleutelLinks A14
#define inputSleutelRechts A15

// UITGANGEN
// Snelheid
#define outputLinks 2
#define outputRechts 3
#define outputSpoorOnder 4
#define outputSpoorBoven 5
#define outputZijSpoorOnder 6
#define outputZijSpoorBoven 7

// Wissels
#define outputwisselSpoorOnder_Links 32
#define outputwisselSpoorOnder_Rechts 33
#define outputwisselSpoorBoven_Links 34
#define outputwisselSpoorOnder_Rechts 35
#define outputWisselActivator 36

// Ompolers
#define outputOmpolerSpoorOnder 22
#define outputOmpolerZijSpoorOnder 23
#define outputOmpolerSpoorBoven 24
#define outputOmpolerZijSpoorBoven 25
#define outputOmpolerLinks 26
#define outputOmpolerRechts 27

// VARIABELEN
// - INGANGEN
//    Reedcontacten
boolean reedSpoorOnderLinks;
boolean reedSpoorOnderRechts;
boolean reedZijSpoorOnderLinks;
boolean reedZijSpoorOnderRechts;
boolean reedSpoorBovenLinks;
boolean reedSpoorBovenRechts;
boolean reedZijSpoorBovenLinks;
boolean reedZijSpoorBovenRechts;
boolean reedLinksSpoorOnder;
boolean reedLinksSpoorBoven;
boolean reedRechtsSpoorOnder;
boolean reedRechtsSpoorBoven;

//    Sleutelschakelaar
boolean SleutelLinks;
boolean SleutelRechts;

// - UITGANGEN
//    Snelheid (Analoog 0-255)
float multiplier = 2.55; // vermenigvuldigingsfactor van % naar 0-255
int snelheidSpoorOnder = 0;
int snelheidSpoorBoven = 0;
int snelheidZijSpoorOnder = 0;
int snelheidZijSpoorBoven = 0;
int snelheidLinks = 0;
int snelheidRechts = 0;

//    Wissels
//    In rust worden geen zijsporen gebruikt
boolean wisselSpoorOnder_Links = 0;
boolean wisselSpoorOnder_Rechts = 0;
boolean wisselSpoorBoven_Links = 0;
boolean wisselSpoorBoven_Rechts = 0;

//    Ompolers
//    In rust rijdt de trein in wijzerzin
boolean ompolerRechts = 0;
boolean ompolerLinks = 0;
boolean ompolerZijSpoorBoven = 0;
boolean ompolerSpoorBoven = 0;
boolean ompolerZijSpoorOnder = 0;
boolean ompolerSpoorOnder = 0;

// - DIVERS
const int reedContactTrigger = 512; //Bij bij welke analoge waarde moeten de reedcontacten triggeren
int huidigePlaats = 0;              //Houdt de plaats van de trein bij, als hij rijdt
boolean bestemming = 1;             //Houdt bij als de trein op zijn bestemming is of niet

// Tekststrings
String start;
String stop;
String richting;
String noodstop;

// Void loops
// dit moet je toevoegen bij platformio, in de arduino ide is dit niet nodig maar kan geen kwaad
void rijdenWijzersin(String start, String stop);
void leesReedContacten();
void leesSleutelSchakelaar();
void reset();
void noodstopFunctie();

void setup() {
  // Start seriële communicatie
  Serial1.begin(115200); // Als je werkt met de NodeMCU
  Serial.begin(115200); // Manuele invoer (met pc)

  // Definieer ingangen
  // Reedcontacten
  pinMode(inputReedSpoorOnderLinks, INPUT);
  pinMode(inputReedSpoorOnderRechts, INPUT);
  pinMode(inputReedZijSpoorOnderLinks, INPUT);
  pinMode(inputReedZijSpoorOnderRechts, INPUT);
  pinMode(inputReedSpoorBovenLinks, INPUT);
  pinMode(inputReedSpoorBovenRechts, INPUT);
  pinMode(inputReedZijSpoorBovenLinks, INPUT);
  pinMode(inputReedZijSpoorBovenRechts, INPUT);
  pinMode(inputReedLinksSpoorOnder, INPUT);
  pinMode(inputReedLinksSpoorBoven, INPUT);
  pinMode(inputReedRechtsSpoorOnder, INPUT);
  pinMode(inputReedRechtsSpoorBoven, INPUT);

  // Definieer uitgangen
  // Snelheid
  pinMode(outputLinks, OUTPUT);
  pinMode(outputRechts, OUTPUT);
  pinMode(outputSpoorOnder, OUTPUT);
  pinMode(outputSpoorBoven, OUTPUT);
  // Wissels
  pinMode(outputwisselSpoorOnder_Links, OUTPUT);
  pinMode(outputwisselSpoorOnder_Rechts, OUTPUT);
  pinMode(outputwisselSpoorOnder_Links, OUTPUT);
  pinMode(outputwisselSpoorOnder_Rechts, OUTPUT);
  pinMode(outputWisselActivator, OUTPUT);
}

void loop() {
  while (Serial.available()) {                                                                        //zolang er data ontvangen wordt vanaf de NodeMcu                                                                         //zolang de bestemming bereikt is (dus trein staat stil)
    String serialInput = Serial.readStringUntil('\n');                                              //Leest de input tot een enter
    parseSerial(serialInput);
    if (noodstop.equals("1")) { //kijkt of de noodstop is gebruikt
      noodstopFunctie(); //gaat naar de noodstopFunctie() loop
    }
    else {
      if (richting.equals("CW")) { //als de richting gelijk is aan CW (clockwise)
        bestemming = 0; //de bestemming is niet bereikt
        rijdenWijzersin(start, stop);//begin met rijden in wijzersin met start en stop locatie
      }
      //else if (richting.equals("CCW")) { //als de richting gelijk is aan CCW (counterclockwise)
      //  bestemming = 0; //de bestemming is niet bereikt
      //  rijdenTegenWijzersin(start, stop); //begin met rijden in tegenwijzersin met start en stop locatie
      //}
    }
  }
}

void rijdenWijzersin(String start, String stop) {                               //TODO: De zijsporen moeten nog toegevoegd worden !! + servos
  while (!bestemming) {                                                         //Zo lang je de bestemming niet bereikt hebt blijf dit door lopen
    leesReedContacten();                                                        //Lees alle reedcontacten uit
    /*
      Deze if bestaat uit 3 delen
      Eerst kijken als Spoor 1 het start punt is
      Of het kan zijn dat de trein moet aankomen in deze sectie (trein rijd al)
      Voer pas uit als de trein voorbij het reedcontact rijdt
    */
    if ((start.equals("SpoorOnder") || huidigePlaats == 1) && (reedContactTrigger < reedRechtsSpoorOnder)) {
      if (stop.equals("SpoorOnder")) {                                              //Moet de trein hier stoppen?
        analogWrite(outputLinks, 0);                                            //Zet active sporen uit
        analogWrite(outputSpoorOnder, 0);
        bestemming = 1;                                                         //Zorg dat je uit de while loop kunt
        huidigePlaats = 0;                                                      //de trein staat stil dus heeft geen (rijdende) huidige plaats reset deze voor volgende opdracht
      }
      else {                                                                    //Als de trein moet door rijden
        analogWrite(outputLinks, 0);                                            //Zet de vorige sectie zonder spanning zetten
        analogWrite(outputSpoorOnder, snelheidSpoorOnder);                              //Als de trein vertrek moeten we op de huidige sectie spanning zetten als de trein al rijd heeft deze sectie al spanning en doet deze lijn niks
        analogWrite(outputRechts, snelheidRechts);                               //Zet op de volgende sectie spanning
        huidigePlaats = 2;                                                      //Gaan naar de volgende stap
      }
    }

    if (huidigePlaats == 2 && reedContactTrigger < reedRechtsSpoorBoven) {          //if als de trein op het rechter spoor rijdt en het reedcontact voorbij rijdt
      analogWrite(outputSpoorOnder, 0);                                             //Zet de vorige sectie zonder spanning zetten
      analogWrite(outputSpoorBoven, snelheidSpoorBoven);                                //Zet op de volgende sectie spanning
      huidigePlaats = 3;                                                        //Ga naar de volgende stap
    }
    /*
      Deze if bestaat uit 3 delen
      Eerst kijken als Spoor 2 het start punt is
      Of het kan zijn dat de trein moet aankomen in deze sectie (trein rijd al
      Voer pas uit als de trein voorbij het reedcontact rijdt
    */
    if ((start.equals("SpoorBoven") || huidigePlaats == 3) && (reedContactTrigger < reedSpoorBovenLinks)) {
      if (stop.equals("SpoorBoven")) {                                              //Moet de trein hier stoppen?
        analogWrite(outputRechts, 0);                                            //Zet active sporen uit
        analogWrite(outputSpoorBoven, 0);
        bestemming = 1;                                                         //Zorg dat je uit de while loop kunt
        huidigePlaats = 0;                                                      //De trein staat stil dus heeft geen (rijdende) huidige plaats reset deze voor volgende opdracht
      }
      else {
        analogWrite(outputRechts, 0);                                            //Zet de vorige sectie zonder spanning zetten
        analogWrite(outputSpoorBoven, snelheidSpoorBoven);                              //Als de trein vertrek moeten we op de huidige sectie spanning zetten als de trein al rijd heeft deze sectie al spanning en doet deze lijn niks
        analogWrite(outputLinks, snelheidLinks);                                //Zet op de volgende sectie spanning
        huidigePlaats = 4;                                                      //Ga naar de volgende stap
      }
    }

    if (huidigePlaats == 4 && (reedContactTrigger < reedLinksSpoorOnder)) {         //if als de trein op het linker spoor rijdt en het reedcontact voorbij rijdt
      analogWrite(outputSpoorBoven, 0);                                             //Zet de vorige sectie zonder spanning zetten
      analogWrite(outputSpoorOnder, snelheidSpoorOnder);                                //Zet op de volgende sectie spanning
      huidigePlaats = 1;                                                        //Ga naar de volgende stap (start loop opnieuw)
    }
  }
}

void leesReedContacten() {                                                      //Lees alle reedcontacten en zet ze in een variable
  reedSpoorOnderLinks = analogRead(inputReedSpoorOnderLinks);
  reedSpoorOnderRechts = analogRead(inputReedSpoorOnderRechts);
  reedZijSpoorOnderLinks = analogRead(inputReedZijSpoorOnderLinks);
  reedZijSpoorOnderRechts = analogRead(inputReedZijSpoorOnderRechts);
  reedSpoorBovenLinks = analogRead(inputReedSpoorBovenLinks);
  reedSpoorBovenRechts = analogRead(inputReedSpoorBovenRechts);
  reedZijSpoorBovenLinks = analogRead(inputReedZijSpoorBovenLinks);
  reedZijSpoorBovenRechts = analogRead(inputReedZijSpoorBovenRechts);
  reedLinksSpoorOnder = analogRead(inputReedLinksSpoorOnder);
  reedLinksSpoorBoven = analogRead(inputReedLinksSpoorBoven);
  reedRechtsSpoorOnder = analogRead(inputReedRechtsSpoorOnder);
  reedRechtsSpoorBoven = analogRead(inputReedRechtsSpoorBoven);
}

void leesSleutelSchakelaar() {                                                      //Leest de sleutelschakelaars en zet ze in een variable
  SleutelLinks = analogRead(inputSleutelLinks);
  SleutelRechts = analogRead(inputSleutelRechts);
}

void reset() { //Zet alles in rust
  //TODO
}

void noodstopFunctie() { //Noodstop
  analogWrite(outputLinks, 0);
  analogWrite(outputRechts, 0);
  analogWrite(outputSpoorOnder, 0);
  analogWrite(outputZijSpoorOnder, 0);
  analogWrite(outputSpoorBoven, 0);
  analogWrite(outputZijSpoorBoven, 0);
  for (int i = 0; i <= 10; i++) {//10 keer de led strip laten flikkeren + buzzer (TODO: juiste pin bepalen)
    //analogWrite(ledstrip met buzzer pin, 255);
    delay(250);
    //analogWrite(ledstrip met buzzer pin, 0);
    delay(250);
  }
}

void parseSerial(String serialInput) {
  int startIndex_start = serialInput.indexOf("Start:") + 6;                                       //Zoek de start van de plaats waar de trein moet starten
  int Index_end = serialInput.indexOf(" ", startIndex_start);                                     //Zoek het einde van de waarde
  start = serialInput.substring(startIndex_start, Index_end);                                     //Maak de string start met begin en eind indexen

  int stopIndex_start = serialInput.indexOf("Stop:") + 6;                                         //Zoek de start van de plaats waar de trein moet stoppen
  Index_end = serialInput.indexOf(" ", stopIndex_start);                                          //Zoek het einde van de waarde
  stop = serialInput.substring(stopIndex_start, Index_end);                                       //Maak de string stop met begin en eind indexen

  int richtingIndex_start = serialInput.indexOf("Richting:") + 9;                                 //Zoek de start van de rijrichting
  Index_end = serialInput.indexOf(" ", richtingIndex_start);                                      //Zoek het einde van de waarde
  richting = serialInput.substring(richtingIndex_start, Index_end);                               //Maak de string richting met begin en eind indexen

  int snelheidSpoorOnderIndex_start = serialInput.indexOf("SS1:") + 4;                                //Zoek de start van snelheid sectie 1
  Index_end = serialInput.indexOf(" ", snelheidSpoorOnderIndex_start);                                //Zoek het einde van de waarde
  String snelheidSpoorOnder = serialInput.substring(snelheidSpoorOnderIndex_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidSpoorOnder = multiplier * snelheidSpoorOnder.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoorBovenIndex_start = serialInput.indexOf("SS2:") + 4;                                //Zoek de start van snelheid sectie 2
  Index_end = serialInput.indexOf(" ", snelheidSpoorBovenIndex_start);                                //Zoek het einde van de waarde
  String snelheidSpoorBoven = serialInput.substring(snelheidSpoorBovenIndex_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidZijSpoorOnder = multiplier * snelheidSpoorBoven.toInt();                                        //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor3Index_start = serialInput.indexOf("SS3:") + 4;                                //Zoek de start van snelheid sectie 3
  Index_end = serialInput.indexOf(" ", snelheidSpoor3Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor3 = serialInput.substring(snelheidSpoor3Index_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidSpoorBoven = multiplier * snelheidSpoor3.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor4Index_start = serialInput.indexOf("SS4:") + 4;                                //Zoek de start van snelheid sectie 4
  Index_end = serialInput.indexOf(" ", snelheidSpoor4Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor4 = serialInput.substring(snelheidSpoor4Index_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidZijSpoorBoven = multiplier * snelheidSpoor4.toInt();                                        //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor5Index_start = serialInput.indexOf("SS5:") + 4;                                //Zoek de start van snelheid sectie 5
  Index_end = serialInput.indexOf(" ", snelheidSpoor5Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor5 = serialInput.substring(snelheidSpoor5Index_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidLinks = multiplier * snelheidSpoor5.toInt();                                            //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor6Index_start = serialInput.indexOf("SS6:") + 4;                                //Zoek de start van snelheid sectie 6
  Index_end = serialInput.indexOf(" ", snelheidSpoor6Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor6 = serialInput.substring(snelheidSpoor6Index_start, Index_end);            //Maak de string snelheidSpoorOnder met begin en eind indexen
  snelheidRechts = multiplier * snelheidSpoor6.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int noodstopIndex_start = serialInput.indexOf("ES:") + 3;                                       //Zoek de start van noodstop
  noodstop = serialInput.substring(noodstopIndex_start);                                          //Maak de string noodstop met begin en eind indexen (einde is het einde van de seriële 'blok' zie readUntil)
}
