// DeskPet · Prueba de caras en Wokwi
//
// Dibuja las caras de docs/emociones.md en un lienzo de 240×240 (el tamaño de la
// pantalla redonda GC9A01). En Wokwi se usa una ILI9341 de 240×320 porque no hay
// GC9A01: el círculo gris marca la zona que se verá en la pantalla real.
//
// Botón = sensor táctil (GPIO 10): pasa a la siguiente cara.
// Monitor serie: escribe el número de una cara para ir a ella.
// Sin tocar nada, las caras van pasando solas cada pocos segundos.

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

#define PIN_CS    7
#define PIN_DC    0
#define PIN_RST   1
#define PIN_TOQUE 10

#define LADO        240   // lienzo cuadrado: la pantalla redonda es de 240×240
#define CENTRO      120
#define OFFSET_Y    40    // centrar el lienzo en la ILI9341 de 240×320
#define AUTO_MS     4000  // tiempo de cada cara en el modo automático

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

const uint16_t NEGRO       = 0x0000;
const uint16_t ROJO        = RGB(255, 40, 50);
const uint16_t ROJO_CLARO  = RGB(255, 160, 160);
const uint16_t ROJO_OSCURO = RGB(100, 0, 10);
const uint16_t GRIS_BORDE  = RGB(45, 45, 45);

Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);
GFXcanvas16 lienzo(LADO, LADO);

// Colores en uso: algunas caras los cambian (furioso parpadea)
uint16_t cPrincipal = ROJO, cBrillo = ROJO_CLARO, cHalo = ROJO_OSCURO;

// ---------------------------------------------------------------------------
// Dibujo con transformación: todo lo que se dibuja pasa por punto(), que aplica
// el desplazamiento de la mirada y el giro de la cara (curioso, tumbado).
// ---------------------------------------------------------------------------

float rotCos = 1, rotSin = 0, despX = 0, despY = 0;

void girar(float grados) {
  float r = grados * PI / 180;
  rotCos = cosf(r);
  rotSin = sinf(r);
}

void punto(float x, float y, int16_t &ox, int16_t &oy) {
  x += despX - CENTRO;
  y += despY - CENTRO;
  ox = lroundf(CENTRO + x * rotCos - y * rotSin);
  oy = lroundf(CENTRO + x * rotSin + y * rotCos);
}

void tri(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t c) {
  int16_t a, b, d, e, f, g;
  punto(x0, y0, a, b);
  punto(x1, y1, d, e);
  punto(x2, y2, f, g);
  lienzo.fillTriangle(a, b, d, e, f, g, c);
}

void circulo(float x, float y, float r, uint16_t c) {
  int16_t a, b;
  punto(x, y, a, b);
  lienzo.fillCircle(a, b, lroundf(r), c);
}

void anillo(float x, float y, float r, float grosor, uint16_t c) {
  circulo(x, y, r, c);
  circulo(x, y, r - grosor, NEGRO);
}

// Línea gruesa con extremos redondeados
void linea(float x0, float y0, float x1, float y1, float grosor, uint16_t c) {
  float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy);
  if (l > 0.01f) {
    float nx = -dy / l * grosor / 2, ny = dx / l * grosor / 2;
    tri(x0 + nx, y0 + ny, x1 + nx, y1 + ny, x1 - nx, y1 - ny, c);
    tri(x0 + nx, y0 + ny, x1 - nx, y1 - ny, x0 - nx, y0 - ny, c);
  }
  circulo(x0, y0, grosor / 2, c);
  circulo(x1, y1, grosor / 2, c);
}

// Curva tipo parábola. flecha > 0: el centro baja (sonrisa ‿); < 0: sube (∩).
// incl inclina la curva (sonrisa torcida).
void curva(float cx, float cy, float ancho, float flecha, float incl, float grosor, uint16_t c) {
  const int pasos = 12;
  float px = 0, py = 0;
  for (int i = 0; i <= pasos; i++) {
    float u = -1 + 2.0f * i / pasos;
    float x = cx + u * ancho / 2;
    float y = cy + flecha * (1 - u * u) + incl * u * ancho / 2;
    if (i > 0) linea(px, py, x, y, grosor, c);
    px = x;
    py = y;
  }
}

