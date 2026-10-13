// Sensor táctil TTP223: toque, caricia y toques seguidos
#pragma once
#include <Arduino.h>
#include "eventos.h"

void iniciarTactil();
Evento leerTactil(uint32_t ahora);
