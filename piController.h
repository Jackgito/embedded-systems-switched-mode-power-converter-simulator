/*
 * pi_controller.h
 *
 *  Created on: Nov 26, 2025
 *      Author: saska
 */

#ifndef PI_CONTROLLER_H_
#define PI_CONTROLLER_H_

// Alustusfunktio (nollaa säätimen muistin arvot)
void PI_Init(void); // Aina kun siirrytään modulointiin, tyhjennetään vanhat arvot

// PI-säätimen laskentafunktio luentokalvon 12 muuttujanimillä
// Parametrit: tavoite (y_ref), mittaus (y_act), integraalivahvistus (Ki), proportionaalivahvistus (Kp)
float PI(float y_ref, float y_act, float Ki, float Kp); // Funktion prototyyppi: palauttaa float-arvon (ohjaussignaali)

#endif /* PI_CONTROLLER_H_ */
