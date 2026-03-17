#ifndef xTEMPSCYCLE_H
#define xTEMPSCYCLE_H

#include <Arduino.h>  // Nécessaire pour byte, Serial, etc.

class yTempsCycle {
 private:
  unsigned long intervallePrint_us =0ul;
  unsigned long seuil1 =0ul;
  unsigned long seuil2 =0ul;
  //
  unsigned long valCompteur =0ul;
  unsigned long valCompteurStartStop =0ul;
  unsigned long ValTpsMax =0ul;
  unsigned int nbSeuil1 = 0;
  unsigned int nbSeuil2 =0;

  bool modeSartStop = false;
  //
  bool CalculActif = false;
  //
  // pour calcul durée
  unsigned long memMicros =0ul;
  unsigned long  cumulDuree =0ul;
  // pour calcul durée Start Stop
  unsigned long memMicrosStartStop =0ul;
  unsigned long  cumulDureeStartStop =0ul;
  //
  float f_tempVal = 0.0f;
  //
  // Variables figées pour l'impression
  unsigned long valCompteurPrint =0ul;
  unsigned long cumulDureePrint =0ul;
  unsigned long valTpsMaxPrint =0ul;
  unsigned int nbSeuil1Print =0;
  unsigned int nbSeuil2Print=0;
  // Machine à état
  byte printTour = 0;
  byte nb_decimale = 0;
  unsigned long sp_limite =0ul;
 bool aux_envoiOK = false;
 int aux_nbStartEnvoi = 0;
  //

  void gestionAffichage(bool sp_AffichageSurTerminal);

 public:
  yTempsCycle(const char sNomCycle[15] PROGMEM,
              const unsigned int sIntervallePrintSec, const float sSeuilms1,
              const float sSeuilms2);

  void loop(bool AffichageSurTerminal);
  void start();
  void stop();

  // CES VARIABLES SONT CONSERVÉES POUR ACCÈS EXTERNE
  char texte1[50];  // Taille suffisante pour une ligne
  char texte2[28];
  char NomCycle[16];    // Taille du nom
  bool newTxt = false;  // Indique qu'un nouveau texte est prêt

  // Global à toutes les instances
  static bool printEnCours ;
  static bool calculEnCours ;

};

#endif