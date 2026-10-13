#include "emociones.h"
#include "cara.h"
#include "config.h"
#include "movimiento.h"

#include ROBOT  // la cara de este robot (robots/*.h), elegida en platformio.ini
#ifndef MODO_THEMOCHI  // los robots de animaciones no usan emociones

// ---------------------------------------------------------------------------
// Datos de cada emoción. Cómo se ve cada una lo decide el robot (CARAS[]).
// ---------------------------------------------------------------------------

struct DatosEmocion {
  const char *nombre;
  uint8_t prioridad;  // si coinciden varias, gana la más alta
  uint16_t duracion;  // ms; 0 = dura mientras se cumpla su condición
};

static const DatosEmocion EMOCIONES[] = {
  {"Normal",      0, 0},
  {"Contento",    2, 3000},
  {"Burlón",      2, 2000},
  {"Sorprendido", 2, 1500},
  {"Irritado",    2, 3000},
  {"Enfadado",    4, 3000},
  {"Furioso",     4, 4000},
  {"Mareado",     4, 3000},
  {"Asustado",    5, 3000},
  {"Protesta",    6, 0},
  {"Curioso",     3, 0},
  {"Tumbado",     3, 0},
  {"Modo hacker", 2, 5000},
  {"Aburrido",    1, 0},
  {"Dormido",     1, 0},
  {"Despertando", 2, 2000},
};
static_assert(sizeof(EMOCIONES) / sizeof(EMOCIONES[0]) == NUM_EMOCIONES, "falta alguna emoción");
static_assert(sizeof(CARAS) / sizeof(CARAS[0]) == NUM_EMOCIONES, "al robot le falta alguna cara");


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
  Expresion cara = CARAS[e];
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
#ifdef MODO_DEMO
  // Pasa por todas las caras, una cada 3 s, sin hacer caso a los sensores
  Emocion demo = (Emocion)((ahora / 3000) % NUM_EMOCIONES);
  if (demo != mostrada) mostrar(demo);
  actualizarCara(ahora);
  return;
#endif
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

#endif
