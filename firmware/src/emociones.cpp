#include "emociones.h"
#include "cara.h"
#include "config.h"
#include "movimiento.h"

// ---------------------------------------------------------------------------
// Cómo se ve cada emoción (ojos, boca y efectos al estilo Dasai Mochi)
// ---------------------------------------------------------------------------

enum Emocion {
  NORMAL, CONTENTO, BURLON, SORPRENDIDO, IRRITADO, ENFADADO, FURIOSO, MAREADO,
  ASUSTADO, PROTESTA, CURIOSO, TUMBADO_E, HACKER, ABURRIDO, DORMIDO, DESPERTANDO
};

struct DatosEmocion {
  const char *nombre;
  Expresion cara;
  uint8_t prioridad;  // si coinciden varias, gana la más alta
  uint16_t duracion;  // ms; 0 = dura mientras se cumpla su condición
};

static const DatosEmocion EMOCIONES[] = {
  {"Normal",      {OJOS_PASTILLA,     BOCA_SONRISA,   PARPADEA | MIRA},           0, 0},
  {"Contento",    {OJOS_FELICES,      BOCA_GRANDE,    SALTA},                     2, 3000},
  {"Burlón",      {OJOS_GUINO,        BOCA_LENGUA,    NADA},                      2, 2000},
  {"Sorprendido", {OJOS_REDONDOS,     BOCA_O,         NADA},                      2, 1500},
  {"Irritado",    {OJOS_MOLESTOS,     BOCA_RECTA,     PARPADEA},                  2, 3000},
  {"Enfadado",    {OJOS_ENFADADOS,    BOCA_TRISTE,    PARPADEA},                  4, 3000},
  {"Furioso",     {OJOS_ENFADADOS,    BOCA_DIENTES,   TIEMBLA},                   4, 4000},
  {"Mareado",     {OJOS_ESPIRAL,      BOCA_ONDULADA,  NADA},                      4, 3000},
  {"Asustado",    {OJOS_PEQUENOS,     BOCA_O_PEQUENA, TIEMBLA | SUDOR | PARPADEA}, 5, 3000},
  {"Protesta",    {OJOS_APRETADOS,    BOCA_ONDULADA,  TIEMBLA},                   6, 0},
  {"Curioso",     {OJOS_CURIOSOS,     BOCA_PEQUENA,   PARPADEA},                  3, 0},
  {"Tumbado",     {OJOS_SOMNOLIENTOS, BOCA_RECTA,     PARPADEA, 0, 8},            3, 0},
  {"Modo hacker", {OJOS_X,            BOCA_RECTA,     INTERFERENCIA},             2, 5000},
  {"Aburrido",    {OJOS_SOMNOLIENTOS, BOCA_RECTA,     PARPADEA, 0, 10},           1, 0},
  {"Dormido",     {OJOS_CERRADOS,     BOCA_PEQUENA,   ZZZ},                       1, 0},
  {"Despertando", {OJOS_SOMNOLIENTOS, BOCA_BOSTEZO,   NADA},                      2, 2000},
};


// ---------------------------------------------------------------------------
// Máquina de estados
// ---------------------------------------------------------------------------

static Emocion mostrada = NORMAL;
static Emocion momentanea = NORMAL;  // la que ha provocado el último evento
static uint32_t momentaneaHasta = 0, momentaneaAplicada = 0, ultimaInteraccion = 0;
static Postura posturaAnterior = DE_PIE;

static void lanzar(Emocion e, uint32_t ahora) {
  bool activa = ahora < momentaneaHasta;
  if (activa && EMOCIONES[e].prioridad < EMOCIONES[momentanea].prioridad) return;
  momentanea = e;
  momentaneaHasta = ahora + EMOCIONES[e].duracion;
}

static Emocion elegir(uint32_t ahora) {
  Emocion candidatas[3];
  int n = 0;

  // Por la postura (dura mientras siga así)
  Postura p = postura();
  if (p == BOCA_ABAJO) candidatas[n++] = PROTESTA;
  else if (p == TUMBADO) candidatas[n++] = TUMBADO_E;
  else if (p != DE_PIE) candidatas[n++] = CURIOSO;

  // Por el último evento, mientras dure
  if (ahora < momentaneaHasta) candidatas[n++] = momentanea;

  // Por el tiempo sin hacerle caso
  uint32_t sinCaso = ahora - ultimaInteraccion;
  candidatas[n++] = sinCaso > T_DORMIDO ? DORMIDO : sinCaso > T_ABURRIDO ? ABURRIDO : NORMAL;

  Emocion mejor = candidatas[0];
  for (int i = 1; i < n; i++)
    if (EMOCIONES[candidatas[i]].prioridad > EMOCIONES[mejor].prioridad) mejor = candidatas[i];
  return mejor;
}

static void mostrar(Emocion e) {
  mostrada = e;
  Expresion cara = EMOCIONES[e].cara;
  if (e == CURIOSO) cara.mirarX = postura() == INCLINADO_DER ? 16 : -16;
  ponerCara(cara);
  Serial.printf("Emoción: %s\n", EMOCIONES[e].nombre);
}

void iniciarEmociones() {
  iniciarCara();
  mostrar(NORMAL);
}

void atenderEvento(Evento ev, uint32_t ahora) {
  Serial.printf("Evento: %s\n", nombreEvento(ev));
  bool dormido = mostrada == DORMIDO;
  bool yaEnfadado = mostrada == ENFADADO || mostrada == FURIOSO;
  ultimaInteraccion = ahora;

  switch (ev) {
    case TOQUE:
    case GOLPE_SUAVE:   lanzar(dormido ? DESPERTANDO : SORPRENDIDO, ahora); break;
    case CARICIA:       lanzar(dormido ? DESPERTANDO : CONTENTO, ahora); break;
    case DOBLE_TOQUE:   lanzar(BURLON, ahora); break;
    case TRIPLE_TOQUE:  lanzar(IRRITADO, ahora); break;
    case MUCHOS_TOQUES: lanzar(HACKER, ahora); break;
    case GOLPE_FUERTE:  lanzar(yaEnfadado ? FURIOSO : ENFADADO, ahora); break;
    case SACUDIDA:      lanzar(MAREADO, ahora); break;
    case LEVANTADO:     lanzar(ASUSTADO, ahora); break;
    default: break;
  }
}

void actualizarEmociones(uint32_t ahora) {
  // Cambiar de postura también cuenta como hacerle caso
  if (postura() != posturaAnterior) {
    posturaAnterior = postura();
    ultimaInteraccion = ahora;
  }

  Emocion e = elegir(ahora);
  // Se vuelve a aplicar si un evento nuevo la ha relanzado (p. ej. otra risa)
  if (e != mostrada || (e == momentanea && momentaneaHasta != momentaneaAplicada)) {
    mostrar(e);
    if (e == momentanea) momentaneaAplicada = momentaneaHasta;
  }

  actualizarCara(ahora);
}
