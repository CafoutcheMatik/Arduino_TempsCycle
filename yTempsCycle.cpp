#include <Arduino.h>
#include "yTempsCycle.h"

/*
 *  Permet surveiller le temp de cycle sur la base des micro seconde
 *  hervé CHUTEAU pour le Bistromatik
 *  version 2.05 du 17/03/2026
 *
 *  Affichage sur le Terminal :
 *  Temps de cycle (9999)  moyen 0.016 ms  max 0.125  >0.030 25  >>0.120 1
 *
 *  Temps de cycle : Nom donné à l'instance
 *  (9999)         : nombre de tour de cycle (affichage limité à 9999)
 *  moyen 0.016 ms : temps moyen d'éxécution
 *  max 0.128      : temps max constaté
 *  >0.030 25      : Le premier seuil de 0.030 ms a été dépassé 25 fois
 *                   (on ne comptabilise pas les dépassements de second seuil)
 *  >>0.120 1      : le second seuil de 0.120 ms a été dépassé qu'une fois
 *
 *  Création de Class : yTempsCycle TempsCycle("Temps de cycle", 5, 0.03, 0.13);
 *      "Temps de cycle" : Nom modifiable
 * 			5 :  Nombre de seconde entre deux affichage
 *      0.030 : Seuil 1 en ms
 *      0.120 : Seuil 2 en ms
 *
 *  Dans la boucle à surveiller : TempsCycle.loop(true);
 *
 *  TempsCycle.newTxt : nouveau affichage formaté
 *  TempsCycle.texte1 et TempsCycle.texte2 : affichage
 *
 *  Possibilité de mesurer le temps d'une partie de code en plaçant start stop
 *  TempsCycle.start();
 *  /.   votre code   ./
 *  TempsCycle.stop();
 *  (On laisse TempsCycle.loop(); dans la boucle loop() pour la gestion de
 * l'affichage)
 *
 *  Pour limiter l'impact sur le temps de cycle la mise en forme est decomposée
 *  opération par opération le serial.print se fait 10 caractères par 10
 *  caractères ce qui limite le temps max à 0,14ms (temps équivalent aux
 *  opérations les plus gourmantes de mise en forme)
 *
 *  sur MEGA2560  avec monitor_speed = 115200
 *         Par rapport à un Mega affichant "Hello World"
 *         RAM: 182 bytes sur 8192 bytes (2,2%)  pour chaque instance
 *         Flash: 4924 bytes sur 253952 bytes (1,94%)
 *
 *  Temps moyen d'éxécution 0.020ms  par instance
 *  Temps maximum 0.14ms (0.200ms si 3 instances)
 *  (seulement 25 tours de cycle > 0,030ms)
 *
 *  Ce qui consomme du temps c'est
 * dtostrf(sValeur, 1, sNbDeci, sp_conversionBuf); pour 112µs environ
 */
//
const unsigned int maxAff_Compteur = 9999;
const float maxAff_Seuil_a = (float)9.999;
const float maxAff_Seuil_b = (float)99.99;
const float maxAff_Seuil_c = (float)999.9;
const float maxAff_Seuil_d = (float)99999;
const unsigned long ms_to_us = 1000ul;
const unsigned long sec_to_us = 1000ul * 1000ul;
const unsigned long sec_to_ms = 1000ul;
const unsigned long maxintervallePrintSec = 3600;
const int nbCharMaxparPrint = 8;
// const unsigned int seuil_U_INT = 65535;
const unsigned long seuil_U_LONG = 4294967295ul;

bool yTempsCycle::printEnCours = false;
bool yTempsCycle::calculEnCours = false;

// **********  D E C L A R A T I O N    D E    F O N C T I O N
void ajout_au_texte(char* sTexte, PGM_P sAvant, float& sValeur, byte& sNbDeci);
byte calcul_nb_decimale(float& sValeur);
bool envoiWrite(char* sp_texte, int sp_indexCar);

