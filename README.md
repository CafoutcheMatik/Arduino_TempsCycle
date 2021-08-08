# Arduino_TempsCycle
Affichage du temps de cycle de la boucle LOOP

pour surveiller le temp de cycle sur la base des micro seconde

hervé CHUTEAU pour le Bistromatik

version 0.5 du 06/08/2021

Le temps de cycle pris par ce module, aussi bien à 9600 bauds qu'à 250000 bauds est de 0.12 ms environ

occupation    RAM 168 bytes     Flash 3888 bytes

Développé avec un MÉGA2560 REV3

Développé sous Visual Studio Code  1.57.1 et PlatformIO Core 5.1.1 Home 3.3.4

  
Affichage typique 

_ temps moyen

_ temps max

_ nombre de cycle avec un temps supérieur au seuil 1 (ici 5 ms)

_ nombre de cycle avec un temps supérieur au seuil 2 (ici 10 ms)


exemple "Temps de cycle en ms   moyen 4.02   max 4.03   >5.00 0   >10.00 0"
 
## Paramètrage ##

cyc_intervallePrint = fréquence d'affichage (5 000 = 5 sec)

cyc_seuil1 = valeur en ms de la surveillance seuil 1 (5.00 = 5 ms)

cyc_seuil2 = valeur en ms de la surveillance seuil 2 (10.00 = 10 ms)

Dans la boucle Setup : xTempsCycleParam(5000.00,5.00,10.00); //temps en millisecondes


## Lancement du programme ##

xTempsCycle(sSerialPrint) dans la boucle Loop

la surveillance est lancée et le paramètre sSerialPrint à true imprime le résultat sur le terminal

 
## Variables disponibles ##

cyc_CharLCD1[]   Première ligne de l'afficheur LCD  avec le temps moyen et le temsp max

cyc_CharLCD2[]   Seconde ligne de l'afficheur LCD  avec le nombre de cycle supérieur aux seuils definis

cyc_newLCD       Bool prévenant d'un nouvel affichage. Il est à reseter par l'utilisateur.
