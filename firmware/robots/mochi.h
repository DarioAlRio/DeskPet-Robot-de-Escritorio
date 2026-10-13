// Mochi: ojos de pastilla azules al estilo Dasai Mochi.
#pragma once
#include "cara.h"
#include "emociones.h"

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

const uint16_t COLOR_CARA = RGB(110, 200, 255);  // azul clarito
const int SEPARACION = 40;               // del centro a cada ojo
const int OJOS_Y = 68, OJO_ANCHO = 34, OJO_ALTO = 50;
const int OJO_REDONDEZ = 99;            // 99 = pastilla del todo
const int BOCA_Y = 120;
// Una cara por emoción, en el orden de enum Emocion (emociones.h)
const Expresion CARAS[] = {
  {OJOS_PASTILLA,     BOCA_SONRISA,   PARPADEA | MIRA},  // Normal
  {OJOS_FELICES,      BOCA_GRANDE,    SALTA},  // Contento
  {OJOS_GUINO,        BOCA_LENGUA,    NADA},  // Burlón
  {OJOS_REDONDOS,     BOCA_O,         NADA},  // Sorprendido
  {OJOS_MOLESTOS,     BOCA_RECTA,     PARPADEA},  // Irritado
  {OJOS_ENFADADOS,    BOCA_TRISTE,    PARPADEA},  // Enfadado
  {OJOS_ENFADADOS,    BOCA_DIENTES,   TIEMBLA},  // Furioso
  {OJOS_ESPIRAL,      BOCA_ONDULADA,  NADA},  // Mareado
  {OJOS_PEQUENOS,     BOCA_O_PEQUENA, TIEMBLA | SUDOR | PARPADEA},  // Asustado
  {OJOS_APRETADOS,    BOCA_ONDULADA,  TIEMBLA},  // Protesta
  {OJOS_CURIOSOS,     BOCA_PEQUENA,   PARPADEA},  // Curioso
  {OJOS_SOMNOLIENTOS, BOCA_RECTA,     PARPADEA, 0, 8},  // Tumbado
  {OJOS_X,            BOCA_RECTA,     INTERFERENCIA},  // Modo hacker
  {OJOS_SOMNOLIENTOS, BOCA_RECTA,     PARPADEA, 0, 10},  // Aburrido
  {OJOS_CERRADOS,     BOCA_PEQUENA,   ZZZ},  // Dormido
  {OJOS_SOMNOLIENTOS, BOCA_BOSTEZO,   NADA},  // Despertando
};
