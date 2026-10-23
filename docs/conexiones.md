# Conexiones

Borrador. Se confirmará en la fase 1 con la placa real: los pines de la ESP32-C3 SuperMini pueden variar según el fabricante.

## Pantalla redonda 1,28" (SPI, GC9A01)

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

Algunos módulos GC9A01 no traen pin BLK; en ese caso la retroiluminación va siempre encendida.

## Pantalla OLED 0,96" (I2C, SSD1306) *(alternativa a la redonda)*

Rectangular, 128×64, 4 pines. Comparte los dos cables del I2C con el MPU6050.
En PlatformIO se usan los entornos `<robot>-oled` (por ejemplo `themochi-oled`).

| OLED | ESP32-C3 |
|---|---|
| GND | GND |
| VCC | 3V3 |
| SCL | GPIO 9 |
| SDA | GPIO 8 |

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
