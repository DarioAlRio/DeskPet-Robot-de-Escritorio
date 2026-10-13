// DeskPet · Sensores y emociones (prueba en Wokwi)
//
// Lee el sensor táctil y el MPU6050, convierte lo que pasa en eventos (toque,
// caricia, golpe, sacudida, boca abajo…) y una máquina de estados decide qué
// emoción muestran los ojos de RoboEyes. Reglas en docs/emociones.md.
//
// En Wokwi:
//   - Botón rojo = sensor táctil. Pulsación corta = toque; mantenido = caricia;
//     2, 3 o 5 toques seguidos = burlón, irritado, modo hacker.
//   - Pulsa el MPU6050 durante la simulación para mover sus deslizadores:
//       az = -1 g         boca abajo      ax = ±0,5 g     inclinado
//       ax = ±1, az = 0   tumbado         subir y bajar rápido un eje = golpe / sacudida
//       az > 1,3 g        levantado
//
// En Wokwi se usa una ILI9341 de 240×320 porque no hay GC9A01: el círculo gris
// marca la zona que se verá en la pantalla redonda de verdad.

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Wire.h>

#define PIN_CS    7
#define PIN_DC    0
#define PIN_RST   1
#define PIN_TOQUE 10
#define PIN_SDA   8
#define PIN_SCL   9

#define LADO       240  // pantalla redonda de 240×240
#define OFFSET_Y   40   // centrar los 240×240 en la ILI9341 de 240×320
#define OJOS_ANCHO 180  // zona donde se mueven los ojos: cabe entera dentro del círculo
#define OJOS_ALTO  140

// 1 = tiempos cortos para probar en el simulador; 0 = tiempos reales
#define MODO_PRUEBA 1
#if MODO_PRUEBA
const uint32_t T_ABURRIDO = 20000, T_DORMIDO = 40000;
#else
const uint32_t T_ABURRIDO = 2 * 60000UL, T_DORMIDO = 5 * 60000UL;
#endif

#define RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

const uint16_t NEGRO      = 0x0000;
const uint16_t ROJO       = RGB(255, 30, 40);
const uint16_t GRIS_BORDE = RGB(45, 45, 45);

// ---------------------------------------------------------------------------
// Pantalla: RoboEyes dibuja en un lienzo de 1 bit y aquí se pinta en rojo,
// centrado dentro del círculo.
// ---------------------------------------------------------------------------

Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);

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

  // Borde de la pantalla redonda (se dibuja una vez)
  void dibujarBorde() {
    tft.drawCircle(LADO / 2, OFFSET_Y + LADO / 2, LADO / 2 - 1, GRIS_BORDE);
  }
};

PantallaRedonda pantalla;

#include <FluxGarage_RoboEyes.h>
RoboEyes<PantallaRedonda> ojos(pantalla);

// ---------------------------------------------------------------------------
// Tipos. Van antes de cualquier función porque el IDE de Arduino declara las
// funciones al principio del archivo y tienen que conocer estos tipos.
// ---------------------------------------------------------------------------

enum Postura { DE_PIE, INCLINADO_IZQ, INCLINADO_DER, TUMBADO, BOCA_ABAJO };

enum Emocion {
  NORMAL, CONTENTO, BURLON, SORPRENDIDO, IRRITADO, ENFADADO, FURIOSO, MAREADO,
  ASUSTADO, PROTESTA, CURIOSO, TUMBADO_E, HACKER, ABURRIDO, DORMIDO, DESPERTANDO
};

// ---------------------------------------------------------------------------
// Eventos
// ---------------------------------------------------------------------------

enum Evento {
  NINGUNO, TOQUE, DOBLE_TOQUE, TRIPLE_TOQUE, MUCHOS_TOQUES, CARICIA,
  GOLPE_SUAVE, GOLPE_FUERTE, SACUDIDA, LEVANTADO
};
const char *NOMBRE_EVENTO[] = {
  "-", "toque", "doble toque", "triple toque", "5 toques", "caricia",
  "golpe suave", "golpe fuerte", "sacudida", "levantado"
};

