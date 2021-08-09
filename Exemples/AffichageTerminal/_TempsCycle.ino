#include <Arduino.h>
#include "xTempsCycle.h"

/* ********************************
*  Permet de visualiser 
*  le temps de cycle pris par le module
*  avec sortie sur le Terminal
*  sur MÉGA2560  moyen 0.02ms max 0.12ms 

/* ********************************
*            S E T U P           */
void setup()
{
  Serial.begin(250000);
  xTempsCycleParam(5000.00,0.05,0.10); //temps en millisecondes
}

/* ********************************
*              L O O P           */
void loop()
{
  xTempsCycle(true);
}
