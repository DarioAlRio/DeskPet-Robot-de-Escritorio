# Convierte los GIF de animaciones/themochi/ en robots/themochi_animaciones.h
#
# Cada fotograma es de 128×64 a 1 bit (fila a fila, bit más alto a la izquierda).
# Para que quepa en la ESP32-C3 se guarda solo lo que cambia respecto al
# fotograma anterior (XOR) comprimido por tramos:
#   byte c < 0x80  → saltar c bytes (no cambian)
#   byte c >= 0x80 → copiar los siguientes (c & 0x7F) bytes haciendo XOR
#
# Uso (desde firmware/):  python scripts/convertir_animaciones.py

import json
import os
import sys

from PIL import Image, ImageSequence

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORIGEN = os.path.join(RAIZ, "..", "animaciones", "themochi")
DESTINO = os.path.join(RAIZ, "robots", "themochi_animaciones.h")
ANCHO, ALTO = 128, 64
BYTES = ANCHO * ALTO // 8


def fotogramas(ruta):
    gif = Image.open(ruta)
    escala = gif.width // ANCHO
    for f in ImageSequence.Iterator(gif):
        img = f.convert("L")
        if escala > 1:
            img = img.resize((ANCHO, ALTO), Image.NEAREST)
        bits = img.point(lambda v: 255 if v > 127 else 0).convert("1").tobytes()
        yield bits, f.info.get("duration", 100)


def comprimir(cambios):
    sal = bytearray()
    i = 0
    while i < len(cambios):
        j = i
        if cambios[i] == 0:
            while j < len(cambios) and cambios[j] == 0 and j - i < 127:
                j += 1
            sal.append(j - i)
        else:
            while j < len(cambios) and cambios[j] != 0 and j - i < 127:
                j += 1
            sal.append(0x80 | (j - i))
            sal += cambios[i:j]
        i = j
    return sal


def main():
    indice = json.load(open(os.path.join(ORIGEN, "indice.json"), encoding="utf-8"))
    partes, tabla, total = [], [], 0
    for n, anim in enumerate(indice):
        nombre = anim["nombre"]
        datos, ms = bytearray(), []
        anterior = bytes(BYTES)
        for bits, duracion in fotogramas(os.path.join(ORIGEN, nombre + ".gif")):
            datos += comprimir(bytes(a ^ b for a, b in zip(bits, anterior)))
            ms.append(duracion)
            anterior = bits
        total += len(datos)
        hexa = ",".join("0x%02X" % b for b in datos)
        partes.append(f"static const uint16_t ms_{n}[] = {{{','.join(map(str, ms))}}};\n"
                      f"static const uint8_t datos_{n}[] PROGMEM = {{{hexa}}};\n")
        tabla.append(f'  {{"{nombre}", {len(ms)}, ms_{n}, datos_{n}}},')

    with open(DESTINO, "w", encoding="utf-8", newline="\n") as f:
        f.write("// Generado por scripts/convertir_animaciones.py a partir de animaciones/themochi/.\n"
                "// No editar a mano.\n#pragma once\n#include <Arduino.h>\n\n"
                "struct Animacion {\n  const char *nombre;\n  uint16_t fotogramas;\n"
                "  const uint16_t *ms;     // duración de cada fotograma\n"
                "  const uint8_t *datos;   // fotogramas comprimidos (ver el script)\n};\n\n")
        f.write("".join(partes))
        f.write("\nstatic const Animacion ANIMACIONES[] = {\n" + "\n".join(tabla) + "\n};\n"
                "static const int NUM_ANIMACIONES = sizeof(ANIMACIONES) / sizeof(ANIMACIONES[0]);\n")
    print(f"{len(indice)} animaciones, {total // 1024} KB -> {DESTINO}")


if __name__ == "__main__":
    sys.exit(main())
