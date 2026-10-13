#include "emociones.h"
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"
#include "movimiento.h"

#ifdef PANTALLA_WOKWI
#include <Adafruit_ILI9341.h>
static Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);
#else
#include <Adafruit_GC9A01A.h>
static Adafruit_GC9A01A tft(PIN_CS, PIN_DC, PIN_RST);
#endif

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
const uint16_t NEGRO      = 0x0000;
const uint16_t ROJO       = RGB(255, 30, 40);
const uint16_t GRIS_BORDE = RGB(45, 45, 45);

#define OJOS_ANCHO 180  // zona donde se mueven los ojos: cabe entera dentro del círculo
#define OJOS_ALTO  140

// ---------------------------------------------------------------------------
// Pantalla: RoboEyes dibuja en un lienzo de 1 bit (como en una OLED) y aquí se
// pinta en rojo, centrado dentro del círculo.
// ---------------------------------------------------------------------------

class PantallaRedonda : public GFXcanvas1 {
 public:
  uint16_t color = ROJO;

  PantallaRedonda() : GFXcanvas1(OJOS_ANCHO, OJOS_ALTO) {}

  void clearDisplay() { fillScreen(0); }

  // Envía solo la zona de los ojos y solo si ha cambiado desde la última vez
  void display() {
    static uint8_t anterior[(OJOS_ANCHO + 7) / 8 * OJOS_ALTO];
    static uint16_t anteriorColor = 0;
    if (anteriorColor == color && memcmp(anterior, getBuffer(), sizeof(anterior)) == 0) return;
    memcpy(anterior, getBuffer(), sizeof(anterior));
    anteriorColor = color;

    static uint16_t linea[OJOS_ANCHO];
    tft.startWrite();
    tft.setAddrWindow((LADO - OJOS_ANCHO) / 2, OFFSET_Y + (LADO - OJOS_ALTO) / 2, OJOS_ANCHO, OJOS_ALTO);
    for (int y = 0; y < OJOS_ALTO; y++) {
      for (int x = 0; x < OJOS_ANCHO; x++) linea[x] = getPixel(x, y) ? color : NEGRO;
      tft.writePixels(linea, OJOS_ANCHO);
    }
    tft.endWrite();
  }
};

static PantallaRedonda pantalla;

// Va después del resto de includes: define macros cortas (N, S, E, W, ON, OFF…)
#include <FluxGarage_RoboEyes.h>
static RoboEyes<PantallaRedonda> ojos(pantalla);

// ---------------------------------------------------------------------------
// Cómo se ve cada emoción con RoboEyes
// ---------------------------------------------------------------------------

enum Emocion {
  NORMAL, CONTENTO, BURLON, SORPRENDIDO, IRRITADO, ENFADADO, FURIOSO, MAREADO,
  ASUSTADO, PROTESTA, CURIOSO, TUMBADO_E, HACKER, ABURRIDO, DORMIDO, DESPERTANDO
};

static void reiniciarOjos() {
  ojos.setWidth(64, 64);
  ojos.setHeight(64, 64);
  ojos.setBorderradius(16, 16);
  ojos.setSpacebetween(20);
  ojos.setMood(DEFAULT);
  ojos.setPosition(DEFAULT);
  ojos.setCuriosity(OFF);
  ojos.setCyclops(OFF);
  ojos.setSweat(OFF);
  ojos.setHFlicker(OFF);
  ojos.setVFlicker(OFF);
  ojos.setAutoblinker(ON, 3, 2);
  ojos.setIdleMode(ON, 2, 2);
  ojos.open();
}

static void quieto() {
  ojos.setIdleMode(OFF);
  ojos.setPosition(DEFAULT);
}

static void verNormal() {}
static void verContento() { ojos.setMood(HAPPY); ojos.anim_laugh(); }
static void verBurlon() { ojos.setMood(HAPPY); ojos.blink(true, false); }  // guiño
static void verSorprendido() { quieto(); ojos.setWidth(56, 56); ojos.setHeight(80, 80); ojos.setBorderradius(28, 28); }
static void verIrritado() { ojos.setMood(ANGRY); ojos.setHeight(48, 48); }
static void verEnfadado() { quieto(); ojos.setMood(ANGRY); }
static void verFurioso() { quieto(); ojos.setMood(ANGRY); ojos.setHFlicker(ON, 2); }
static void verMareado() { ojos.anim_confused(); ojos.setHFlicker(ON, 3); }
static void verAsustado() { ojos.setWidth(44, 44); ojos.setHeight(44, 44); ojos.setSweat(ON); ojos.setVFlicker(ON, 2); ojos.setIdleMode(ON, 1, 0); }
static void verProtesta() { quieto(); ojos.setMood(ANGRY); ojos.setHeight(24, 24); ojos.setVFlicker(ON, 1); }
static void verCurioso() {
  ojos.setIdleMode(OFF);
  ojos.setCuriosity(ON);
  ojos.setPosition(postura() == INCLINADO_DER ? E : W);
}
static void verTumbado() { quieto(); ojos.setMood(TIRED); ojos.setPosition(S); }
static void verHacker() { ojos.setCyclops(ON); ojos.setHFlicker(ON, 4); ojos.setIdleMode(ON, 1, 0); }
static void verAburrido() { quieto(); ojos.setMood(TIRED); ojos.setPosition(S); }
static void verDormido() { quieto(); ojos.setAutoblinker(OFF); ojos.close(); }
static void verDespertando() { quieto(); ojos.setMood(TIRED); }

struct DatosEmocion {
  const char *nombre;
  uint8_t prioridad;  // si coinciden varias, gana la más alta
  uint16_t duracion;  // ms; 0 = dura mientras se cumpla su condición
  void (*ver)();
};

static const DatosEmocion EMOCIONES[] = {
  {"Normal",      0, 0,    verNormal},
  {"Contento",    2, 3000, verContento},
  {"Burlón",      2, 2000, verBurlon},
  {"Sorprendido", 2, 1500, verSorprendido},
  {"Irritado",    2, 3000, verIrritado},
  {"Enfadado",    4, 3000, verEnfadado},
  {"Furioso",     4, 4000, verFurioso},
  {"Mareado",     4, 3000, verMareado},
  {"Asustado",    5, 3000, verAsustado},
  {"Protesta",    6, 0,    verProtesta},
  {"Curioso",     3, 0,    verCurioso},
  {"Tumbado",     3, 0,    verTumbado},
  {"Modo hacker", 2, 5000, verHacker},
  {"Aburrido",    1, 0,    verAburrido},
  {"Dormido",     1, 0,    verDormido},
  {"Despertando", 2, 2000, verDespertando},
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
  reiniciarOjos();
  EMOCIONES[e].ver();
  Serial.printf("Emoción: %s\n", EMOCIONES[e].nombre);
}

void iniciarEmociones() {
  tft.begin(40000000);
  tft.fillScreen(NEGRO);
  tft.drawCircle(LADO / 2, OFFSET_Y + LADO / 2, LADO / 2 - 1, GRIS_BORDE);  // borde de la pantalla redonda
  ojos.begin(OJOS_ANCHO, OJOS_ALTO, 30);
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

  ojos.update();
}
