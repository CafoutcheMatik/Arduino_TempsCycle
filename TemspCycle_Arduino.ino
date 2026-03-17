// #pragma GCC optimize("O0")

#include <Arduino.h>
#include "yTempsCycle.h"

/* ********************************
 *     P A R A M E T R A G E      */
//
// déclaration : "Nom de l'instance", interval en seconde, seuil1 en ms, seuil2 en ms
yTempsCycle TempsCycle("Tps de cycle", 4, 0.120, 0.200);
yTempsCycle TempsInstruction("Tps instruction", 4, 0.060, 0.070);
//

unsigned long memMillis;
const unsigned lancerPrintms = 15000ul;

void setup() {
  Serial.begin(115200);  // Terminal
  //
  Serial.print("setup : ");
  Serial.print(millis());
  Serial.println(" ms");
}

void loop() {
  byte test;
  // lancement d'un compteur de partie de programme avec affichage
  TempsInstruction.start();
  test = random(100);
  TempsInstruction.stop();
  //
  TempsInstruction.loop(true);
  //
  // temps d'une boucle (false) sans affichage
  TempsCycle.loop(false);
  //
  // Si besoin de récupérer les informations
  // texte1[50]  pour moyen et max
  // texte2[28]  pour les seuils
  if ((unsigned long)(millis() - memMillis) > lancerPrintms) {
    memMillis = millis();
    Serial.print("-- ");
    Serial.print(TempsCycle.texte1);
    Serial.println(TempsCycle.texte2);
  }
}