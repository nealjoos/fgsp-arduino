/* FGSP Testbaan
    Code: Jorik Wittevrongel, Neal Joos

    De begin en einde bestemming wordt voorlopig ingegeven via de seriele monitior

    INSTELLINGEN SERIËLE COMMUNICATIE: 115200 baud
      Per doorvoer vanaf NodeMcu (of pc) staat alles op één regel. Bij het einde van die regel begint het aanpassen van de waarden opnieuw.

      Voorlopig is enkel het rijden van / naar Spoor1 en Spoor2 mogelijk

      We gaan er van uit dat alle sporen in wijzersin gepolariseerd zijn. Dus van Spoor1 naar Rechts naar Spoor 2 naar Links en terug naar Spoor1

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

//Snelheid (Analoog 0-255)
float multiplier = 2.55; //de factor waarmee de waarde vanaf de NodeMcu vermeningvuldigd wordt (om een waarde van 0-255 te bekomen ipv 0-100)
int snelheidSpoor1 = 0;
int snelheidSpoor2 = 0;
int snelheidZijSpoor1 = 0;
int snelheidZijSpoor2 = 0;
int snelheidLinks = 0;
int snelheidRechts = 0;

//Uitgangen Snelheid
#define outputLinks 2
#define outputRechts 3
#define outputSpoor1 4
#define outputSpoor2 5
#define outputZijSpoor1 6
#define outputZijSpoor2 7

//Wissels
//De rust positie van de wissels laat de trein rijden op de buiten cirkel (geen zijsporen)
boolean wisselSporen1_Rechts = 0;
boolean wisselSporen1_Links = 0;
boolean wisselSporen2_Rechts = 0;
boolean wisselSporen2_Links = 0;

//Uitgangen Wissels
#define outputWisselSporen1_Rechts 32
#define outputWisselSporen1_Links 33
#define outputWisselSporen2_Rechts 34
#define outputWisselSporen2_Links 35
#define outputWisselActivator 36

//Ompolers
//De rust positie van de ompolers laat de trein rijden in wijzerzin
boolean ompolerRechts = 0;
boolean ompolerLinks = 0;
boolean ompolerZijSpoor2 = 0;
boolean ompolerSpoor2 = 0;
boolean ompolerZijSpoor1 = 0;
boolean ompolerSpoor1 = 0;

//Uitgangen Ompolers
#define outputOmpolerRechts 43
#define outputOmpolerLinks 44
#define outputOmpolerZijSpoor2 45
#define outputOmpolerSpoor2 46
#define outputOmpolerZijSpoor1 47
#define outputOmpolerSpoor1 48

//Reed sensoren
boolean reedSpoor1Links;
boolean reedSpoor1Rechts;
boolean reedZijSpoor1Links;
boolean reedZijSpoor1Rechts;
boolean reedSpoor2Links;
boolean reedSpoor2Rechts;
boolean reedZijSpoor2Links;
boolean reedZijSpoor2Rechts;
boolean reedLinksSpoor1;
boolean reedLinksSpoor2;
boolean reedRechtsSpoor1;
boolean reedRechtsSpoor2;

//Input Reed
#define inputReedSpoor1Links A0
#define inputReedSpoor1Rechts A1
#define inputReedZijSpoor1Links A2
#define inputReedZijSpoor1Rechts A3
#define inputReedSpoor2Links A4
#define inputReedSpoor2Rechts A5
#define inputReedZijSpoor2Links A6
#define inputReedZijSpoor2Rechts A7
#define inputReedLinksSpoor1 A8
#define inputReedLinksSpoor2 A9
#define inputReedRechtsSpoor1 A10
#define inputReedRechtsSpoor2 A11

//Reed Sleutelschakelaar
boolean SleutelLinks;
boolean SleutelRechts;

//Input Sleutelschakelaar
#define inputSleutelLinks A14
#define inputSleutelRechts A15

//voids dit moet je toevoegen bij platformio in de arduino ide is dit niet nodig maar kan geen kwaad
void rijdenWijzersin(String start, String stop);
void leesReedContacten();
void leesSleutelSchakelaar();
void reset();
void noodstopFunctie();

//Extra variablen
const int reedContactTrigger = 512;                                             //Bij bij welke analoge waarde moeten de reedcontacten triggeren
int huidigePlaats = 0;                                                          //Houdt de plaats van de trein bij, als hij rijdt
boolean bestemming = 1;                                                         //Houdt bij als de trein op zijn bestemming is of niet

//String over uitlezen Serial
String start;
String stop;
String richting;
String noodstop;


void setup() {
  //Seriele comunicatie
  Serial1.begin(115200);                                                      //Als je werkt met de ESP
  Serial.begin(115200);                                                       //Manuele invoer (met pc)

  //Inputs
  //Reedcontacten
  pinMode(inputReedSpoor1Links, INPUT);
  pinMode(inputReedSpoor1Rechts, INPUT);
  pinMode(inputReedZijSpoor1Links, INPUT);
  pinMode(inputReedZijSpoor1Rechts, INPUT);
  pinMode(inputReedSpoor2Links, INPUT);
  pinMode(inputReedSpoor2Rechts, INPUT);
  pinMode(inputReedZijSpoor2Links, INPUT);
  pinMode(inputReedZijSpoor2Rechts, INPUT);
  pinMode(inputReedLinksSpoor1, INPUT);
  pinMode(inputReedLinksSpoor2, INPUT);
  pinMode(inputReedRechtsSpoor1, INPUT);
  pinMode(inputReedRechtsSpoor2, INPUT);

  //Outputs
  //Snelheid regelaar
  pinMode(outputLinks, OUTPUT);
  pinMode(outputRechts, OUTPUT);
  pinMode(outputSpoor1, OUTPUT);
  pinMode(outputSpoor2, OUTPUT);
  //Uitgangen Wissels
  pinMode(outputWisselSporen1_Links, OUTPUT);
  pinMode(outputWisselSporen1_Rechts, OUTPUT);
  pinMode(outputWisselSporen2_Links, OUTPUT);
  pinMode(outputWisselSporen2_Rechts, OUTPUT);
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
    if ((start.equals("Spoor1") || huidigePlaats == 1) && (reedContactTrigger < reedRechtsSpoor1)) {
      if (stop.equals("Spoor1")) {                                              //Moet de trein hier stoppen?
        analogWrite(outputLinks, 0);                                            //Zet active sporen uit
        analogWrite(outputSpoor1, 0);
        bestemming = 1;                                                         //Zorg dat je uit de while loop kunt
        huidigePlaats = 0;                                                      //de trein staat stil dus heeft geen (rijdende) huidige plaats reset deze voor volgende opdracht
      }
      else {                                                                    //Als de trein moet door rijden
        analogWrite(outputLinks, 0);                                            //Zet de vorige sectie zonder spanning zetten
        analogWrite(outputSpoor1, snelheidSpoor1);                              //Als de trein vertrek moeten we op de huidige sectie spanning zetten als de trein al rijd heeft deze sectie al spanning en doet deze lijn niks
        analogWrite(outputRechts, snelheidRechts);                               //Zet op de volgende sectie spanning
        huidigePlaats = 2;                                                      //Gaan naar de volgende stap
      }
    }

    if (huidigePlaats == 2 && reedContactTrigger < reedRechtsSpoor2) {          //if als de trein op het rechter spoor rijdt en het reedcontact voorbij rijdt
      analogWrite(outputSpoor1, 0);                                             //Zet de vorige sectie zonder spanning zetten
      analogWrite(outputSpoor2, snelheidSpoor2);                                //Zet op de volgende sectie spanning
      huidigePlaats = 3;                                                        //Ga naar de volgende stap
    }
    /*
      Deze if bestaat uit 3 delen
      Eerst kijken als Spoor 2 het start punt is
      Of het kan zijn dat de trein moet aankomen in deze sectie (trein rijd al
      Voer pas uit als de trein voorbij het reedcontact rijdt
    */
    if ((start.equals("Spoor2") || huidigePlaats == 3) && (reedContactTrigger < reedSpoor2Links)) {
      if (stop.equals("Spoor2")) {                                              //Moet de trein hier stoppen?
        analogWrite(outputRechts, 0);                                            //Zet active sporen uit
        analogWrite(outputSpoor2, 0);
        bestemming = 1;                                                         //Zorg dat je uit de while loop kunt
        huidigePlaats = 0;                                                      //De trein staat stil dus heeft geen (rijdende) huidige plaats reset deze voor volgende opdracht
      }
      else {
        analogWrite(outputRechts, 0);                                            //Zet de vorige sectie zonder spanning zetten
        analogWrite(outputSpoor2, snelheidSpoor2);                              //Als de trein vertrek moeten we op de huidige sectie spanning zetten als de trein al rijd heeft deze sectie al spanning en doet deze lijn niks
        analogWrite(outputLinks, snelheidLinks);                                //Zet op de volgende sectie spanning
        huidigePlaats = 4;                                                      //Ga naar de volgende stap
      }
    }

    if (huidigePlaats == 4 && (reedContactTrigger < reedLinksSpoor1)) {         //if als de trein op het linker spoor rijdt en het reedcontact voorbij rijdt
      analogWrite(outputSpoor2, 0);                                             //Zet de vorige sectie zonder spanning zetten
      analogWrite(outputSpoor1, snelheidSpoor1);                                //Zet op de volgende sectie spanning
      huidigePlaats = 1;                                                        //Ga naar de volgende stap (start loop opnieuw)
    }
  }
}