// Boca de dientes en zigzag siguiendo una curva
void zigzag(float cx, float cy, float ancho, float flecha, float incl, int dientes, float alto,
            float grosor, uint16_t c) {
  int n = dientes * 2;
  float px = 0, py = 0;
  for (int i = 0; i <= n; i++) {
    float u = -1 + 2.0f * i / n;
    float x = cx + u * ancho / 2;
    float y = cy + flecha * (1 - u * u) + incl * u * ancho / 2 + ((i % 2) ? alto / 2 : -alto / 2);
    if (i > 0) linea(px, py, x, y, grosor, c);
    px = x;
    py = y;
  }
}

// ---------------------------------------------------------------------------
// Piezas de la cara
// ---------------------------------------------------------------------------

// Ojo afilado "malvado". lado: -1 izquierdo, +1 derecho.
// incl: cuánto sube el lado de fuera (más = más enfadado). apertura: 0 cerrado, 1 abierto.
void ojoMalvado(float cx, float cy, int lado, float incl, float apertura) {
  const float px[4] = {-26, 24, 20, -4};
  const float py[4] = {6, -10, 8, 12};
  const float escala[3] = {1.25f, 1.0f, 0.45f};
  const uint16_t color[3] = {cHalo, cPrincipal, cBrillo};
  for (int capa = 0; capa < 3; capa++) {
    float x[4], y[4];
    for (int k = 0; k < 4; k++) {
      float yk = py[k] - (k == 1 ? incl : 0);
      x[k] = cx + lado * px[k] * escala[capa];
      y[k] = cy + yk * escala[capa] * apertura;
    }
    tri(x[0], y[0], x[1], y[1], x[2], y[2], color[capa]);
    tri(x[0], y[0], x[2], y[2], x[3], y[3], color[capa]);
  }
}

void ojoRedondo(float cx, float cy, float r) {
  circulo(cx, cy, r + 4, cHalo);
  circulo(cx, cy, r, cPrincipal);
  circulo(cx - r * 0.3f, cy - r * 0.3f, r * 0.35f, cBrillo);
}

void bocaMalvada(float cx, float cy, float ancho, float flecha, float incl, int dientes, float alto) {
  zigzag(cx, cy, ancho, flecha, incl, dientes, alto, 9, cHalo);
  zigzag(cx, cy, ancho, flecha, incl, dientes, alto, 4, cPrincipal);
}

void trazo(float x0, float y0, float x1, float y1, float grosor) {
  linea(x0, y0, x1, y1, grosor + 5, cHalo);
  linea(x0, y0, x1, y1, grosor, cPrincipal);
}

void arco(float cx, float cy, float ancho, float flecha, float incl, float grosor) {
  curva(cx, cy, ancho, flecha, incl, grosor + 5, cHalo);
  curva(cx, cy, ancho, flecha, incl, grosor, cPrincipal);
}

void espiral(float cx, float cy, float r, float giro) {
  float px = cx, py = cy;
  for (int i = 1; i <= 30; i++) {
    float t = i / 30.0f, a = t * 4 * PI + giro;
    float x = cx + cosf(a) * r * t, y = cy + sinf(a) * r * t;
    linea(px, py, x, y, 4, cPrincipal);
    px = x;
    py = y;
  }
}

void bocaOndulada(float cx, float cy, float ancho, float amplitud, float fase) {
  float px = 0, py = 0;
  for (int i = 0; i <= 12; i++) {
    float x = cx - ancho / 2 + ancho * i / 12;
    float y = cy + sinf(i * 1.1f + fase) * amplitud;
    if (i > 0) linea(px, py, x, y, 4, cPrincipal);
    px = x;
    py = y;
  }
}

