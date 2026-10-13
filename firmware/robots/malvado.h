// Malvado: la cara roja y traviesa de los primeros renders.
#pragma once
#include "cara.h"
#include "emociones.h"

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

const uint16_t COLOR_CARA = RGB(255, 30, 40);  // rojo
const int SEPARACION = 44;
const int OJOS_Y = 70, OJO_ANCHO = 48, OJO_ALTO = 34;
const int OJO_REDONDEZ = 8;            // ojos rasgados, casi rectos
const int BOCA_Y = 122;
// Una cara por emoción, en el orden de enum Emocion (emociones.h)
const Expresion CARAS[] = {
  {OJOS_MOLESTOS,     BOCA_DIENTES,   PARPADEA | MIRA},  // Normal
  {OJOS_FELICES,      BOCA_DIENTES,   SALTA},  // Contento
  {OJOS_GUINO,        BOCA_LENGUA,    NADA},  // Burlón
  {OJOS_REDONDOS,     BOCA_O,         TIEMBLA},  // Sorprendido
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
