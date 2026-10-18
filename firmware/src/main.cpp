// DeskPet · robot de escritorio con ojos animados
//
// Los sensores generan eventos (toque, caricia, golpe, sacudida…) y la máquina
// de emociones decide qué cara poner. Detalles en docs/emociones.md.

#include <Arduino.h>
#include "config.h"
#include "emociones.h"
#include "movimiento.h"
#include "tactil.h"
#include ROBOT

#ifdef MODO_THEMOCHI  // robots de animaciones: sustituyen a las emociones
#include "reproductor.h"
#define iniciarEmociones iniciarReproductor
#define atenderEvento atenderEventoReproductor
#define actualizarEmociones actualizarReproductor
#endif

void setup() {
  Serial.begin(115200);
  // Sin esto random() da la misma secuencia en cada arranque
  randomSeed(esp_random());
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
