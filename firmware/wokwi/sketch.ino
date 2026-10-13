// DeskPet · Ojos con RoboEyes en la pantalla redonda
//
// Usa la librería FluxGarage RoboEyes (pensada para OLED de un color) en la
// pantalla redonda GC9A01 de 240×240. En Wokwi se usa una ILI9341 de 240×320
// porque no hay GC9A01: el círculo gris marca la zona que se verá de verdad.
//
// RoboEyes dibuja en un lienzo de 1 bit (encendido/apagado) y PantallaRedonda
// lo pinta en rojo, centrado dentro del círculo.
//
// Botón = sensor táctil (GPIO 10): pasa a la siguiente emoción.
// Monitor serie: escribe el número de una emoción para ir a ella.
// Sin tocar nada, las emociones van pasando solas cada pocos segundos.

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

#define PIN_CS    7
#define PIN_DC    0
#define PIN_RST   1
#define PIN_TOQUE 10

#define LADO      240   // pantalla redonda de 240×240
#define OFFSET_Y  40    // centrar los 240×240 en la ILI9341 de 240×320
#define OJOS_ANCHO 180  // zona donde se mueven los ojos: cabe entera dentro del círculo
#define OJOS_ALTO  140
#define AUTO_MS   5000  // tiempo de cada emoción en el modo automático

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

const uint16_t NEGRO      = 0x0000;
const uint16_t ROJO       = RGB(255, 30, 40);
const uint16_t GRIS_BORDE = RGB(45, 45, 45);

Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);

// Lienzo de 1 bit con los dos métodos que RoboEyes espera de una pantalla OLED
class PantallaRedonda : public GFXcanvas1 {
 public:
  uint16_t color = ROJO;

  PantallaRedonda() : GFXcanvas1(OJOS_ANCHO, OJOS_ALTO) {}

  void clearDisplay() { fillScreen(0); }

  // Pinta el lienzo centrado en la pantalla, con el borde del círculo
  void display() {
    static uint16_t linea[LADO];
    const int x0 = (LADO - OJOS_ANCHO) / 2, y0 = (LADO - OJOS_ALTO) / 2;
    const int r = LADO / 2;
    tft.startWrite();
    tft.setAddrWindow(0, OFFSET_Y, LADO, LADO);
    for (int y = 0; y < LADO; y++) {
      int dy = y - r;
      for (int x = 0; x < LADO; x++) {
        int dx = x - r, d2 = dx * dx + dy * dy;
        uint16_t c = NEGRO;
        if (d2 >= (r - 1) * (r - 1) && d2 < r * r) {
          c = GRIS_BORDE;
        } else if (x >= x0 && x < x0 + OJOS_ANCHO && y >= y0 && y < y0 + OJOS_ALTO &&
                   getPixel(x - x0, y - y0)) {
          c = color;
        }
        linea[x] = c;
      }
      tft.writePixels(linea, LADO);
    }
    tft.endWrite();
  }
};

PantallaRedonda pantalla;

#include <FluxGarage_RoboEyes.h>
RoboEyes<PantallaRedonda> ojos(pantalla);

// ---------------------------------------------------------------------------
// Emociones de prueba. RoboEyes trae 4 estados de ánimo (DEFAULT, TIRED, ANGRY,
// HAPPY) y varios efectos; aquí se combinan para acercarse a docs/emociones.md.
// ---------------------------------------------------------------------------

void reiniciarOjos() {
  ojos.setWidth(64, 64);
  ojos.setHeight(64, 64);
  ojos.setBorderradius(16, 16);
  ojos.setSpacebetween(20);
  ojos.setMood(DEFAULT);
  ojos.setPosition(DEFAULT);
  ojos.setCuriosity(OFF);
  ojos.setSweat(OFF);
  ojos.setHFlicker(OFF);
  ojos.setVFlicker(OFF);
  ojos.setAutoblinker(ON, 3, 2);
  ojos.setIdleMode(ON, 2, 2);
  ojos.open();
  pantalla.color = ROJO;
}