// ********** C O N S T R U C T E U R
yTempsCycle::yTempsCycle(const char sNomCycle[15] PROGMEM,
                         const unsigned int sIntervallePrintSec,
                         const float sSeuilms1, const float sSeuilms2) {
  //
  if (sIntervallePrintSec > (maxintervallePrintSec)) {
    intervallePrint_us = maxintervallePrintSec * 1000000ul;
    ;
  } else {
    intervallePrint_us = sIntervallePrintSec * 1000000ul;
  }
  //
  if (sSeuilms2 > maxAff_Seuil_d) {
    seuil2 = (unsigned long)(maxAff_Seuil_d * 1000.00f);
    f_tempVal = maxAff_Seuil_d;
  } else {
    seuil2 = (unsigned long)(sSeuilms2 * 1000.00f);
    f_tempVal = sSeuilms2;
  }
  //
  if (sSeuilms1 > maxAff_Seuil_d) {
    seuil1 = seuil2;
  } else {
    if (sSeuilms1 < f_tempVal) {
      seuil1 = (unsigned long)(sSeuilms1 * 1000.00f);
    } else {
      seuil1 = (unsigned long)(f_tempVal * 1000.00f);
    }
  }
  //
  memset(NomCycle, 0, 16);
  strncpy(NomCycle, sNomCycle, 15);
  //
  sp_limite =
      (unsigned long)seuil_U_LONG - intervallePrint_us - (unsigned long)10;
  //
  strcpy_P(texte1, PSTR(""));
  strcpy_P(texte2, PSTR(""));
}
// ********** L O O P
void yTempsCycle::loop(bool AffichageSurTerminal) {
  //
  unsigned long sp_microsVal;
  unsigned long sp_dureeCycle;
  //
  //
  sp_microsVal = micros();
  //
  if (valCompteur < seuil_U_LONG) {
    valCompteur++;
  }

  // ******************    C A L C U L   A C T I F
  if (!CalculActif) {
    if (sp_microsVal < sp_limite) {
      CalculActif = true;
      ValTpsMax = 0ul;
      nbSeuil1 = 0;
      nbSeuil2 = 0;
      valCompteur = 0ul;
      cumulDuree = 0ul;
      memMicros = 0ul;
    }
  } else {
    // calcul max/seuils
    if (memMicros > 0ul) {
      sp_dureeCycle = sp_microsVal - memMicros;
      cumulDuree += sp_dureeCycle;
      if (!modeSartStop) {
        if (sp_dureeCycle > ValTpsMax) {
          if (sp_dureeCycle >= (unsigned long)maxAff_Seuil_d) {
            ValTpsMax = maxAff_Seuil_d;
          } else {
            ValTpsMax = sp_dureeCycle;
          }
        }
        if (sp_dureeCycle > seuil2) {
          if (nbSeuil2 < maxAff_Compteur) {  // On limite à 9999
            nbSeuil2++;
          }
        } else {
          if (sp_dureeCycle > seuil1) {
            if (nbSeuil1 < maxAff_Compteur) {
              nbSeuil1++;
            }
          }
        }
      }
    }

    //
    if (sp_microsVal > sp_limite) {
      CalculActif = false;  // limite de débordement
    } else {
      memMicros = sp_microsVal;  // pour ne pas perdre une micro seconde
    }
    gestionAffichage(AffichageSurTerminal);
  }
}
// ******************    S T A R T
void yTempsCycle::start() {
  modeSartStop = true;
  memMicrosStartStop = micros();
}
//
// ******************    S T O P
void yTempsCycle::stop() {
  unsigned long sp_microsVal;
  unsigned long sp_dureeCycle;
  //
  sp_microsVal = micros();
  //
  if (valCompteurStartStop < seuil_U_LONG) {
    valCompteurStartStop++;
  }
  //
  if ((memMicros > 0ul) && CalculActif && modeSartStop) {
    sp_dureeCycle = sp_microsVal - memMicrosStartStop;
    cumulDureeStartStop += sp_dureeCycle;

    if (sp_dureeCycle > ValTpsMax) {
      ValTpsMax = sp_dureeCycle;
    }
    if (sp_dureeCycle > seuil2) {
      if (nbSeuil2 < maxAff_Compteur) {  // On limite à 9999
        nbSeuil2++;
      }
    } else {
      if (sp_dureeCycle > seuil1) {
        if (nbSeuil1 < maxAff_Compteur) {
          nbSeuil1++;
        }
      }
    }
  }
}
//
//
// ******************    G E S T I O N    A F F I C H A G E
void yTempsCycle::gestionAffichage(bool sp_AffichageSurTerminal) {
  //
  if ((cumulDuree > intervallePrint_us) && (printTour < 1)) {
    if (!modeSartStop) {
      valCompteurPrint = valCompteur;
      cumulDureePrint = cumulDuree;
    } else {
      valCompteurPrint = valCompteurStartStop;
      cumulDureePrint = cumulDureeStartStop;
    }
    valTpsMaxPrint = ValTpsMax;
    nbSeuil1Print = nbSeuil1;
    nbSeuil2Print = nbSeuil2;
    // Reset des compteurs
    ValTpsMax = 0ul;
    nbSeuil1 = 0;
    nbSeuil2 = 0;
    valCompteur = 0ul;
    cumulDuree = 0ul;
    valCompteurStartStop = 0ul;
    cumulDureeStartStop = 0ul;
    //
    printTour = 1;
  }
  //
  if (printTour > 0) {
    switch (printTour) {
      case 1:  // ******************    Capture des valeurs

        if (!calculEnCours) {
          calculEnCours = true;
          printTour++;
        }
        break;
      case 2:  // ******************    TEXTE 1 : "Nom"
        strcpy(texte1, NomCycle);
        printTour++;
        break;
      case 3:  // ******************    TEXTE 1 : nombre de tour
        if ((valCompteurPrint > maxAff_Compteur)) {
          f_tempVal = maxAff_Compteur;
        } else if (valCompteurPrint > 0) {
          f_tempVal = (float)valCompteurPrint;
        }
        printTour++;
        break;
      case 4:
        nb_decimale = 0;
        ajout_au_texte(texte1, PSTR(" ("), f_tempVal, nb_decimale);
        printTour++;
        break;
      case 5:  // ******************    TEXTE 1
        if ((valCompteurPrint > 0)) {
          f_tempVal = ((float)cumulDureePrint / (float)valCompteurPrint) /
                      (float)1000.00;
        } else {
          f_tempVal = 0;
        }
        printTour++;
        break;
      case 6:
        if (f_tempVal > maxAff_Seuil_d) {
          f_tempVal = maxAff_Seuil_d;
        }
        nb_decimale = calcul_nb_decimale(f_tempVal);
        printTour++;
        break;
      case 7:
        if (valCompteurPrint > 0) {
          ajout_au_texte(texte1, PSTR(")  moyen "), f_tempVal, nb_decimale);
          printTour++;
        } else {
          strcat_P(texte1, PSTR(")"));
          printTour = 11;  // saut si pas d'affichage moyen
        }
        break;
      case 8:
        f_tempVal = (float)valTpsMaxPrint / (float)1000.00;
        printTour++;
        break;
      case 9:
        nb_decimale = calcul_nb_decimale(f_tempVal);
        printTour++;
        break;
      case 10:
        ajout_au_texte(texte1, PSTR(" ms  max "), f_tempVal, nb_decimale);
        printTour++;
        break;
      case 11:  // ******************    TEXTE 2
        strcpy_P(texte2, PSTR(""));
        printTour++;
        break;
      case 12:
        f_tempVal = (float)seuil1 / (float)1000;
        printTour++;
        break;
      case 13:
        nb_decimale = calcul_nb_decimale(f_tempVal);
        printTour++;
        break;
      case 14:
        ajout_au_texte(texte2, PSTR("  >"), f_tempVal, nb_decimale);
        printTour++;
        break;
      case 15:
        nb_decimale = 0;
        f_tempVal = (float)nbSeuil1Print;
        printTour++;
        break;
      case 16:
        ajout_au_texte(texte2, PSTR(" "), f_tempVal, nb_decimale);
        printTour++;
        break;
      case 17:
        f_tempVal = (float)seuil2 / (float)1000;
        printTour++;
        break;
      case 18:
        nb_decimale = calcul_nb_decimale(f_tempVal);
        printTour++;
        break;
      case 19:
        ajout_au_texte(texte2, PSTR("  >>"), f_tempVal, nb_decimale);
        printTour++;
        break;
      case 20:
        nb_decimale = 0;
        f_tempVal = (float)nbSeuil2Print;
        ajout_au_texte(texte2, PSTR(" "), f_tempVal, nb_decimale);
        printTour++;
        break;
        //
      case 21:  // ****************    GESTION ENVOI
        newTxt = true;
        printTour++;
        break;
      case 22:  // Mutex
        if (!sp_AffichageSurTerminal) {
          printTour = 99;  // pour aller à "default:"
        } else {
          if (!printEnCours) {
            printEnCours = true;
            aux_nbStartEnvoi = 0;
            printTour++;
          }
        }
        break;
      case 23:  // Envoi Série Texte 1
        // le Serial.print d'un caractère c'est 8 μs en arduino on va se limiter
        // à 0,130ms soit 10 caractères constante nbCharMaxparPrin
        if (envoiWrite(texte1, aux_nbStartEnvoi)) {  // true si envoi fait
          if (aux_nbStartEnvoi > (int)strlen(texte1)) {
            aux_nbStartEnvoi = 0;
            printTour++;
          } else {
            aux_nbStartEnvoi += nbCharMaxparPrint;
          }
        }
        break;
      case 24:  // Envoi Série  Texte 2
        if (envoiWrite(texte2, aux_nbStartEnvoi)) {
          if (aux_nbStartEnvoi > (int)strlen(texte2)) {
            aux_nbStartEnvoi = 0;
            printTour++;
          } else {
            aux_nbStartEnvoi += nbCharMaxparPrint;
          }
        }
        break;
      case 25:
        Serial.println("");
        printTour = 99;  // pour aller à "default:"
        break;
      default:
        printTour = 0;
        printEnCours = false;
        calculEnCours = false;
        break;
    }
  }
}

