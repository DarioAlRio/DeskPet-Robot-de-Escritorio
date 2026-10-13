// DeskPet · robot de escritorio con ojos animados
//
// Los sensores generan eventos (toque, caricia, golpe, sacudida…) y la máquina
// de emociones decide qué cara poner. Detalles en docs/emociones.md.

#include <Arduino.h>
#include "config.h"
#include "emociones.h"
#include "movimiento.h"
#include "tactil.h"

void setup() {
  Serial.begin(115200);
  iniciarTactil();
  iniciarMovimiento();
  iniciarEmociones();
  Serial.println("DeskPet listo");
}

void loop() {
  uint32_t ahora = millis();

  Evento ev = leerTactil(ahora);
  if (ev != NINGUNO) atenderEvento(ev, ahora);
  ev = leerMovimiento(ahora);
  if (ev != NINGUNO) atenderEvento(ev, ahora);

  actualizarEmociones(ahora);
}
