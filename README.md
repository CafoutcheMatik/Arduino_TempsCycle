# Arduino_TempsCycle 
 Permet surveiller le temp de cycle sur la base des micro seconde  
 hervé CHUTEAU pour le Bistromatik  
 version 2.05 du 17/03/2026  

 Affichage sur le Terminal :  
 Temps de cycle (9999)  moyen 0.016 ms  max 0.125  >0.030 25  >>0.120 1  
 
 Temps de cycle : Nom donné à l'instance  
 (9999)         : nombre de tour de cycle (affichage limité à 9999)  
 moyen 0.016 ms : temps moyen d'éxécution  
 max 0.128      : temps max constaté  
 \>0.030 25      : Le premier seuil de 0.030 ms a été dépassé 25 fois  
                  (on ne comptabilise pas les dépassements de second seuil)  
 \>>0.120 1      : le second seuil de 0.120 ms a été dépassé qu'une fois  
 
 Création de Class : yTempsCycle TempsCycle("Temps de cycle", 5, 0.03, 0.13);
     "Temps de cycle" : Nom modifiable
			5 :  Nombre de seconde entre deux affichage
     0.030 : Seuil 1 en ms
     0.120 : Seuil 2 en ms
 
 Dans la boucle à surveiller : TempsCycle.loop(true);  
 TempsCycle.newTxt : nouveau affichage formaté
 TempsCycle.texte1 et TempsCycle.texte2 : affichage
 
 Possibilité de mesurer le temps d'une partie de code en plaçant start stop  
 TempsCycle.start();  
     /.   votre code   ./
 TempsCycle.stop();  
 (On laisse TempsCycle.loop(); dans la boucle loop() pour la gestion de l'affichage)  
 
 Pour limiter l'impact sur le temps de cycle la mise en forme est decomposée opération par opération le serial.print se fait 10 caractères par 10
 caractères ce qui limite le temps max à 0,14ms (temps équivalent aux opérations les plus gourmantes de mise en forme)  
 
 sur MEGA2560  avec monitor_speed = 115200  
        Par rapport à un Mega affichant "Hello World"  
        RAM: 182 bytes sur 8192 bytes (2,2%)  pour chaque instance  
        Flash: 4924 bytes sur 253952 bytes (1,94%)  
 
 Temps moyen d'éxécution 0.020ms  par instance  
 Temps maximum 0.14ms (0.200ms si 3 instances)  
 (seulement 25 tours de cycle > 0,030ms)  
 
 Ce qui consomme du temps c'est dtostrf(sValeur, 1, sNbDeci, sp_conversionBuf); pour 112µs environ
