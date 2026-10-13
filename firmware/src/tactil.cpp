#include "tactil.h"
#include "config.h"

const uint32_t REBOTE_MS = 30, CARICIA_MS = 600, ENTRE_TOQUES_MS = 400;

static bool tocando = false, caricia = false;
static uint32_t ultimoCambio = 0, ultimoSoltar = 0;
static int toques = 0;

void iniciarTactil() {
  pinMode(PIN_TOQUE, INPUT_PULLDOWN);  // el TTP223 da HIGH mientras se toca
}

Evento leerTactil(uint32_t ahora) {
  bool ahoraTocando = digitalRead(PIN_TOQUE);
  if (ahoraTocando != tocando && ahora - ultimoCambio > REBOTE_MS) {
    tocando = ahoraTocando;
    ultimoCambio = ahora;
    if (!tocando) {
      if (caricia) {
        caricia = false;
      } else {
        toques++;
        ultimoSoltar = ahora;
      }
    }
  }

  // Mantener el dedo = caricia (salta sin esperar a soltar)
  if (tocando && !caricia && ahora - ultimoCambio >= CARICIA_MS) {
    caricia = true;
    toques = 0;
    return CARICIA;
  }

  // Toques cortos: se cuentan hasta que pasa un rato sin otro toque
  if (!tocando && toques > 0 && ahora - ultimoSoltar > ENTRE_TOQUES_MS) {
    int n = toques;
    toques = 0;
    if (n == 1) return TOQUE;
    if (n == 2) return DOBLE_TOQUE;
    if (n < 5) return TRIPLE_TOQUE;
    return MUCHOS_TOQUES;
  }
  return NINGUNO;
}
