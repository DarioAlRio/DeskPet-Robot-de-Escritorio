// Robots de animaciones (como The Mochi): ponen animaciones según los toques
// en lugar de usar la máquina de emociones.
#pragma once
#include <Arduino.h>
#include "eventos.h"

void iniciarReproductor();
void atenderEventoReproductor(Evento ev, uint32_t ahora);
void actualizarReproductor(uint32_t ahora);