// ---------------------------------------------------------------------------
// Sensor táctil (TTP223: HIGH mientras se toca)
// ---------------------------------------------------------------------------

const uint32_t REBOTE_MS = 30, CARICIA_MS = 600, ENTRE_TOQUES_MS = 400;

struct {
  bool tocando = false, caricia = false;
  uint32_t ultimoCambio = 0, ultimoSoltar = 0;
  int toques = 0;
} tactil;

Evento leerTactil(uint32_t ahora) {
  bool tocando = digitalRead(PIN_TOQUE);
  if (tocando != tactil.tocando && ahora - tactil.ultimoCambio > REBOTE_MS) {
    tactil.tocando = tocando;
    tactil.ultimoCambio = ahora;
    if (!tocando) {
      if (tactil.caricia) {
        tactil.caricia = false;
      } else {
        tactil.toques++;
        tactil.ultimoSoltar = ahora;
      }
    }
  }
  // Mantener el dedo = caricia (salta sin esperar a soltar)
  if (tactil.tocando && !tactil.caricia && ahora - tactil.ultimoCambio >= CARICIA_MS) {
    tactil.caricia = true;
    tactil.toques = 0;
    return CARICIA;
  }
  // Toques cortos: se cuentan hasta que pasa un rato sin otro toque
  if (!tactil.tocando && tactil.toques > 0 && ahora - tactil.ultimoSoltar > ENTRE_TOQUES_MS) {
    int n = tactil.toques;
    tactil.toques = 0;
    if (n == 1) return TOQUE;
    if (n == 2) return DOBLE_TOQUE;
    if (n < 5) return TRIPLE_TOQUE;
    return MUCHOS_TOQUES;
  }
  return NINGUNO;
}

// ---------------------------------------------------------------------------
// MPU6050: golpes, sacudidas, levantado y postura
// ---------------------------------------------------------------------------

#define MPU_DIR 0x68
const float LSB_POR_G = 4096;  // rango ±8 g

const float UMBRAL_PICO   = 0.6f;  // g: por debajo se ignora (teclear cerca, vibraciones)
const float UMBRAL_FUERTE = 1.5f;  // g: golpe fuerte
const int   PICOS_SACUDIDA = 4;    // picos en 1 s = sacudida
const float UMBRAL_LEVANTAR = 1.25f;  // g total mantenido = lo están subiendo
const uint32_t LEVANTAR_MS = 100, POSTURA_MS = 400;

const char *NOMBRE_POSTURA[] = {"de pie", "inclinado izq.", "inclinado der.", "tumbado", "boca abajo"};

struct {
  bool ok = false;
  float gx = 0, gy = 0, gz = 1;  // gravedad (aceleración filtrada)
  Postura postura = DE_PIE, candidata = DE_PIE;
  uint32_t candidataDesde = 0, ultimaLectura = 0, ultimoPico = 0, ultimoGolpe = 0;
  uint32_t picos[PICOS_SACUDIDA] = {0};
  int siguientePico = 0;
  uint32_t subiendoDesde = 0, ultimoLevantado = 0;
} mov;

void escribirMPU(uint8_t reg, uint8_t valor) {
  Wire.beginTransmission(MPU_DIR);
  Wire.write(reg);
  Wire.write(valor);
  Wire.endTransmission();
}

void iniciarMPU() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(MPU_DIR);
  mov.ok = Wire.endTransmission() == 0;
  if (!mov.ok) {
    Serial.println("MPU6050 no encontrado: solo funcionará el táctil");
    return;
  }
  escribirMPU(0x6B, 0x00);  // despertar
  escribirMPU(0x1C, 0x10);  // acelerómetro a ±8 g
}

bool leerAcelerometro(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU_DIR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(MPU_DIR, 6) != 6) return false;
  int16_t x = (Wire.read() << 8) | Wire.read();
  int16_t y = (Wire.read() << 8) | Wire.read();
  int16_t z = (Wire.read() << 8) | Wire.read();
  ax = x / LSB_POR_G;
  ay = y / LSB_POR_G;
  az = z / LSB_POR_G;
  return true;
}

