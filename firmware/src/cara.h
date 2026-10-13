// Cara al estilo Dasai Mochi: dos ojos tipo pastilla y una boca pequeña.
// Cada expresión combina una forma de ojos, una de boca y algunos efectos.
#pragma once
#include <Arduino.h>

enum Ojos {
  OJOS_PASTILLA,     // normales, parpadean y miran
  OJOS_FELICES,      // ^ ^
  OJOS_GUINO,        // uno normal y otro ^
  OJOS_REDONDOS,     // muy abiertos
  OJOS_MOLESTOS,     // pastilla con el párpado inclinado un poco
  OJOS_ENFADADOS,    // pastilla con el párpado muy inclinado
  OJOS_ESPIRAL,      // mareado
  OJOS_PEQUENOS,     // asustado
  OJOS_APRETADOS,    // > <
  OJOS_CURIOSOS,     // uno más grande que el otro
  OJOS_SOMNOLIENTOS, // medio cerrados
  OJOS_CERRADOS,     // ‿ ‿ durmiendo
  OJOS_CORAZON,      // enamorado
  OJOS_X             // x x
};

enum Boca {
  BOCA_SONRISA, BOCA_GRANDE, BOCA_LENGUA, BOCA_O, BOCA_O_PEQUENA, BOCA_RECTA,
  BOCA_TRISTE, BOCA_DIENTES, BOCA_ONDULADA, BOCA_PEQUENA, BOCA_BOSTEZO
};

enum Efecto : uint8_t {
  NADA       = 0,
  PARPADEA   = 1 << 0,  // parpadeo automático
  MIRA       = 1 << 1,  // mira a sitios al azar
  SALTA      = 1 << 2,  // da botecitos (risa)
  TIEMBLA    = 1 << 3,  // vibra (enfado, miedo)
  SUDOR      = 1 << 4,  // gota de sudor
  ZZZ        = 1 << 5,  // zetas flotando
  INTERFERENCIA = 1 << 6  // glitch (modo hacker)
};

struct Expresion {
  Ojos ojos;
  Boca boca;
  uint8_t efectos;
  int8_t mirarX, mirarY;  // hacia dónde mira fijo (si no tiene MIRA)
};

void iniciarCara();
void ponerCara(const Expresion &e);
void actualizarCara(uint32_t ahora);
