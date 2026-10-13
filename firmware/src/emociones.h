// Máquina de emociones y ojos. Reglas en docs/emociones.md.
#pragma once
#include <Arduino.h>
#include "eventos.h"

enum Emocion {
  NORMAL, CONTENTO, BURLON, SORPRENDIDO, IRRITADO, ENFADADO, FURIOSO, MAREADO,
  ASUSTADO, PROTESTA, CURIOSO, TUMBADO_E, HACKER, ABURRIDO, DORMIDO, DESPERTANDO,
  NUM_EMOCIONES
};

void iniciarEmociones();
void atenderEvento(Evento ev, uint32_t ahora);
void actualizarEmociones(uint32_t ahora);