Postura calcularPostura() {
  float g = sqrtf(mov.gx * mov.gx + mov.gy * mov.gy + mov.gz * mov.gz);
  if (g < 0.3f) return mov.postura;  // en caída libre o leyendo mal: no cambiar
  float nz = mov.gz / g, nx = mov.gx / g;
  if (nz < -0.7f) return BOCA_ABAJO;
  if (nz < 0.4f) return TUMBADO;
  if (nz < 0.9f) return nx > 0 ? INCLINADO_DER : INCLINADO_IZQ;
  return DE_PIE;
}

Evento leerMovimiento(uint32_t ahora) {
  if (!mov.ok || ahora - mov.ultimaLectura < 10) return NINGUNO;  // 100 lecturas/s
  mov.ultimaLectura = ahora;
  float ax, ay, az;
  if (!leerAcelerometro(ax, ay, az)) return NINGUNO;

  // La gravedad cambia despacio; un golpe es lo que se aparta de ella
  const float k = 0.1f;
  mov.gx += (ax - mov.gx) * k;
  mov.gy += (ay - mov.gy) * k;
  mov.gz += (az - mov.gz) * k;
  float dx = ax - mov.gx, dy = ay - mov.gy, dz = az - mov.gz;
  float pico = sqrtf(dx * dx + dy * dy + dz * dz);
  float total = sqrtf(ax * ax + ay * ay + az * az);

  // Postura: tiene que mantenerse un rato para contar
  Postura p = calcularPostura();
  if (p != mov.candidata) {
    mov.candidata = p;
    mov.candidataDesde = ahora;
  } else if (p != mov.postura && ahora - mov.candidataDesde >= POSTURA_MS) {
    mov.postura = p;
    Serial.printf("Postura: %s\n", NOMBRE_POSTURA[p]);
  }

  // Levantado: aceleración hacia arriba mantenida estando de pie
  if (total > UMBRAL_LEVANTAR && mov.postura == DE_PIE) {
    if (mov.subiendoDesde == 0) mov.subiendoDesde = ahora;
    if (ahora - mov.subiendoDesde >= LEVANTAR_MS && ahora - mov.ultimoLevantado > 2000) {
      mov.ultimoLevantado = ahora;
      return LEVANTADO;
    }
  } else {
    mov.subiendoDesde = 0;
  }

  // Golpes y sacudidas
  if (pico > UMBRAL_PICO && ahora - mov.ultimoPico > 80) {
    mov.ultimoPico = ahora;
    uint32_t masAntiguo = mov.picos[mov.siguientePico];
    mov.picos[mov.siguientePico] = ahora;
    mov.siguientePico = (mov.siguientePico + 1) % PICOS_SACUDIDA;
    if (masAntiguo != 0 && ahora - masAntiguo < 1000) {
      memset(mov.picos, 0, sizeof(mov.picos));
      return SACUDIDA;
    }
    if (ahora - mov.ultimoGolpe > 300) {
      mov.ultimoGolpe = ahora;
      return pico > UMBRAL_FUERTE ? GOLPE_FUERTE : GOLPE_SUAVE;
    }
  }
  return NINGUNO;
}

// ---------------------------------------------------------------------------
// Emociones: cómo se ve cada una con RoboEyes
// ---------------------------------------------------------------------------

