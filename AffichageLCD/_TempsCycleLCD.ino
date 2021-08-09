#include <Arduino.h>
#include "z02_wire.h" // configuration TimeOut dans twi.c Version 2020
// Bibliothèque LiquidCrystal_I2C
// https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include "z01_LiquidCrystal_I2C.h"
#include "xTempsCycle.h"

/* ********************************
*  Permet de visualiser 
*  le temps de cycle sur un afficheur
*  LCD 2x16 caractère             
*  temps utilisé par le LCD 24ms environ */

// Affichage via ecran LCD 2x16 caractères
bool LCDPresent = false;
const byte config_ADRESS_LCD = 39; //Adresse I2C de l'afficheur (@dresse par défaut)
const byte config_DIGITS_LCD = 16;
const byte Config_LIGNES_LCD = 2;
// Déclaration de l'afficheur
LiquidCrystal_I2C lcd(config_ADRESS_LCD, config_DIGITS_LCD, Config_LIGNES_LCD); // (Adresse,digits,lignes)

/* ********************************
*            S E T U P           */
void setup()
{
  Serial.begin(250000);
  xTempsCycleParam(5000.00, 0.05, 0.10); //temps en millisecondes

  Wire.begin();
  // contrôle de présence du LCD
  Wire.beginTransmission(config_ADRESS_LCD);

  if (Wire.endTransmission() == 0)
  {
    LCDPresent = true;
  }
  else
  {
    LCDPresent = false;
  }

  if (LCDPresent)
  {
    lcd.begin();
  }
}

/* ********************************
*              L O O P           */
void loop()
{
  xTempsCycle(true);

  static int LCDTour = 0;
  static unsigned long LCDmillis = 0ul;
  const unsigned long LCDdelay = 50ul; //50 ms entre chaque opération
  if (cyc_newLCD && (LCDTour == 0))
  {
    LCDTour = 1;
  }
  switch (LCDTour)
  {
  case 1:
    if (LCDmillis == 0)
    {
      lcd.clear(); // effacement
      LCDmillis = millis();
    }
    if ((LCDmillis - millis()) > LCDdelay)
    {
      LCDTour++;
      LCDmillis = 0ul;
    }
    break;
  case 2:
    if (LCDmillis == 0)
    {
      lcd.setCursor(0, 0); // première ligne
      LCDmillis = millis();
    }
    if ((LCDmillis - millis()) > LCDdelay)
    {
      LCDTour++;
      LCDmillis = 0ul;
    }
    break;
  case 3:
    if (LCDmillis == 0)
    {
      lcd.print(cyc_CharLCD1); // première ligne
      LCDmillis = millis();
    }
    if ((LCDmillis - millis()) > LCDdelay)
    {
      LCDTour++;
      LCDmillis = 0ul;
    }
    break;
  case 4:
    if (LCDmillis == 0)
    {
      lcd.setCursor(0, 1); // seconde ligne
      LCDmillis = millis();
    }
    if ((LCDmillis - millis()) > LCDdelay)
    {
      LCDTour++;
      LCDmillis = 0ul;
    }
    break;
  case 5:
    if (LCDmillis == 0)
    {
      lcd.print(cyc_CharLCD2); // seconde ligne
      LCDmillis = millis();
    }
    if ((LCDmillis - millis()) > LCDdelay)
    {
      LCDTour++;
      LCDmillis = 0ul;
    }
    break;

  default:
    LCDTour = 0;
    LCDmillis = 0ul;
    cyc_newLCD = false;
    break;
  }
}
