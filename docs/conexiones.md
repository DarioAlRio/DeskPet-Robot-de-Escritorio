# Conexiones

Borrador. Se confirmará en la fase 1 con la placa real: los pines de la ESP32-C3 SuperMini pueden variar según el fabricante.

## Pantalla 0,96" (SPI, ST7735)

| Pantalla | ESP32-C3 |
|---|---|
| GND | GND |
| VCC | 3V3 |
| SCL (reloj) | GPIO 4 |
| SDA (datos) | GPIO 6 |
| RES | GPIO 1 |
| DC | GPIO 0 |
| CS | GPIO 7 |
| BLK (retroiluminación) | 3V3 |

## MPU6050 (I2C)

| MPU6050 | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 8 |
| SCL | GPIO 9 |

## TTP223

| TTP223 | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| I/O | GPIO 10 |

## MAX98357A *(opcional, I2S)*

| MAX98357A | ESP32-C3 |
|---|---|
| VIN | 5V |
| GND | GND |
| BCLK | GPIO 2 |
| LRC | GPIO 3 |
| DIN | GPIO 5 |

El altavoz se conecta a los terminales + y − del MAX98357A.

## Notas

- GPIO 8 y 9 también son pines de arranque: si la placa no arranca con el MPU6050 conectado, mover el I2C a otros pines.
- GPIO 20 y 21 son el puerto serie: mejor dejarlos libres para depurar.