void ajout_au_texte(char* sTexte, PGM_P sAvant, float& sValeur, byte& sNbDeci) {
  // valeur max 99999 ou 999.9
  char sp_conversionBuf[10];
  strcat_P(sTexte, sAvant);
  dtostrf(sValeur, 1, sNbDeci, sp_conversionBuf);
  strcat(sTexte, sp_conversionBuf);
}

byte calcul_nb_decimale(float& sValeur) {
  byte val_return = 0;
  if (sValeur > maxAff_Seuil_c) {
    val_return = 0;
  } else {
    if (sValeur > maxAff_Seuil_b) {
      val_return = 1;
    } else {
      if (sValeur > maxAff_Seuil_a) {
        val_return = 2;
      } else {
        val_return = 3;
      }
    }
  }
  return val_return;
}

bool envoiWrite(char* sp_texte, int sp_indexCar) {
  bool aux_return = false;
  int aux_strlen = (int)strlen(sp_texte);
  //
  if ((aux_strlen - sp_indexCar) > 0) {
    if ((unsigned int)Serial.availableForWrite() > nbCharMaxparPrint) {
      if ((aux_strlen - sp_indexCar) > nbCharMaxparPrint) {
        Serial.write(sp_texte + (size_t)sp_indexCar, (size_t)nbCharMaxparPrint);
      } else {
        Serial.write(sp_texte + (size_t)sp_indexCar,
                     (size_t)(aux_strlen - sp_indexCar));
      }
      aux_return = true;
    }
  } else {
    aux_return = true;
  }
  return aux_return;
}
