# Plan del proyecto

Cada fase tiene un objetivo comprobable. No se pasa a la siguiente sin cumplir el criterio de "hecho".

## Fase 0 · Material y entorno

- Comprar los componentes de [material.md](material.md).
- Instalar VS Code + PlatformIO (o Arduino IDE) con soporte para ESP32-C3.
- Programar la placa con un ejemplo de parpadeo de LED.

**Hecho cuando:** la ESP32-C3 se programa desde el ordenador y muestra mensajes por el monitor serie.

## Fase 1 · Ojos animados

- Conectar la pantalla redonda 1,28" y mostrar una imagen de prueba.
- Dibujar dos ojos (rectángulos redondeados) con la librería TFT_eSPI o Adafruit GFX.
- Animaciones básicas: parpadeo, mirar a izquierda y derecha, mirar arriba y abajo.
- Dibujar en un búfer en memoria (sprite) para que la animación no parpadee.
- Parpadeos y miradas a intervalos aleatorios.

**Hecho cuando:** los ojos se animan solos de forma fluida (≥ 30 fps) y sin parpadeos de pantalla.

## Fase 2 · Reacción al toque

- Conectar el TTP223 y leer el toque.
- Distinguir toque corto y caricia (toque largo).
- Toque corto → sorpresa; caricia → ojos contentos.
- Comprobar que funciona a través de 2–3 mm de plástico.

**Hecho cuando:** el robot reacciona en menos de 200 ms a un toque y distingue toque de caricia.

## Fase 3 · Golpes, sacudidas y orientación

- Conectar el MPU6050 por I2C y leer aceleración y giro.
- Detectar: golpe suave, golpe fuerte, sacudida, boca abajo y que lo levanten.
- Ajustar umbrales para evitar falsos positivos (por ejemplo, al teclear en la mesa).

**Hecho cuando:** los cinco eventos se detectan correctamente en 9 de cada 10 pruebas.

## Fase 4 · Emociones y comportamiento

- Máquina de estados de emociones (ver [emociones.md](emociones.md)).
- Transiciones suaves entre expresiones.
- Aburrimiento y sueño si no hay interacción durante un rato; despertar al tocarlo.

**Hecho cuando:** cada evento produce la emoción esperada y el robot vuelve solo al estado tranquilo.

## Fase 5 · Sonido *(opcional)*

- Conectar el MAX98357A y el microaltavoz por I2S.
- Efectos cortos para cada emoción (pitidos, "risa", queja).

**Hecho cuando:** cada reacción suena sin cortar la animación de los ojos.

## Fase 6 · Carcasa

- Diseñar la carcasa en 3D con hueco para la pantalla y el táctil por dentro.
- Montaje final y fotos o GIF para el README.

**Hecho cuando:** todo cabe en la carcasa, se alimenta por USB-C y funciona igual que en la protoboard.
