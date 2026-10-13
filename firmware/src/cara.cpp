#include "cara.h"
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"
#include ROBOT

#ifdef PANTALLA_WOKWI
#include <Adafruit_ILI9341.h>
static Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);
#else
#include <Adafruit_GC9A01A.h>
static Adafruit_GC9A01A tft(PIN_CS, PIN_DC, PIN_RST);
#endif

#define RGB_(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
const uint16_t NEGRO      = 0x0000;
const uint16_t GRIS_BORDE = RGB_(45, 45, 45);

// La cara se dibuja en un lienzo de 1 bit (como en una OLED) y se pinta en
// COLOR_CARA centrada dentro del círculo. 180×170 cabe entero en la pantalla.
#define ANCHO 180
#define ALTO  170
static GFXcanvas1 lienzo(ANCHO, ALTO);

// Las medidas de los ojos y la boca vienen del robot (robots/*.h)
const int CENTRO_X = ANCHO / 2;

// ---------------------------------------------------------------------------
// Estado
// ---------------------------------------------------------------------------

static Expresion actual = {OJOS_PASTILLA, BOCA_SONRISA, PARPADEA | MIRA};
static uint32_t inicio = 0;               // cuándo empezó la expresión actual
static float apertura = 1;                // 1 = ojos abiertos; baja a 0 al parpadear
static float miradaX = 0, miradaY = 0, objetivoX = 0, objetivoY = 0;
static uint32_t proximoParpadeo = 0, proximaMirada = 0, ultimoFotograma = 0;
static float despX = 0, despY = 0;        // desplazamiento de lo que se dibuja ahora

// ---------------------------------------------------------------------------
// Dibujo básico: trazos de punta redonda
// ---------------------------------------------------------------------------

static void punto(float x, float y, int r) {
  lienzo.fillCircle(lroundf(x + despX), lroundf(y + despY), r, 1);
}

static void segmento(float x0, float y0, float x1, float y1, int r) {
  float d = hypotf(x1 - x0, y1 - y0);
  int pasos = max(1, (int)(d / 1.5f));
  for (int i = 0; i <= pasos; i++) {
    float t = (float)i / pasos;
    punto(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, r);
  }
}

// Curva tipo sonrisa. flecha > 0: el centro baja (‿); < 0: sube (∩ o ^).
static void curva(float cx, float cy, float ancho, float flecha, int r) {
  float px = 0, py = 0;
  for (int i = 0; i <= 12; i++) {
    float u = -1 + i / 6.0f;
    float x = cx + u * ancho / 2, y = cy + flecha * (1 - u * u);
    if (i > 0) segmento(px, py, x, y, r);
    px = x;
    py = y;
  }
}

static void pastilla(float cx, float cy, int w, int h) {
  h = max(4, h);
  int x = lroundf(cx - w / 2.0f + despX), y = lroundf(cy - h / 2.0f + despY);
  lienzo.fillRoundRect(x, y, w, h, min(min(w, h) / 2, OJO_REDONDEZ), 1);
}

static void borrarTriangulo(float x0, float y0, float x1, float y1, float x2, float y2) {
  lienzo.fillTriangle(x0 + despX, y0 + despY, x1 + despX, y1 + despY, x2 + despX, y2 + despY, 0);
}

static void anillo(float cx, float cy, int r, int grosor) {
  for (int g = 0; g < grosor; g++) lienzo.drawCircle(lroundf(cx + despX), lroundf(cy + despY), r - g, 1);
}

// ---------------------------------------------------------------------------
// Ojos
// ---------------------------------------------------------------------------

// Párpado inclinado: borra un triángulo de arriba, más bajo hacia el centro
static void parpado(float cx, int lado, int h, int hondo) {
  float arriba = OJOS_Y - h / 2.0f - 1;
  float fuera = cx + lado * (OJO_ANCHO / 2.0f + 2), dentro = cx - lado * (OJO_ANCHO / 2.0f + 2);
  borrarTriangulo(fuera, arriba, dentro, arriba, dentro, arriba + hondo);
}

static void corazon(float cx, float cy, float s) {
  punto(cx - 8 * s, cy - 6 * s, 10 * s);
  punto(cx + 8 * s, cy - 6 * s, 10 * s);
  lienzo.fillTriangle(cx - 18 * s + despX, cy - 2 * s + despY, cx + 18 * s + despX, cy - 2 * s + despY,
                      cx + despX, cy + 20 * s + despY, 1);
}

static void espiral(float cx, float cy, float r, float giro) {
  float px = cx, py = cy;
  for (int i = 1; i <= 40; i++) {
    float t = i / 40.0f, a = t * 4 * PI + giro;
    float x = cx + cosf(a) * r * t, y = cy + sinf(a) * r * t;
    segmento(px, py, x, y, 2);
    px = x;
    py = y;
  }
}