void leesReedContacten() {                                                      //Lees alle reedcontacten en zet ze in een variable
  reedSpoor1Links = analogRead(inputReedSpoor1Links);
  reedSpoor1Rechts = analogRead(inputReedSpoor1Rechts);
  reedZijSpoor1Links = analogRead(inputReedZijSpoor1Links);
  reedZijSpoor1Rechts = analogRead(inputReedZijSpoor1Rechts);
  reedSpoor2Links = analogRead(inputReedSpoor2Links);
  reedSpoor2Rechts = analogRead(inputReedSpoor2Rechts);
  reedZijSpoor2Links = analogRead(inputReedZijSpoor2Links);
  reedZijSpoor2Rechts = analogRead(inputReedZijSpoor2Rechts);
  reedLinksSpoor1 = analogRead(inputReedLinksSpoor1);
  reedLinksSpoor2 = analogRead(inputReedLinksSpoor2);
  reedRechtsSpoor1 = analogRead(inputReedRechtsSpoor1);
  reedRechtsSpoor2 = analogRead(inputReedRechtsSpoor2);
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
  analogWrite(outputSpoor1, 0);
  analogWrite(outputZijSpoor1, 0);
  analogWrite(outputSpoor2, 0);
  analogWrite(outputZijSpoor2, 0);
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

  int snelheidSpoor1Index_start = serialInput.indexOf("SS1:") + 4;                                //Zoek de start van snelheid sectie 1
  Index_end = serialInput.indexOf(" ", snelheidSpoor1Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor1 = serialInput.substring(snelheidSpoor1Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidSpoor1 = multiplier * snelheidSpoor1.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor2Index_start = serialInput.indexOf("SS2:") + 4;                                //Zoek de start van snelheid sectie 2
  Index_end = serialInput.indexOf(" ", snelheidSpoor2Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor2 = serialInput.substring(snelheidSpoor2Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidZijSpoor1 = multiplier * snelheidSpoor2.toInt();                                        //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor3Index_start = serialInput.indexOf("SS3:") + 4;                                //Zoek de start van snelheid sectie 3
  Index_end = serialInput.indexOf(" ", snelheidSpoor3Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor3 = serialInput.substring(snelheidSpoor3Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidSpoor2 = multiplier * snelheidSpoor3.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor4Index_start = serialInput.indexOf("SS4:") + 4;                                //Zoek de start van snelheid sectie 4
  Index_end = serialInput.indexOf(" ", snelheidSpoor4Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor4 = serialInput.substring(snelheidSpoor4Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidZijSpoor2 = multiplier * snelheidSpoor4.toInt();                                        //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor5Index_start = serialInput.indexOf("SS5:") + 4;                                //Zoek de start van snelheid sectie 5
  Index_end = serialInput.indexOf(" ", snelheidSpoor5Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor5 = serialInput.substring(snelheidSpoor5Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidLinks = multiplier * snelheidSpoor5.toInt();                                            //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int snelheidSpoor6Index_start = serialInput.indexOf("SS6:") + 4;                                //Zoek de start van snelheid sectie 6
  Index_end = serialInput.indexOf(" ", snelheidSpoor6Index_start);                                //Zoek het einde van de waarde
  String snelheidSpoor6 = serialInput.substring(snelheidSpoor6Index_start, Index_end);            //Maak de string snelheidSpoor1 met begin en eind indexen
  snelheidRechts = multiplier * snelheidSpoor6.toInt();                                           //Zet de string met de waarde om naar een integer en vermenigvuldigd met 2.55 om een waarde te krijgen 0-255 ipv 0-100

  int noodstopIndex_start = serialInput.indexOf("ES:") + 3;                                       //Zoek de start van noodstop
  noodstop = serialInput.substring(noodstopIndex_start);                                          //Maak de string noodstop met begin en eind indexen (einde is het einde van de seriële 'blok' zie readUntil)
}
