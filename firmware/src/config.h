// Pines y ajustes generales. Conexiones explicadas en docs/conexiones.md.
#pragma once
#include <Arduino.h>

// Pantalla (SPI)
#define PIN_CS  7
#define PIN_DC  0
#define PIN_RST 1

// Sensores
#define PIN_TOQUE 10  // TTP223
#define PIN_SDA   8   // MPU6050
#define PIN_SCL   9

// Pantalla redonda de 240×240. En Wokwi se usa una ILI9341 de 240×320 y se centra.
#define LADO 240
#ifdef PANTALLA_WOKWI
#define OFFSET_Y 40
#else
#define OFFSET_Y 0
#endif

// Tiempo sin hacerle caso hasta aburrirse y dormirse.
// En el simulador se acortan para poder probarlo.
#ifdef MODO_PRUEBA
const uint32_t T_ABURRIDO = 20000, T_DORMIDO = 40000;
#else
const uint32_t T_ABURRIDO = 2 * 60000UL, T_DORMIDO = 5 * 60000UL;
#endif
