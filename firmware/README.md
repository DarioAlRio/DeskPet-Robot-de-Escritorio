# Firmware

Código de la ESP32-C3 (C++ con Arduino sobre PlatformIO).

```
firmware/
├── platformio.ini   Entornos: deskpet (robot real) y wokwi (simulador)
├── wokwi.toml       Configuración del simulador Wokwi para VS Code
├── diagram.json     Circuito del simulador
├── src/
│   ├── main.cpp        Bucle principal
│   ├── config.h        Pines y ajustes
│   ├── eventos.h       Lista de eventos de los sensores
│   ├── tactil.*        TTP223: toque, caricia, toques seguidos
│   ├── movimiento.*    MPU6050: golpes, sacudidas, levantado, postura
│   └── emociones.*     Máquina de emociones y ojos (RoboEyes)
└── experimentos/    Pruebas que no forman parte del firmware
```

## Requisitos

- [VS Code](https://code.visualstudio.com) con la extensión **PlatformIO IDE**.
- Para simular: la extensión **Wokwi Simulator** (pide activar una licencia gratuita la primera vez).

## Cargar en el robot

1. Abre la carpeta `firmware/` en VS Code.
2. Conecta la ESP32-C3 por USB-C.
3. En la barra de PlatformIO (abajo), elige el entorno `env:deskpet` y pulsa **Upload** (→).
4. Pulsa **Serial Monitor** (🔌) para ver los eventos y emociones.

Si la placa no aparece, mantén pulsado **BOOT** mientras la conectas.

## Probar en el simulador

1. Elige el entorno `env:wokwi` y pulsa **Build** (✓).
2. Abre `diagram.json` o ejecuta **Wokwi: Start Simulator** (F1).
3. El botón rojo es el sensor táctil. Haz clic en el MPU6050 durante la simulación para moverlo:
   - `az = -1 g`: boca abajo · `ax = ±1, az = 0`: tumbado · `ax = ±0,5 g`: inclinado
   - Cambios rápidos de un eje: golpe o sacudida · `az > 1,3 g`: levantado

En el entorno `wokwi` el robot se aburre a los 20 s y se duerme a los 40 s para poder probarlo; en el robot real son 2 y 5 minutos.
