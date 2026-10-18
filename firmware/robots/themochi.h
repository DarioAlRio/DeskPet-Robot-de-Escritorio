// The Mochi: reproduce las animaciones del firmware The Mochi (huykhong.com)
// con su mismo comportamiento, en lugar de la máquina de emociones:
//   - al encender pone la intro
//   - en reposo repite la cara por defecto y, cada PAUSA_MS, pone una animación
//     al azar de la lista y vuelve a la cara por defecto
//   - toque, doble toque y toque largo ponen cada uno su animación
//
// Animaciones y nombres sacados de The Mochi v4.3.2, con permiso de Dasai y
// Huy Khong. Equivalen a sus ajustes gif_intro, gif_default, gif_touch,
// gif_double_touch, gif_long_touch, gif_delay y gif_speed. Sus valores de
// fábrica no se leen en el firmware: intro y defecto son los que aparecen
// primero en él; el resto están elegidos a mano. Se pueden cambiar.
#pragma once
#include "mochi.h"  // medidas y color de la cara (cara.cpp los necesita)

#define MODO_THEMOCHI

const char *const ANIM_INTRO = "intro_3";
const char *const ANIM_DEFECTO = "mac_dinh";  // "por defecto" en vietnamita
const char *const ANIM_TOQUE = "happy_2";
const char *const ANIM_DOBLE_TOQUE = "leu_leu";
const char *const ANIM_TOQUE_LARGO = "love";

const uint32_t PAUSA_MS = 5000;  // gif_delay: tiempo en reposo entre animaciones
const float VELOCIDAD = 1.0;     // gif_speed: 2 = el doble de rápido

// Las que nunca salen al azar, además de ANIM_INTRO y ANIM_DEFECTO
const char *const NO_ALEATORIAS[] = {"LB_intro"};
