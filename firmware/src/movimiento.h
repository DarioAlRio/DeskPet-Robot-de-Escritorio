// MPU6050: golpes, sacudidas, levantado y postura
#pragma once
#include <Arduino.h>
#include "eventos.h"

enum Postura { DE_PIE, INCLINADO_IZQ, INCLINADO_DER, TUMBADO, BOCA_ABAJO };

void iniciarMovimiento();
Evento leerMovimiento(uint32_t ahora);
Postura postura();