void reiniciarOjos() {
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

void quieto() {
  ojos.setIdleMode(OFF);
  ojos.setPosition(DEFAULT);
}

void verNormal() {}
void verContento() { ojos.setMood(HAPPY); ojos.anim_laugh(); }
void verBurlon() { ojos.setMood(HAPPY); ojos.blink(true, false); }  // guiño
void verSorprendido() { quieto(); ojos.setWidth(56, 56); ojos.setHeight(80, 80); ojos.setBorderradius(28, 28); }
void verIrritado() { ojos.setMood(ANGRY); ojos.setHeight(48, 48); }
void verEnfadado() { quieto(); ojos.setMood(ANGRY); }
void verFurioso() { quieto(); ojos.setMood(ANGRY); ojos.setHFlicker(ON, 2); }
void verMareado() { ojos.anim_confused(); ojos.setHFlicker(ON, 3); }
void verAsustado() { ojos.setWidth(44, 44); ojos.setHeight(44, 44); ojos.setSweat(ON); ojos.setVFlicker(ON, 2); ojos.setIdleMode(ON, 1, 0); }
void verProtesta() { quieto(); ojos.setMood(ANGRY); ojos.setHeight(24, 24); ojos.setVFlicker(ON, 1); }
void verCurioso() {
  ojos.setIdleMode(OFF);
  ojos.setCuriosity(ON);
  ojos.setPosition(mov.postura == INCLINADO_DER ? E : W);
}
void verTumbado() { quieto(); ojos.setMood(TIRED); ojos.setPosition(S); }
void verHacker() { ojos.setCyclops(ON); ojos.setHFlicker(ON, 4); ojos.setIdleMode(ON, 1, 0); }
void verAburrido() { quieto(); ojos.setMood(TIRED); ojos.setPosition(S); }
void verDormido() { quieto(); ojos.setAutoblinker(OFF); ojos.close(); }
void verDespertando() { quieto(); ojos.setMood(TIRED); }

struct DatosEmocion {
  const char *nombre;
  uint8_t prioridad;  // si coinciden varias, gana la más alta (docs/emociones.md)
  uint16_t duracion;  // ms; 0 = dura mientras se cumpla su condición
  void (*ver)();
};

const DatosEmocion EMOCIONES[] = {
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

Emocion mostrada = NORMAL;
Emocion momentanea = NORMAL;  // la que ha provocado el último evento
uint32_t momentaneaHasta = 0, ultimaInteraccion = 0;
Postura posturaAnterior = DE_PIE;

void lanzar(Emocion e, uint32_t ahora) {
  bool activa = ahora < momentaneaHasta;
  if (activa && EMOCIONES[e].prioridad < EMOCIONES[momentanea].prioridad) return;
  momentanea = e;
  momentaneaHasta = ahora + EMOCIONES[e].duracion;
}

void atenderEvento(Evento ev, uint32_t ahora) {
  Serial.printf("Evento: %s\n", NOMBRE_EVENTO[ev]);
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

Emocion elegirEmocion(uint32_t ahora) {
  Emocion candidatas[3];
  int n = 0;

  // Por la postura (dura mientras siga así)
  if (mov.postura == BOCA_ABAJO) candidatas[n++] = PROTESTA;
  else if (mov.postura == TUMBADO) candidatas[n++] = TUMBADO_E;
  else if (mov.postura != DE_PIE) candidatas[n++] = CURIOSO;

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

void mostrar(Emocion e) {
  mostrada = e;
  reiniciarOjos();
  EMOCIONES[e].ver();
  Serial.printf("Emoción: %s\n", EMOCIONES[e].nombre);
}

// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOQUE, INPUT_PULLDOWN);  // el TTP223 da HIGH al tocarlo
  tft.begin(40000000);
  tft.fillScreen(NEGRO);
  pantalla.dibujarBorde();
  iniciarMPU();

  ojos.begin(OJOS_ANCHO, OJOS_ALTO, 30);
  mostrar(NORMAL);
  Serial.println("DeskPet listo. Toca el botón o mueve el MPU6050.");
}

void loop() {
  uint32_t ahora = millis();

  Evento ev = leerTactil(ahora);
  if (ev != NINGUNO) atenderEvento(ev, ahora);
  ev = leerMovimiento(ahora);
  if (ev != NINGUNO) atenderEvento(ev, ahora);

  // Cambiar de postura también cuenta como hacerle caso
  if (mov.postura != posturaAnterior) {
    posturaAnterior = mov.postura;
    ultimaInteraccion = ahora;
  }

  Emocion e = elegirEmocion(ahora);
  // Vuelve a aplicarla si un evento nuevo la ha relanzado (p. ej. otra risa)
  static uint32_t momentaneaAplicada = 0;
  if (e != mostrada || (e == momentanea && momentaneaHasta != momentaneaAplicada)) {
    mostrar(e);
    if (e == momentanea) momentaneaAplicada = momentaneaHasta;
  }

  ojos.update();
}
