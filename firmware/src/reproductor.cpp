#include "config.h"
#include ROBOT
#ifdef MODO_THEMOCHI

#include "reproductor.h"
#include "cara.h"
#include "themochi_animaciones.h"

static const Animacion *actual = nullptr;
static bool enBucle = false;          // la cara por defecto se repite; las demás van una vez
static uint16_t fotograma = 0;
static uint32_t siguienteFotograma = 0, siguienteAleatoria = 0;
static const uint8_t *lectura = nullptr;  // por dónde va leyendo los datos comprimidos
static uint8_t imagen[128 * 64 / 8];

static const Animacion *buscar(const char *nombre) {
  for (int i = 0; i < NUM_ANIMACIONES; i++)
    if (strcmp(ANIMACIONES[i].nombre, nombre) == 0) return &ANIMACIONES[i];
  Serial.printf("No existe la animación %s\n", nombre);
  return &ANIMACIONES[0];
}

static void poner(const Animacion *a, bool bucle, uint32_t ahora) {
  actual = a;
  enBucle = bucle;
  fotograma = 0;
  lectura = a->datos;
  memset(imagen, 0, sizeof(imagen));
  siguienteFotograma = ahora;
  Serial.printf("Animación: %s\n", a->nombre);
}

// Aplica los cambios del siguiente fotograma sobre la imagen
static void descomprimir() {
  size_t i = 0;
  while (i < sizeof(imagen)) {
    uint8_t c = pgm_read_byte(lectura++);
    if (c < 0x80) {
      i += c;
    } else {
      for (int n = c & 0x7F; n > 0; n--) imagen[i++] ^= pgm_read_byte(lectura++);
    }
  }
}

static bool aleatoriaValida(const Animacion *a) {
  for (const char *n : NO_ALEATORIAS)
    if (strcmp(a->nombre, n) == 0) return false;
  return true;
}

static void ponerDefecto(uint32_t ahora) {
  poner(buscar(ANIM_DEFECTO), true, ahora);
  siguienteAleatoria = ahora + PAUSA_MS;
}

void iniciarReproductor() {
  iniciarCara();
  poner(buscar(ANIM_INTRO), false, millis());
}

void atenderEventoReproductor(Evento ev, uint32_t ahora) {
  Serial.printf("Evento: %s\n", nombreEvento(ev));
  switch (ev) {
    case TOQUE:       poner(buscar(ANIM_TOQUE), false, ahora); break;
    case DOBLE_TOQUE: poner(buscar(ANIM_DOBLE_TOQUE), false, ahora); break;
    case CARICIA:     poner(buscar(ANIM_TOQUE_LARGO), false, ahora); break;
    default: break;
  }
}

void actualizarReproductor(uint32_t ahora) {
  // En reposo, de vez en cuando una animación al azar
  if (enBucle && ahora >= siguienteAleatoria) {
    const Animacion *a;
    do a = &ANIMACIONES[random(NUM_ANIMACIONES)]; while (!aleatoriaValida(a));
    poner(a, false, ahora);
  }
  if (ahora < siguienteFotograma) return;

  if (fotograma >= actual->fotogramas) {
    if (enBucle) poner(actual, true, ahora);
    else ponerDefecto(ahora);
  }
  descomprimir();
  mostrarFotograma(imagen);
  siguienteFotograma = ahora + actual->ms[fotograma] / VELOCIDAD;
  fotograma++;
}

#endif
