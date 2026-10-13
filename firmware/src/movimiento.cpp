#include "movimiento.h"
#include <Wire.h>
#include "config.h"

#define MPU_DIR 0x68
const float LSB_POR_G = 4096;  // rango ±8 g

// Umbrales: habrá que ajustarlos con el sensor real dentro de la carcasa
const float UMBRAL_PICO     = 0.6f;   // g: por debajo se ignora (teclear cerca, vibraciones)
const float UMBRAL_FUERTE   = 1.5f;   // g: golpe fuerte
const int   PICOS_SACUDIDA  = 4;      // picos en 1 s = sacudida
const float UMBRAL_LEVANTAR = 1.25f;  // g total mantenido = lo están subiendo
const uint32_t LEVANTAR_MS = 100, POSTURA_MS = 400;

static const char *NOMBRE_POSTURA[] = {"de pie", "inclinado izq.", "inclinado der.", "tumbado", "boca abajo"};

static bool ok = false;
static float gx = 0, gy = 0, gz = 1;  // gravedad (aceleración filtrada)
static Postura actual = DE_PIE, candidata = DE_PIE;
static uint32_t candidataDesde = 0, ultimaLectura = 0, ultimoPico = 0, ultimoGolpe = 0;
static uint32_t picos[PICOS_SACUDIDA] = {0};
static int siguientePico = 0;
static uint32_t subiendoDesde = 0, ultimoLevantado = 0;

static void escribir(uint8_t reg, uint8_t valor) {
  Wire.beginTransmission(MPU_DIR);
  Wire.write(reg);
  Wire.write(valor);
  Wire.endTransmission();
}

void iniciarMovimiento() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(MPU_DIR);
  ok = Wire.endTransmission() == 0;
  if (!ok) {
    Serial.println("MPU6050 no encontrado: solo funcionará el táctil");
    return;
  }
  escribir(0x6B, 0x00);  // despertar
  escribir(0x1C, 0x10);  // acelerómetro a ±8 g
}

static bool leerAcelerometro(float &ax, float &ay, float &az) {
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

static Postura calcularPostura() {
  float g = sqrtf(gx * gx + gy * gy + gz * gz);
  if (g < 0.3f) return actual;  // en caída libre o leyendo mal: no cambiar
  float nz = gz / g, nx = gx / g;
  if (nz < -0.7f) return BOCA_ABAJO;
  if (nz < 0.4f) return TUMBADO;
  if (nz < 0.9f) return nx > 0 ? INCLINADO_DER : INCLINADO_IZQ;
  return DE_PIE;
}

Postura postura() { return actual; }

Evento leerMovimiento(uint32_t ahora) {
  if (!ok || ahora - ultimaLectura < 10) return NINGUNO;  // 100 lecturas por segundo
  ultimaLectura = ahora;
  float ax, ay, az;
  if (!leerAcelerometro(ax, ay, az)) return NINGUNO;

  // La gravedad cambia despacio; un golpe es lo que se aparta de ella
  const float k = 0.1f;
  gx += (ax - gx) * k;
  gy += (ay - gy) * k;
  gz += (az - gz) * k;
  float dx = ax - gx, dy = ay - gy, dz = az - gz;
  float pico = sqrtf(dx * dx + dy * dy + dz * dz);
  float total = sqrtf(ax * ax + ay * ay + az * az);

  // Postura: tiene que mantenerse un rato para contar
  Postura p = calcularPostura();
  if (p != candidata) {
    candidata = p;
    candidataDesde = ahora;
  } else if (p != actual && ahora - candidataDesde >= POSTURA_MS) {
    actual = p;
    Serial.printf("Postura: %s\n", NOMBRE_POSTURA[p]);
  }

  // Levantado: aceleración hacia arriba mantenida estando de pie
  if (total > UMBRAL_LEVANTAR && actual == DE_PIE) {
    if (subiendoDesde == 0) subiendoDesde = ahora;
    if (ahora - subiendoDesde >= LEVANTAR_MS && ahora - ultimoLevantado > 2000) {
      ultimoLevantado = ahora;
      return LEVANTADO;
    }
  } else {
    subiendoDesde = 0;
  }

  // Golpes y sacudidas
  if (pico > UMBRAL_PICO && ahora - ultimoPico > 80) {
    ultimoPico = ahora;
    uint32_t masAntiguo = picos[siguientePico];
    picos[siguientePico] = ahora;
    siguientePico = (siguientePico + 1) % PICOS_SACUDIDA;
    if (masAntiguo != 0 && ahora - masAntiguo < 1000) {
      memset(picos, 0, sizeof(picos));
      return SACUDIDA;
    }
    if (ahora - ultimoGolpe > 300) {
      ultimoGolpe = ahora;
      return pico > UMBRAL_FUERTE ? GOLPE_FUERTE : GOLPE_SUAVE;
    }
  }
  return NINGUNO;
}