// Símbolo de enfado de anime (✳)
void venaEnfado(float cx, float cy, float t) {
  float s = 1 + 0.15f * sinf(t * 0.015f);
  for (int i = 0; i < 4; i++) {
    float a = i * PI / 4;
    linea(cx - cosf(a) * 9 * s, cy - sinf(a) * 9 * s, cx + cosf(a) * 9 * s, cy + sinf(a) * 9 * s, 3, cPrincipal);
  }
}

void texto(const char *s, int16_t x, int16_t y, uint8_t tam) {
  lienzo.setTextSize(tam);
  lienzo.setTextColor(cPrincipal);
  lienzo.setCursor(x, y);
  lienzo.print(s);
}

// Interferencia digital: desplaza franjas horizontales y oscurece una línea de cada tres
void glitch(float fuerza) {
  uint16_t *buf = lienzo.getBuffer();
  int bandas = 1 + (int)(fuerza * 5);
  for (int b = 0; b < bandas; b++) {
    int y0 = random(LADO), alto = random(3, 10 + (int)(fuerza * 20));
    int dx = (int)(random(-30, 31) * fuerza);
    for (int y = y0; y < min(LADO, y0 + alto); y++) {
      uint16_t *fila = buf + y * LADO;
      if (dx > 0) {
        memmove(fila + dx, fila, (LADO - dx) * 2);
        memset(fila, 0, dx * 2);
      } else if (dx < 0) {
        memmove(fila, fila - dx, (LADO + dx) * 2);
        memset(fila + LADO + dx, 0, -dx * 2);
      }
    }
  }
  for (int y = 0; y < LADO; y += 3) {
    uint16_t *fila = buf + y * LADO;
    for (int x = 0; x < LADO; x++) fila[x] = (fila[x] >> 1) & 0x7BEF;
  }
}

// ---------------------------------------------------------------------------
// Animación común: parpadeo y mirada
// ---------------------------------------------------------------------------

float apertura = 1;              // 1 = ojos abiertos; baja a 0 al parpadear
float miradaX = 0, miradaY = 0;  // hacia dónde mira (se suma a despX/despY)
float objetivoX = 0, objetivoY = 0;
uint32_t proximoParpadeo = 0, proximaMirada = 0;

void animarComun(uint32_t ahora) {
  const uint32_t DURACION_PARPADEO = 160;
  if (ahora >= proximoParpadeo + DURACION_PARPADEO) {
    proximoParpadeo = ahora + random(2000, 6000);
    apertura = 1;
  } else if (ahora >= proximoParpadeo) {
    float p = (ahora - proximoParpadeo) / (float)DURACION_PARPADEO;
    apertura = max(0.08f, 1 - sinf(p * PI));
  }

  if (ahora >= proximaMirada) {
    proximaMirada = ahora + random(1500, 4000);
    bool alCentro = random(3) == 0;
    objetivoX = alCentro ? 0 : random(-14, 15);
    objetivoY = alCentro ? 0 : random(-7, 8);
  }
  miradaX += (objetivoX - miradaX) * 0.15f;
  miradaY += (objetivoY - miradaY) * 0.15f;
}

// ---------------------------------------------------------------------------
// Caras. t = milisegundos desde que empezó la cara.
// ---------------------------------------------------------------------------

const float OJO_IZQ = 78, OJO_DER = 162, OJOS_Y = 100, BOCA_Y = 160;

void caraNormal(uint32_t t) {
  ojoMalvado(OJO_IZQ, OJOS_Y, -1, 8, apertura);
  ojoMalvado(OJO_DER, OJOS_Y, 1, 8, apertura);
  bocaMalvada(CENTRO, BOCA_Y, 80, 8, 0, 4, 10);
}

void caraSonriente(uint32_t t) {
  arco(OJO_IZQ, OJOS_Y + 4, 46, -14, 0.1f, 7);
  arco(OJO_DER, OJOS_Y + 4, 46, -14, -0.1f, 7);
  float rebote = sinf(t * 0.012f) * 2;
  bocaMalvada(CENTRO, BOCA_Y - 4 + rebote, 112, 20, 0, 5, 12);
}