void normal() {}

void enfadado() {
  ojos.setMood(ANGRY);
}

void furioso() {
  ojos.setMood(ANGRY);
  ojos.setHFlicker(ON, 2);
  ojos.setIdleMode(OFF);
  ojos.setPosition(DEFAULT);
}

void contento() {
  ojos.setMood(HAPPY);
  ojos.anim_laugh();
}

void aburrido() {
  ojos.setMood(TIRED);
  ojos.setIdleMode(OFF);
  ojos.setPosition(S);
}

void sorprendido() {
  ojos.setHeight(80, 80);
  ojos.setWidth(56, 56);
  ojos.setBorderradius(28, 28);
  ojos.setIdleMode(OFF);
  ojos.setPosition(DEFAULT);
}

void mareado() {
  ojos.anim_confused();
  ojos.setHFlicker(ON, 3);
}

void asustado() {
  ojos.setWidth(44, 44);
  ojos.setHeight(44, 44);
  ojos.setSweat(ON);
  ojos.setVFlicker(ON, 2);
  ojos.setIdleMode(ON, 1, 0);
}

void curioso() {
  ojos.setCuriosity(ON);
  ojos.setIdleMode(ON, 1, 1);
}

void dormido() {
  ojos.setAutoblinker(OFF);
  ojos.setIdleMode(OFF);
  ojos.setPosition(DEFAULT);
  ojos.close();
}

struct Emocion {
  const char *nombre;
  void (*aplicar)();
};

const Emocion EMOCIONES[] = {
  {"Normal", normal},
  {"Enfadado", enfadado},
  {"Furioso", furioso},
  {"Contento", contento},
  {"Aburrido", aburrido},
  {"Sorprendido", sorprendido},
  {"Mareado", mareado},
  {"Asustado", asustado},
  {"Curioso", curioso},
  {"Dormido", dormido},
};
const int NUM_EMOCIONES = sizeof(EMOCIONES) / sizeof(EMOCIONES[0]);

// ---------------------------------------------------------------------------

int emocionActual = 0;
uint32_t inicioEmocion = 0;
bool toqueAnterior = false;
uint32_t ultimoCambioToque = 0;

void cambiarEmocion(int i) {
  emocionActual = (i + NUM_EMOCIONES) % NUM_EMOCIONES;
  inicioEmocion = millis();
  reiniciarOjos();
  EMOCIONES[emocionActual].aplicar();
  Serial.printf("Emoción %d: %s\n", emocionActual, EMOCIONES[emocionActual].nombre);
}

void leerEntradas(uint32_t ahora) {
  bool toque = digitalRead(PIN_TOQUE);
  if (toque != toqueAnterior && ahora - ultimoCambioToque > 50) {
    ultimoCambioToque = ahora;
    toqueAnterior = toque;
    if (toque) cambiarEmocion(emocionActual + 1);
  }
  if (Serial.available()) {
    int n = Serial.parseInt();
    while (Serial.available()) Serial.read();
    if (n >= 0 && n < NUM_EMOCIONES) cambiarEmocion(n);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOQUE, INPUT_PULLDOWN);  // el TTP223 da HIGH al tocarlo
  tft.begin(40000000);
  tft.fillScreen(NEGRO);

  ojos.begin(OJOS_ANCHO, OJOS_ALTO, 30);
  Serial.println("DeskPet · RoboEyes. Botón = siguiente emoción; escribe un número para elegir una:");
  for (int i = 0; i < NUM_EMOCIONES; i++) Serial.printf("  %d  %s\n", i, EMOCIONES[i].nombre);
  cambiarEmocion(0);
}

void loop() {
  uint32_t ahora = millis();
  leerEntradas(ahora);
  if (ahora - inicioEmocion > AUTO_MS) cambiarEmocion(emocionActual + 1);
  ojos.update();
}
