# Firmware

Código de la ESP32-C3 (C++ con Arduino sobre PlatformIO).

Pendiente de la fase 0. Estructura prevista:

```
firmware/
├── platformio.ini
└── src/
    ├── main.cpp        Bucle principal
    ├── ojos.*          Dibujo y animación de los ojos
    ├── sensores.*      TTP223 y MPU6050: detección de eventos
    ├── emociones.*     Máquina de estados de emociones
    └── sonido.*        Efectos por I2S (opcional)
```