void caraBurlon(uint32_t t) {
  trazo(OJO_IZQ - 20, OJOS_Y - 12, OJO_IZQ + 16, OJOS_Y + 2, 7);  // guiño ">"
  trazo(OJO_IZQ + 16, OJOS_Y + 2, OJO_IZQ - 20, OJOS_Y + 14, 7);
  ojoMalvado(OJO_DER, OJOS_Y, 1, 12, apertura);
  bocaMalvada(CENTRO + 10, BOCA_Y, 74, 8, -0.18f, 4, 9);
}

void caraSorprendido(uint32_t t) {
  float crece = min(1.0f, 0.5f + t / 250.0f);
  for (float x : {OJO_IZQ, OJO_DER}) {
    anillo(x, OJOS_Y, 28 * crece, 13, cHalo);
    anillo(x, OJOS_Y, 25 * crece, 8, cPrincipal);
  }
  anillo(CENTRO, BOCA_Y + 8, 14 * crece, 10, cHalo);
  anillo(CENTRO, BOCA_Y + 8, 12 * crece, 6, cPrincipal);
}

void caraIrritado(uint32_t t) {
  ojoMalvado(OJO_IZQ, OJOS_Y + 4, -1, 16, apertura * 0.7f);
  ojoMalvado(OJO_DER, OJOS_Y + 4, 1, 16, apertura * 0.7f);
  bocaMalvada(CENTRO, BOCA_Y + 8, 54, -8, 0.08f, 3, 6);
  venaEnfado(186, 54, t);
}

void dibujarEnfadado(float apert) {
  ojoMalvado(OJO_IZQ, OJOS_Y + 6, -1, 22, apert * 0.6f);
  ojoMalvado(OJO_DER, OJOS_Y + 6, 1, 22, apert * 0.6f);
  trazo(OJO_IZQ - 26, OJOS_Y - 30, OJO_IZQ + 24, OJOS_Y - 8, 7);  // cejas
  trazo(OJO_DER + 26, OJOS_Y - 30, OJO_DER - 24, OJOS_Y - 8, 7);
  arco(CENTRO, BOCA_Y + 14, 64, -14, 0, 6);
}

void caraEnfadado(uint32_t t) {
  dibujarEnfadado(apertura);
}

void caraFurioso(uint32_t t) {
  despX += random(-4, 5);
  despY += random(-3, 4);
  if ((t / 120) % 2) cPrincipal = ROJO_CLARO;
  dibujarEnfadado(1);
  venaEnfado(186, 50, t);
  venaEnfado(54, 50, t + 400);
}

void caraMareado(uint32_t t) {
  espiral(OJO_IZQ, OJOS_Y, 26, t * 0.01f);
  espiral(OJO_DER, OJOS_Y, 26, -t * 0.01f);
  bocaOndulada(CENTRO, BOCA_Y + 6, 70, 6, t * 0.01f);
  despX += sinf(t * 0.004f) * 8;
}

void caraAsustado(uint32_t t) {
  float mira = sinf(t * 0.012f) * 14;
  ojoRedondo(OJO_IZQ + mira, OJOS_Y, 10);
  ojoRedondo(OJO_DER + mira, OJOS_Y, 10);
  bocaOndulada(CENTRO, BOCA_Y + 8, 46, 3, t * 0.05f);
}