static void dibujarOjo(int lado, uint32_t t) {
  float cx = CENTRO_X + lado * SEPARACION;
  int alto = OJO_ALTO * apertura;

  switch (actual.ojos) {
    case OJOS_PASTILLA:
      pastilla(cx, OJOS_Y, OJO_ANCHO, alto);
      break;
    case OJOS_FELICES:
      curva(cx, OJOS_Y + 6, OJO_ANCHO, -14, 4);
      break;
    case OJOS_GUINO:
      if (lado < 0) pastilla(cx, OJOS_Y, OJO_ANCHO, OJO_ALTO);
      else curva(cx, OJOS_Y + 6, OJO_ANCHO, -14, 4);
      break;
    case OJOS_REDONDOS:
      punto(cx, OJOS_Y, min(24.0f, 12 + t / 15.0f));  // se abren de golpe
      break;
    case OJOS_MOLESTOS:
      pastilla(cx, OJOS_Y, OJO_ANCHO, alto);
      parpado(cx, lado, alto, 16);
      break;
    case OJOS_ENFADADOS:
      pastilla(cx, OJOS_Y + 4, OJO_ANCHO, alto);
      parpado(cx, lado, alto - 8, 30);
      break;
    case OJOS_ESPIRAL:
      espiral(cx, OJOS_Y, 20, lado * t * 0.008f);
      break;
    case OJOS_PEQUENOS:
      pastilla(cx, OJOS_Y, 18, 26 * apertura);
      break;
    case OJOS_APRETADOS:  // > <
      segmento(cx - lado * 12, OJOS_Y - 13, cx + lado * 12, OJOS_Y, 4);
      segmento(cx + lado * 12, OJOS_Y, cx - lado * 12, OJOS_Y + 13, 4);
      break;
    case OJOS_CURIOSOS:
      if (lado < 0) pastilla(cx, OJOS_Y + 4, 28, 38 * apertura);
      else pastilla(cx, OJOS_Y - 2, 40, 60 * apertura);
      break;
    case OJOS_SOMNOLIENTOS: {
      pastilla(cx, OJOS_Y, OJO_ANCHO, alto);
      int corte = OJO_ALTO * 0.55f;  // párpado caído
      lienzo.fillRect(cx - OJO_ANCHO / 2 - 1 + despX, OJOS_Y - OJO_ALTO / 2 - 1 + despY, OJO_ANCHO + 2, corte, 0);
      break;
    }
    case OJOS_CERRADOS:
      curva(cx, OJOS_Y + 4, OJO_ANCHO, 8, 4);
      break;
    case OJOS_CORAZON:
      corazon(cx, OJOS_Y - 2, 1 + 0.1f * sinf(t * 0.012f));  // late
      break;
    case OJOS_X:
      segmento(cx - 12, OJOS_Y - 12, cx + 12, OJOS_Y + 12, 4);
      segmento(cx + 12, OJOS_Y - 12, cx - 12, OJOS_Y + 12, 4);
      break;
  }
}

// ---------------------------------------------------------------------------
// Boca
// ---------------------------------------------------------------------------

static void dibujarBoca(uint32_t t) {
  float cx = CENTRO_X, cy = BOCA_Y;

  switch (actual.boca) {
    case BOCA_SONRISA:
      curva(cx, cy - 3, 26, 7, 3);
      break;
    case BOCA_GRANDE:  // media luna rellena
      punto(cx, cy - 6, 15);
      lienzo.fillRect(cx - 16 + despX, cy - 22 + despY, 33, 16, 0);
      break;
    case BOCA_LENGUA:
      curva(cx, cy - 3, 26, 7, 3);
      punto(cx + 6, cy + 6, 5);
      break;
    case BOCA_O:
      anillo(cx, cy + 2, 10, 4);
      break;
    case BOCA_O_PEQUENA:
      anillo(cx, cy + 2, 6, 3);
      break;
    case BOCA_RECTA:
      segmento(cx - 12, cy, cx + 12, cy, 3);
      break;
    case BOCA_TRISTE:
      curva(cx, cy + 4, 26, -7, 3);
      break;
    case BOCA_DIENTES: {
      float px = 0, py = 0;
      for (int i = 0; i <= 6; i++) {
        float x = cx - 18 + 6 * i, y = cy + ((i % 2) ? -4 : 4);
        if (i > 0) segmento(px, py, x, y, 2);
        px = x;
        py = y;
      }
      break;
    }
    case BOCA_ONDULADA: {
      float px = 0, py = 0;
      for (int i = 0; i <= 10; i++) {
        float x = cx - 20 + 4 * i, y = cy + sinf(i * 1.2f + t * 0.01f) * 4;
        if (i > 0) segmento(px, py, x, y, 2);
        px = x;
        py = y;
      }
      break;
    }
    case BOCA_PEQUENA:
      curva(cx, cy - 2, 14, 4, 2);
      break;
    case BOCA_BOSTEZO: {
      float p = t < 1600 ? sinf(t / 1600.0f * PI) : 0;  // se abre y se cierra
      if (p > 0.15f) punto(cx, cy + 2, 4 + 10 * p);
      else curva(cx, cy - 2, 14, 4, 2);
      break;
    }
  }
}

