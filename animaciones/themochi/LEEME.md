# Animaciones de The Mochi

71 animaciones de 128×64 sacadas del firmware **The Mochi v4.3.2** (https://themochi.huykhong.com),
basado en las expresiones de **Dasai Mochi** (https://dasai.com.au).

Se usan en DeskPet con permiso de Dasai y de Huy Khong (uso libre), concedido en octubre de 2026.

- `*.gif`: cada animación, ampliada ×2 para verla mejor, con sus tiempos originales.
- `indice.json`: nombre, número de fotogramas y duración total de cada una.

Para usarlas en el robot: `python firmware/scripts/convertir_animaciones.py` genera
`firmware/robots/themochi_animaciones.h`, y el entorno `themochi` de PlatformIO las reproduce.