void caraProtesta(uint32_t t) {
  despX += random(-2, 3);
  trazo(OJO_IZQ - 20, OJOS_Y - 16, OJO_IZQ + 18, OJOS_Y, 8);  // ">"
  trazo(OJO_IZQ + 18, OJOS_Y, OJO_IZQ - 20, OJOS_Y + 16, 8);
  trazo(OJO_DER + 20, OJOS_Y - 16, OJO_DER - 18, OJOS_Y, 8);  // "<"
  trazo(OJO_DER - 18, OJOS_Y, OJO_DER + 20, OJOS_Y + 16, 8);
  // Dientes apretados
  float x0 = 88, x1 = 152, y0 = BOCA_Y - 2, y1 = BOCA_Y + 24;
  trazo(x0, y0, x1, y0, 4);
  trazo(x0, y1, x1, y1, 4);
  trazo(x0, y0, x0, y1, 4);
  trazo(x1, y0, x1, y1, 4);
  trazo(x0, (y0 + y1) / 2, x1, (y0 + y1) / 2, 3);
  for (float x = x0 + 16; x < x1; x += 16) trazo(x, y0, x, y1, 3);
}

void caraCurioso(uint32_t t) {
  girar(14);
  despX += 10;
  ojoRedondo(OJO_IZQ + 8, OJOS_Y, 20 * max(apertura, 0.3f));
  ojoRedondo(OJO_DER + 8, OJOS_Y - 4, 24 * max(apertura, 0.3f));
  arco(CENTRO + 8, BOCA_Y + 8, 26, -4, 0, 5);
}

void caraTumbado(uint32_t t) {
  girar(90);
  ojoMalvado(OJO_IZQ, OJOS_Y, -1, 6, apertura * 0.45f);
  ojoMalvado(OJO_DER, OJOS_Y, 1, 6, apertura * 0.45f);
  trazo(CENTRO - 24, BOCA_Y, CENTRO + 24, BOCA_Y, 5);
}

void caraHacker(uint32_t t) {
  despX += random(-3, 4);
  ojoMalvado(OJO_IZQ, OJOS_Y, -1, 10, 1);
  ojoMalvado(OJO_DER, OJOS_Y, 1, 10, 1);
  bocaMalvada(CENTRO, BOCA_Y, 84, 8, 0, 5, 10);
  texto("??", 150, 36, 3);
  // Líneas sueltas como en la imagen de diseño
  for (int i = 0; i < 4; i++) {
    int y = random(40, 200), x = random(30, 170);
    lienzo.fillRect(x, y, random(10, 40), 3, cPrincipal);
  }
  glitch(0.8f);
}

void caraAburrido(uint32_t t) {
  despY += 10;
  ojoMalvado(OJO_IZQ, OJOS_Y, -1, 4, apertura * 0.4f);
  ojoMalvado(OJO_DER, OJOS_Y, 1, 4, apertura * 0.4f);
  trazo(CENTRO - 22, BOCA_Y, CENTRO + 22, BOCA_Y, 5);
}

void dibujarDormido(uint32_t t) {
  arco(OJO_IZQ, OJOS_Y, 40, 9, 0, 6);
  arco(OJO_DER, OJOS_Y, 40, 9, 0, 6);
  arco(CENTRO, BOCA_Y, 20, 4, 0, 4);
}

void caraDormido(uint32_t t) {
  despY += sinf(t * 0.003f) * 3;  // respira
  dibujarDormido(t);
  // "Zzz" que sube y vuelve a empezar
  int sube = (t / 40) % 40;
  texto("z", 168, 64 - sube / 2, 2);
  texto("Z", 182, 48 - sube / 2, 3);
}

void caraDespertando(uint32_t t) {
  if (t < 700) {
    dibujarDormido(t);
    return;
  }
  float abre = min(1.0f, (t - 700) / 300.0f);
  ojoMalvado(OJO_IZQ, OJOS_Y, -1, 8, abre);
  if (t < 1300) {
    arco(OJO_DER, OJOS_Y, 40, 9, 0, 6);
  } else {
    ojoMalvado(OJO_DER, OJOS_Y, 1, 8, min(1.0f, (t - 1300) / 300.0f));
  }
  if (t > 1700 && t < 2900) {  // bostezo
    float p = sinf((t - 1700) / 1200.0f * PI);
    anillo(CENTRO, BOCA_Y + 6, 6 + 14 * p, 6 + 4 * p, cPrincipal);
  } else {
    arco(CENTRO, BOCA_Y, 24, 4, 0, 4);
  }
}