// ---------------------------------------------------------------------------
// Efectos
// ---------------------------------------------------------------------------

static void gotaSudor(uint32_t t) {
  float x = CENTRO_X + SEPARACION + 30, y = 34 + (t / 40) % 24;
  punto(x, y, 4);
  lienzo.fillTriangle(x - 4 + despX, y + despY, x + 4 + despX, y + despY, x + despX, y - 9 + despY, 1);
}

static void zetas(uint32_t t) {
  int sube = (t / 50) % 30;
  lienzo.setTextColor(1);
  lienzo.setTextSize(2);
  lienzo.setCursor(CENTRO_X + 56, 30 - sube / 2);
  lienzo.print("z");
  lienzo.setTextSize(3);
  lienzo.setCursor(CENTRO_X + 68, 14 - sube / 2);
  lienzo.print("Z");
}

static void interferencia() {
  for (int i = 0; i < 3; i++) {
    int y = random(ALTO), alto = random(2, 7);
    lienzo.fillRect(0, y, ANCHO, alto, 0);
    lienzo.fillRect(random(ANCHO - 40), random(ALTO), random(10, 40), 2, 1);
  }
}

// ---------------------------------------------------------------------------
// Parpadeo y mirada
// ---------------------------------------------------------------------------

static void animar(uint32_t ahora) {
  const uint32_t DURACION_PARPADEO = 150;
  if (!(actual.efectos & PARPADEA)) {
    apertura = 1;
  } else if (ahora >= proximoParpadeo + DURACION_PARPADEO) {
    proximoParpadeo = ahora + random(2000, 6000);
    apertura = 1;
  } else if (ahora >= proximoParpadeo) {
    float p = (ahora - proximoParpadeo) / (float)DURACION_PARPADEO;
    apertura = max(0.08f, 1 - sinf(p * PI));
  }

  if (actual.efectos & MIRA) {
    if (ahora >= proximaMirada) {
      proximaMirada = ahora + random(1500, 4000);
      bool alCentro = random(3) == 0;
      objetivoX = alCentro ? 0 : random(-18, 19);
      objetivoY = alCentro ? 0 : random(-10, 11);
    }
  } else {
    objetivoX = actual.mirarX;
    objetivoY = actual.mirarY;
  }
  miradaX += (objetivoX - miradaX) * 0.2f;
  miradaY += (objetivoY - miradaY) * 0.2f;
}

// ---------------------------------------------------------------------------
// Pantalla
// ---------------------------------------------------------------------------

// Envía el lienzo solo si ha cambiado desde la última vez
static void enviar() {
  static uint8_t anterior[(ANCHO + 7) / 8 * ALTO];
  if (memcmp(anterior, lienzo.getBuffer(), sizeof(anterior)) == 0) return;
  memcpy(anterior, lienzo.getBuffer(), sizeof(anterior));

  static uint16_t linea[ANCHO];
  tft.startWrite();
  tft.setAddrWindow((LADO - ANCHO) / 2, OFFSET_Y + (LADO - ALTO) / 2, ANCHO, ALTO);
  for (int y = 0; y < ALTO; y++) {
    for (int x = 0; x < ANCHO; x++) linea[x] = lienzo.getPixel(x, y) ? COLOR_CARA : NEGRO;
    tft.writePixels(linea, ANCHO);
  }
  tft.endWrite();
}

void iniciarCara() {
  tft.begin(40000000);
  tft.fillScreen(NEGRO);
  tft.drawCircle(LADO / 2, OFFSET_Y + LADO / 2, LADO / 2 - 1, GRIS_BORDE);  // borde de la pantalla redonda
}

void ponerCara(const Expresion &e) {
  actual = e;
  inicio = millis();
  apertura = 1;
  proximoParpadeo = inicio + random(1500, 4000);
}

void actualizarCara(uint32_t ahora) {
  if (ahora - ultimoFotograma < 33) return;  // ~30 fotogramas por segundo
  ultimoFotograma = ahora;
  uint32_t t = ahora - inicio;
  animar(ahora);

  float saltoX = 0, saltoY = 0;
  if (actual.efectos & SALTA) saltoY = -fabsf(sinf(t * 0.015f)) * 6;
  if (actual.efectos & TIEMBLA) {
    saltoX = random(-2, 3);
    saltoY += random(-1, 2);
  }

  lienzo.fillScreen(0);
  // Los ojos siguen la mirada entera; la boca, la mitad
  despX = miradaX + saltoX;
  despY = miradaY + saltoY;
  dibujarOjo(-1, t);
  dibujarOjo(1, t);
  if (actual.efectos & SUDOR) gotaSudor(t);
  despX = miradaX / 2 + saltoX;
  despY = miradaY / 2 + saltoY;
  dibujarBoca(t);
  despX = despY = 0;
  if (actual.efectos & ZZZ) zetas(t);
  if (actual.efectos & INTERFERENCIA) interferencia();

  enviar();
}
