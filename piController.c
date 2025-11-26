/*
 * pi_controller.c
 */

#include "pi_controller.h"
#include "math.h" // kirjasto itseisarvon laskentaaan

#define U1_MAX 3.3f        // Määritellään integraattorin maksimiarvo 3.3 volttiin
static float u1_old = 0.0f; // määritetään muuttuja integraattorin nollaukseen

// alustusfunktio
void PI_Init(void){
	u1_old = 0.0f; // Nollataan säädin
}

// Säädinlogiikka luenttolta 7. Sivuilta 11-12
float PI(float y_ref, float y_act, float Ki, float Kp){
	float error_new; // Muuttuja uudelle virheelle (säädin ei tule 100% varmuudella pääsemään tavoiteltuun referenssijännitteeseen)
	float u1_new; // Muuttuja uudelle integraattorin arvolle
	float u_new; //Muuttuja uudelle säätimen lähtöarvolle

	// Lasketaan ensin virheen suuruus referenssijännitteen ja tämän hetkisen mitatun arvon välilä
	error_new = y_ref - y_act;

	// Lasktaan integraaliosuus. Luento 7 sivu 11 ja 12
	u1_new = u1_old + Ki * error_new;

	// Integraattrion saturointi (estetään integraattorin karkaaminen) Luento 7 sivu 11 ja 12
	if(fabsf(u1_new) >= U1_MAX){
		u1_new = u1_old; // Pidetään vanha arvo, ei kasvateta integraattorin arvoa
	}

	// lasketaan säätimen lähtöarvo (P-osa + I-osa)
	u_new = Kp * error_new + u1_new;

	u1_old = u1_new; // Tallennetaan uusi arvo integraattorin vanhaksi arvoksi
	return u_new;
}


