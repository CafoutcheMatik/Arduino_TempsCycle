#include <Arduino.h>
#include "xTempsCycle.h"
/*
*  pour surveiller le temp de cycle sur la base des micro seconde
*  hervé CHUTEAU pour le Bistromatik
*  version 0.5 du 06/08/2021
*
*  Le temps de cycle pris par ce module, aussi bien à 9600 bauds qu'à 250000 bauds,
*  est au max de 0.12 ms environ
*  occupation mémoire    RAM 218 bytes 2.7%     Flash 4928 bytes 1,9%
*  Développé avec un MÉGA2560 REV3
*  Développé sous Visual Studio Code  1.57.1 et PlatformIO Core 5.1.1 Home 3.3.4
*  
*  Affichage typique sur le terminal
*  _ temps moyen
*  _ temps max
*  _ nombre de cycle avec un temps supérieur au seuil 1 (ici 5 ms)
*  _ nombre de cycle avec un temps supérieur au seuil 2 (ici 10 ms)
*
*  exemple "Temps de cycle en ms   moyen 0.02   max 0.12   >5.00 0   >10.00 0"
*  
*  Paramètrage 
*  cyc_intervallePrint = fréquence d'affichage (5 000 = 5 sec)
*          Ne pas choisir une fréquence trop rapide pour ne pas saturer l'affichage
*          et être au minimum à un centaine de fois le temps cycle moyen    
*  cyc_seuil1 = valeur en ms de la surveillance seuil 1 (5.00 = 5 ms)
*  cyc_seuil2 = valeur en ms de la surveillance seuil 2 (10.00 = 10 ms)
*  Dans la boucle Setup : xTempsCycleParam(5000.00,5.00,10.00); //temps en millisecondes
*
*  Lancement du programme dans la boucle Loop xTempsCycle(sSerialPrint)
*  la surveillance est lancée et le paramètre sSerialPrint à true imprime le résultat sur le terminal
*  
*  Variables disponibles
*  cyc_CharLCD1[]   Première ligne de l'afficheur LCD  avec le temps moyen et le temsp max
*  cyc_CharLCD2[]   Seconde ligne de l'afficheur LCD  avec le nombre de cycle supérieur aux seuils definis
*  cyc_newLCD       Bool prévenant d'un nouvel affichage. Il est à reseter par l'utilisateur.
*  exemple :  LCD1 "m  0.02  M  0.12"   LCD2 ">     0 >>     0"
*/
// P A R A M E T R E S
// Fréquence d'affichage
unsigned long cyc_intervallePrint = 5000000ul; // 5 000 000 = 5 sec
// surveillance des temps de cycle long
unsigned long cyc_seuil1 = 5000ul;	// Temps seuil 1 en μs
unsigned long cyc_seuil2 = 10000ul; // Temps seuil 2 en μs
//
//   Déclarations
//
unsigned long cyc_micros, cyc_memMicros;
unsigned long cyc_dureeCycle; // calcul du temps de cycle actuel
//
unsigned long cyc_compteur, cyc_compteurPrint; // nombre de tour programme
unsigned long cyc_cumulDuree, cyc_cumulDureePrint;
unsigned long cyc_max, cyc_maxPrint;
float cyc_maxMs;
// surveillance des temps de cycle long
unsigned int cyc_nbSeuil1, cyc_nbSeuil1Print, cyc_nbSeuil2, cyc_nbSeuil2Print;
float cyc_seuil1Ms, cyc_seuil2Ms;
// temps de cycle moyen
float cyc_moyenMs;
//
bool cyc_CalculActif = false;
//
byte cyc_printTour = 0; // c'est le moment d'imprimer
// constante du dépassement
const unsigned long cyc_seuil_LONG = 4294967295ul;
// longueur des char array
const int NB_CHAR_CHAINE = 80;
const int NB_CHAR_LCD = 16;
const int NB_CHAR_AUX = 10;
const int NB_CHAR_SERIAL = 10;
int cyc_IndexCharSerial, cyc_IndexAux, cyc_LenChaine;
bool cyc_SerialPrint = false;
//
char cyc_CharChaine[NB_CHAR_CHAINE];
char cyc_CharLCD1[NB_CHAR_LCD + 1];
char cyc_CharLCD2[NB_CHAR_LCD + 1];
bool cyc_newLCD;
char cyc_CharAux[NB_CHAR_AUX];
char cyc_CharSerial[NB_CHAR_SERIAL + 1];
//
const unsigned long cyc_limite = cyc_seuil_LONG - cyc_intervallePrint - 10;
// déclaration des procédures internes
void xTempsCycle_concat_char(char *spCharChaine, unsigned int sCharSize, char *spCharAux);
void xTempsCycle_concat_const(char *spCharChaine, unsigned int sCharSize, const char *spCharAux);
//
/* ******************************************************
*    P A R A M E T R A G E    T E M P S   C Y C L E    */
void xTempsCycleParam(const float sIntervallePrint, const float sSeuil1, const float sSeuil2)
{
	cyc_intervallePrint = sIntervallePrint * 1000.00; // 5 000 000 = 5 sec
	// surveillance des temps de cycle long
	if (sSeuil1 < sSeuil2)
	{
		cyc_seuil1 = sSeuil1 * 1000.00; // Temps seuil 1 en μs
	}
	else
	{
		cyc_seuil1 = sSeuil2 * 1000.00;
	}
	cyc_seuil2 = sSeuil2 * 1000.00; // Temps seuil 2 en μs
}
/* *****************************
*    T E M P S   C Y C L E    */
void xTempsCycle(bool sSerialPrint)
{
	cyc_micros = micros(); // photo de registre microseconde
	cyc_compteur++;

	if (!cyc_CalculActif)
	{
		// on est au premier tour ou au retour de débordement
		if (cyc_micros < cyc_limite)
		{
			// on ne fait quelque chose que si inférieur limite haute (débordement)
			cyc_CalculActif = true;
			cyc_max = 0ul;
			cyc_nbSeuil1 = 0;
			cyc_nbSeuil2 = 0;
			cyc_compteur = 0ul;
			cyc_cumulDuree = 0ul;
			cyc_memMicros = 0ul;
		}
	}
	else
	{
		if (cyc_memMicros > 0ul)
		{
			cyc_dureeCycle = cyc_micros - cyc_memMicros;
			cyc_cumulDuree += cyc_dureeCycle;
			// calcul max et seuils
			if (cyc_dureeCycle > cyc_max)
			{
				cyc_max = cyc_dureeCycle;
			}
			if (cyc_dureeCycle > cyc_seuil2)
			{
				cyc_nbSeuil2++;
			}
			else if (cyc_dureeCycle > cyc_seuil1)
			{
				cyc_nbSeuil1++;
			}
		}
		// doit-on impprimer
		if ((cyc_cumulDuree > cyc_intervallePrint) && (cyc_printTour < 1))
		{
			cyc_printTour = 1; // lance les opération Serial.print
		}

		switch (cyc_printTour)
		{
		case 1:
			cyc_compteurPrint = cyc_compteur;
			cyc_cumulDureePrint = cyc_cumulDuree;
			cyc_maxPrint = cyc_max;
			cyc_nbSeuil1Print = cyc_nbSeuil1;
			cyc_nbSeuil2Print = cyc_nbSeuil2;
			//
			cyc_max = 0ul;
			cyc_nbSeuil1 = 0;
			cyc_nbSeuil2 = 0;
			cyc_compteur = 0ul;
			cyc_cumulDuree = 0ul;
			//
			cyc_printTour++;
			break;
		case 2:
			cyc_moyenMs = ((float)cyc_cumulDureePrint / (float)cyc_compteurPrint) / 1000.00;
			//
			cyc_printTour++;
			break;
		case 3:
			cyc_maxMs = (float)cyc_maxPrint / 1000.00;
			//
			cyc_printTour++;
			break;
		case 4:
			cyc_seuil1Ms = (float)cyc_seuil1 / 1000.00;
			//
			cyc_printTour++;
			break;
		case 5:
			cyc_seuil2Ms = (float)cyc_seuil2 / 1000.00;
			//
			cyc_printTour++;
			break;
		case 6:
			strcpy_P(cyc_CharChaine, PSTR("Temps de cycle en ms   moyen ")); //moyen
			//
			cyc_printTour++;
			break;
		case 7:
			//sprintf ne fonctionne pas avec des float sur Arduino
			dtostrf(cyc_moyenMs, 4, 2, cyc_CharAux); //moyen
			//
			cyc_printTour++;
			break;
		case 8:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux); //moyen
			//
			cyc_printTour++;
			break;
		case 9:
			xTempsCycle_concat_const(cyc_CharChaine, sizeof(cyc_CharChaine), PSTR("   max "));
			//
			cyc_printTour++;
			break;
		case 10:
			dtostrf(cyc_maxMs, 4, 2, cyc_CharAux); //max
			//
			cyc_printTour++;
			break;
		case 11:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux); //max
			//
			cyc_printTour++;
			break;
		case 12:
			xTempsCycle_concat_const(cyc_CharChaine, sizeof(cyc_CharChaine), PSTR("   >"));
			//
			cyc_printTour++;
			break;
		case 13:
			dtostrf(cyc_seuil1Ms, 4, 2, cyc_CharAux); //seuil1
			//
			cyc_printTour++;
			break;
		case 14:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux); //seuil1
			//
			cyc_printTour++;
			break;
		case 15:
			xTempsCycle_concat_const(cyc_CharChaine, sizeof(cyc_CharChaine), PSTR(" "));
			//
			cyc_printTour++;
			break;
		case 16:
			itoa(cyc_nbSeuil1Print, cyc_CharAux, 10); //seuil1
			//
			cyc_printTour++;
			break;
		case 17:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux);
			//
			cyc_printTour++;
			break;
		case 18:
			xTempsCycle_concat_const(cyc_CharChaine, sizeof(cyc_CharChaine), PSTR("   >>")); //seuil2
			//
			cyc_printTour++;
			break;
		case 19:
			dtostrf(cyc_seuil2Ms, 4, 2, cyc_CharAux); //seuil2
			//
			cyc_printTour++;
			break;
		case 20:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux); //seuil2
			//
			cyc_printTour++;
			break;
		case 21:
			xTempsCycle_concat_const(cyc_CharChaine, sizeof(cyc_CharChaine), PSTR(" ")); //seuil2
			//
			cyc_printTour++;
			break;
		case 22:
			itoa(cyc_nbSeuil2Print, cyc_CharAux, 10); //seuil2
			//
			cyc_printTour++;
			break;
		case 23:
			xTempsCycle_concat_char(cyc_CharChaine, sizeof(cyc_CharChaine), cyc_CharAux); //seuil2
			//
			cyc_printTour++;
			break;
		case 24:
			strcpy_P(cyc_CharLCD1, PSTR("m")); //moyen     L C D
			//
			cyc_printTour++;
			break;
		case 25:
			if (cyc_moyenMs < 999.99)
			{
				dtostrf(cyc_moyenMs, 6, 2, cyc_CharAux); //moyen    L C D
			}
			else
			{
				strcpy_P(cyc_CharAux, PSTR("999.99"));
			}
			xTempsCycle_concat_char(cyc_CharLCD1, sizeof(cyc_CharLCD1), cyc_CharAux); //moyen    L C D
			//
			cyc_printTour++;
			break;
		case 26:
			xTempsCycle_concat_const(cyc_CharLCD1, sizeof(cyc_CharLCD1), PSTR("  M")); //max    L C D
			//
			cyc_printTour++;
			break;
		case 27:
			if (cyc_moyenMs < 999.99)
			{
				dtostrf(cyc_maxMs, 6, 2, cyc_CharAux); //max    L C D
			}
			else
			{
				strcpy_P(cyc_CharAux, PSTR("999.99"));
			}
			xTempsCycle_concat_char(cyc_CharLCD1, sizeof(cyc_CharLCD1), cyc_CharAux); //max    L C D
			//
			cyc_printTour++;
			break;
		case 28:
			strcpy_P(cyc_CharLCD2, PSTR(">  ")); //seuil1   L C D
			//
			cyc_printTour++;
			break;
		case 29:
			if (cyc_nbSeuil1Print <= 9)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR("   "));
			}
			else if (cyc_nbSeuil1Print <= 99)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR("  "));
			}
			else if (cyc_nbSeuil1Print <= 999)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR(" "));
			}
			//
			cyc_printTour++;
			break;
		case 30:
			if (cyc_nbSeuil1Print < 9999)
			{
				itoa(cyc_nbSeuil1Print, cyc_CharAux, 10);
			}
			else
			{
				strcpy_P(cyc_CharAux, PSTR("9999"));
			}
			//
			cyc_printTour++;
			break;
		case 31:
			xTempsCycle_concat_char(cyc_CharLCD2, sizeof(cyc_CharLCD2), cyc_CharAux); // Seuil 1  L C D
			//
			cyc_printTour++;
			break;
		case 32:
			xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR(" >>  ")); //seuil2    L C D
			//
			cyc_printTour++;
			break;
		case 33:
			if (cyc_nbSeuil2Print <= 9)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR("   "));
			}
			else if (cyc_nbSeuil2Print <= 99)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR("  "));
			}
			else if (cyc_nbSeuil1Print <= 999)
			{
				xTempsCycle_concat_const(cyc_CharLCD2, sizeof(cyc_CharLCD2), PSTR(" "));
			}
			//
			cyc_printTour++;
			break;
		case 34:
			if (cyc_nbSeuil1Print < 9999)
			{
				itoa(cyc_nbSeuil2Print, cyc_CharAux, 10);
			}
			else
			{
				strcpy_P(cyc_CharAux, PSTR("9999"));
			}
			//
			cyc_printTour++;
			break;
		case 35:
			xTempsCycle_concat_char(cyc_CharLCD2, sizeof(cyc_CharLCD2), cyc_CharAux); // Seuil 2    L C D
			cyc_newLCD = true;
			//
			cyc_printTour++;
			break;
		case 36:
			if (sSerialPrint)
			{
				if (Serial.availableForWrite() >= NB_CHAR_SERIAL)
				{
					if (!cyc_SerialPrint)
					{
						cyc_LenChaine = strlen(cyc_CharChaine);
						for (int i = 0; i < NB_CHAR_SERIAL; i++)
						{
							cyc_IndexAux = cyc_IndexCharSerial + i;
							if (cyc_IndexAux < cyc_LenChaine)
							{
								cyc_CharSerial[i] = cyc_CharChaine[cyc_IndexAux];
							}
							else
							{
								cyc_CharSerial[i] = 0;
							}
						}
						cyc_IndexCharSerial += NB_CHAR_SERIAL;
						cyc_CharSerial[NB_CHAR_SERIAL] = '\0';
						cyc_SerialPrint = true;
					}
					else
					{
						Serial.write(cyc_CharSerial);
						cyc_SerialPrint = false;
					}
				}

				if ((cyc_IndexCharSerial >= cyc_LenChaine) && !cyc_SerialPrint)
				{
					Serial.println(F(""));
					cyc_IndexCharSerial = 0;
					cyc_SerialPrint = false;
					cyc_printTour++;
				}
			}
			else
			{
				cyc_IndexCharSerial = 0;
				cyc_SerialPrint = false;
				cyc_printTour++;
			}
			break;
		default:
			cyc_printTour = 0;
			break;
		}
	}
	//
	if (cyc_micros > cyc_limite)
	{
		cyc_CalculActif = false; // limite de débordement
	}
	else
	{
		cyc_memMicros = cyc_micros; // pour ne pas perdre une micro seconde
	}
}
/* ************************************
*      C O N C A T E N A T I O N     
*  fait une concaténation de char array
*  en vérifiant le débordement
*
*/
void xTempsCycle_concat_char(char *spCharChaine, unsigned int sCharSize, char *spCharAux)
{

	if ((strlen(spCharChaine) + strlen(spCharAux)) < sCharSize)
	{
		strcat(spCharChaine, spCharAux);
	}
	else
	{
		for (unsigned int i = strlen(spCharChaine); i < (sCharSize - 1); i++)
		{
			spCharChaine[i] = '#';
		}

		spCharChaine[sCharSize - 1] = '\0';
	}
}
/* ************************************
*      C O N C A T E N A T I O N     
* fait une concaténation de texte constant
* en vérifiant le débordement
*/
void xTempsCycle_concat_const(char *spCharChaine, unsigned int sCharSize, const char *spCharAux)
{
	if ((strlen(spCharChaine) + strlen_P(spCharAux)) < sCharSize)
	{
		strcat_P(spCharChaine, spCharAux);
	}
	else
	{
		for (unsigned int i = strlen(spCharChaine); i < (sCharSize - 1); i++)
		{
			spCharChaine[i] = '#';
		}
		spCharChaine[sCharSize - 1] = '\0';
	}
}
