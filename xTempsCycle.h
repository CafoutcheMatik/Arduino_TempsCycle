#ifndef xTEMPSCYCLE_H
#define xTEMPSCYCLE_H


void xTempsCycleParam(const float sIntervallePrint, const float sSeuil1, const float sSeuil2);
void xTempsCycle(bool sSerialPrint);

extern char cyc_CharLCD1[];
extern char cyc_CharLCD2[];
extern bool cyc_newLCD;

#endif