void caraArranque(uint32_t t) {
  if (t < 300 && random(2)) return;  // parpadeo de encendido
  caraNormal(t);
  if (t < 1800) glitch(1 - t / 1800.0f);
}

struct Cara {
  const char *nombre;
  void (*dibujar)(uint32_t t);
  bool animacionComun;  // parpadeo y mirada aleatoria
};

const Cara CARAS[] = {
  {"Normal (amenazante)", caraNormal, true},
  {"Sonriente (lado oscuro)", caraSonriente, true},
  {"Burlón", caraBurlon, true},
  {"Sorprendido", caraSorprendido, false},
  {"Irritado", caraIrritado, true},
  {"Enfadado", caraEnfadado, true},
  {"Furioso", caraFurioso, false},
  {"Mareado", caraMareado, false},
  {"Asustado", caraAsustado, false},
  {"Protesta", caraProtesta, false},
  {"Curioso", caraCurioso, true},
  {"Tumbado", caraTumbado, true},
  {"Modo hacker", caraHacker, false},
  {"Aburrido", caraAburrido, true},
  {"Dormido", caraDormido, false},
  {"Despertando", caraDespertando, false},
  {"Arranque", caraArranque, false},
};
const int NUM_CARAS = sizeof(CARAS) / sizeof(CARAS[0]);

// ---------------------------------------------------------------------------

int caraActual = NUM_CARAS - 1;  // empieza con la animación de arranque
uint32_t inicioCara = 0;
bool toqueAnterior = false;
uint32_t ultimoCambioToque = 0;

void cambiarCara(int i) {
  caraActual = (i + NUM_CARAS) % NUM_CARAS;
  inicioCara = millis();
  Serial.printf("Cara %d: %s\n", caraActual, CARAS[caraActual].nombre);
}

void leerEntradas(uint32_t ahora) {
  bool toque = digitalRead(PIN_TOQUE);
  if (toque != toqueAnterior && ahora - ultimoCambioToque > 50) {
    ultimoCambioToque = ahora;
    toqueAnterior = toque;
    if (toque) cambiarCara(caraActual + 1);
  }
  if (Serial.available()) {
    int n = Serial.parseInt();
    while (Serial.available()) Serial.read();
    if (n >= 0 && n < NUM_CARAS) cambiarCara(n);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOQUE, INPUT_PULLDOWN);  // el TTP223 da HIGH al tocarlo
  tft.begin(40000000);
  tft.fillScreen(NEGRO);
  Serial.println("DeskPet · caras. Botón = siguiente cara; escribe un número para elegir una:");
  for (int i = 0; i < NUM_CARAS; i++) Serial.printf("  %2d  %s\n", i, CARAS[i].nombre);
  cambiarCara(caraActual);
}

void loop() {
  static uint32_t fotogramas = 0, inicioFps = 0;
  uint32_t ahora = millis();
  leerEntradas(ahora);
  if (ahora - inicioCara > AUTO_MS) cambiarCara(caraActual + 1);

  const Cara &cara = CARAS[caraActual];
  if (cara.animacionComun) {
    animarComun(ahora);
  } else {
    apertura = 1;
    miradaX = miradaY = 0;
  }
  cPrincipal = ROJO;
  girar(0);
  despX = miradaX;
  despY = miradaY;

  lienzo.fillScreen(NEGRO);
  cara.dibujar(ahora - inicioCara);
  lienzo.drawCircle(CENTRO, CENTRO, CENTRO - 1, GRIS_BORDE);  // borde de la pantalla redonda
  tft.drawRGBBitmap(0, OFFSET_Y, lienzo.getBuffer(), LADO, LADO);

  fotogramas++;
  if (ahora - inicioFps >= 2000) {
    Serial.printf("%.1f fps\n", fotogramas * 1000.0f / (ahora - inicioFps));
    fotogramas = 0;
    inicioFps = ahora;
  }
}
