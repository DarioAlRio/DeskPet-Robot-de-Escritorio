// Máquina de emociones y ojos. Reglas en docs/emociones.md.
#pragma once
#include <Arduino.h>
#include "eventos.h"

void iniciarEmociones();
void atenderEvento(Evento ev, uint32_t ahora);
void actualizarEmociones(uint32_t ahora);
