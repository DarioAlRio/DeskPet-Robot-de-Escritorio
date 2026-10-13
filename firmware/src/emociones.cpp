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
const uint16_t AZUL_CLARO = RGB(110, 200, 255);  // color de la cara
const uint16_t GRIS_BORDE = RGB(45, 45, 45);

#define OJOS_ANCHO 180  // zona donde se mueven los ojos
#define OJOS_ALTO  140
#define CARA_ALTO  170  // ojos + boca debajo; todo cabe dentro del círculo

static void dibujarBoca(GFXcanvas1 &lienzo);

// ---------------------------------------------------------------------------
// Pantalla: RoboEyes dibuja los ojos en un lienzo de 1 bit (como en una OLED),
// aquí se añade la boca y se pinta todo en azul claro, centrado dentro del círculo.
// ---------------------------------------------------------------------------

class PantallaRedonda : public GFXcanvas1 {
 public:
  uint16_t color = AZUL_CLARO;

  PantallaRedonda() : GFXcanvas1(OJOS_ANCHO, CARA_ALTO) {}

  void clearDisplay() { fillScreen(0); }

  // Envía solo la zona de la cara y solo si ha cambiado desde la última vez
  void display() {
    dibujarBoca(*this);
    static uint8_t anterior[(OJOS_ANCHO + 7) / 8 * CARA_ALTO];
    static uint16_t anteriorColor = 0;
    if (anteriorColor == color && memcmp(anterior, getBuffer(), sizeof(anterior)) == 0) return;
    memcpy(anterior, getBuffer(), sizeof(anterior));
    anteriorColor = color;

    static uint16_t linea[OJOS_ANCHO];
    tft.startWrite();
    tft.setAddrWindow((LADO - OJOS_ANCHO) / 2, OFFSET_Y + (LADO - CARA_ALTO) / 2, OJOS_ANCHO, CARA_ALTO);
    for (int y = 0; y < CARA_ALTO; y++) {
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
// Boca: se dibuja debajo de los ojos y los sigue cuando miran a los lados
// ---------------------------------------------------------------------------

enum Boca {
  SONRISA_DIENTES, SONRISA_GRANDE, TORCIDA, BOCA_O, PEQUENA_O, FRUNCIDA,
  FRUNCIDA_DIENTES, ONDULADA, APRETADA, RECTA, PEQUENA, BOSTEZO, GLITCH
};

static Boca bocaActual = SONRISA_DIENTES;
static uint32_t bocaDesde = 0;

static void lineaGruesa(GFXcanvas1 &c, int x0, int y0, int x1, int y1) {
  for (int d = -1; d <= 1; d++) {
    c.drawLine(x0, y0 + d, x1, y1 + d, 1);
    c.drawLine(x0 + d, y0, x1 + d, y1, 1);
  }
}

// Curva tipo sonrisa. flecha > 0: el centro baja (‿); < 0: sube (∩).
// incl inclina la boca; dientes > 0 la convierte en zigzag de esa altura.
static void curva(GFXcanvas1 &c, int cx, int cy, int ancho, float flecha, float incl, int segmentos,
                  float dientes) {
  int px = 0, py = 0;
  for (int i = 0; i <= segmentos; i++) {
    float u = -1 + 2.0f * i / segmentos;
    int x = cx + u * ancho / 2;
    int y = cy + flecha * (1 - u * u) + incl * u * ancho / 2 + ((i % 2) ? dientes / 2 : -dientes / 2);
    if (i > 0) lineaGruesa(c, px, py, x, y);
    px = x;
    py = y;
  }
}

static void anillo(GFXcanvas1 &c, int cx, int cy, int r) {
  for (int g = 0; g < 3; g++) c.drawCircle(cx, cy, r - g, 1);
}

static void dibujarBoca(GFXcanvas1 &c) {
  // Sigue a los ojos: centrada entre los dos y un poco por debajo
  int cx = (ojos.eyeLx + ojos.eyeRx + ojos.eyeRwidthCurrent) / 2;
  int centroOjos = ojos.eyeLy + ojos.eyeLheightCurrent / 2;
  int cy = constrain(150 + (centroOjos - OJOS_ALTO / 2) / 2, 145, 158);
  uint32_t t = millis() - bocaDesde;

  switch (bocaActual) {
    case SONRISA_DIENTES:  curva(c, cx, cy, 60, 6, 0, 8, 8); break;
    case SONRISA_GRANDE:   curva(c, cx, cy - 2, 80, 10, 0, 10, 10); break;
    case TORCIDA:          curva(c, cx + 6, cy, 56, 5, -0.15f, 8, 7); break;
    case BOCA_O:           anillo(c, cx, cy + 2, 10); break;
    case PEQUENA_O:        anillo(c, cx + random(-1, 2), cy + 2, 6); break;
    case FRUNCIDA:         curva(c, cx, cy + 6, 50, -8, 0, 10, 0); break;
    case FRUNCIDA_DIENTES: curva(c, cx + random(-2, 3), cy + 6, 60, -8, 0, 8, 8); break;
    case RECTA:            lineaGruesa(c, cx - 20, cy + 2, cx + 20, cy + 2); break;
    case PEQUENA:          curva(c, cx, cy, 20, 4, 0, 6, 0); break;
    case GLITCH:           curva(c, cx + random(-4, 5), cy, 64, 6, 0, 10, 8); break;
    case ONDULADA: {
      int px = 0, py = 0;
      for (int i = 0; i <= 12; i++) {
        int x = cx - 30 + 5 * i;
        int y = cy + 2 + sinf(i * 1.1f + t * 0.01f) * 5;
        if (i > 0) lineaGruesa(c, px, py, x, y);
        px = x;
        py = y;
      }
      break;
    }
    case APRETADA: {
      int x0 = cx - 28, x1 = cx + 28, y0 = cy - 6, y1 = cy + 10;
      c.drawRect(x0, y0, x1 - x0, y1 - y0, 1);
      c.drawRect(x0 + 1, y0 + 1, x1 - x0 - 2, y1 - y0 - 2, 1);
      c.drawFastHLine(x0, (y0 + y1) / 2, x1 - x0, 1);
      for (int x = x0 + 8; x < x1; x += 8) c.drawFastVLine(x, y0, y1 - y0, 1);
      break;
    }
    case BOSTEZO: {
      // Abre la boca poco a poco y la vuelve a cerrar
      float p = t < 1600 ? sinf(t / 1600.0f * PI) : 0;
      if (p > 0.1f) anillo(c, cx, cy + 2, 4 + 10 * p);
      else curva(c, cx, cy, 20, 4, 0, 6, 0);
      break;
    }
  }
}

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
  Boca boca;
  uint8_t prioridad;  // si coinciden varias, gana la más alta
  uint16_t duracion;  // ms; 0 = dura mientras se cumpla su condición
  void (*ver)();
};

static const DatosEmocion EMOCIONES[] = {
  {"Normal",      SONRISA_DIENTES,   0, 0,    verNormal},
  {"Contento",    SONRISA_GRANDE,    2, 3000, verContento},
  {"Burlón",      TORCIDA,           2, 2000, verBurlon},
  {"Sorprendido", BOCA_O,            2, 1500, verSorprendido},
  {"Irritado",    FRUNCIDA,          2, 3000, verIrritado},
  {"Enfadado",    FRUNCIDA_DIENTES,  4, 3000, verEnfadado},
  {"Furioso",     FRUNCIDA_DIENTES,  4, 4000, verFurioso},
  {"Mareado",     ONDULADA,          4, 3000, verMareado},
  {"Asustado",    PEQUENA_O,         5, 3000, verAsustado},
  {"Protesta",    APRETADA,          6, 0,    verProtesta},
  {"Curioso",     PEQUENA,           3, 0,    verCurioso},
  {"Tumbado",     RECTA,             3, 0,    verTumbado},
  {"Modo hacker", GLITCH,            2, 5000, verHacker},
  {"Aburrido",    RECTA,             1, 0,    verAburrido},
  {"Dormido",     PEQUENA,           1, 0,    verDormido},
  {"Despertando", BOSTEZO,           2, 2000, verDespertando},
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
  bocaActual = EMOCIONES[e].boca;
  bocaDesde = millis();
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
