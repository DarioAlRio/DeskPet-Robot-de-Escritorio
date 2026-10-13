# Material

Precios de AliExpress España consultados el 13/10/2026. Revisa la variante antes de comprar: muchos anuncios muestran en la búsqueda el precio de su opción más barata.

| Componente | Precio | Variante | Enlace |
|---|---|---|---|
| ESP32-C3 SuperMini con pines soldados | 3,09 € | "ESP32-C3 welded" | [AliExpress](https://es.aliexpress.com/item/1005012631245655.html) |
| Pantalla IPS 0,96" 80×160 (ST7735) | 1,06 € | 0,96" IPS | [AliExpress](https://es.aliexpress.com/item/1005004186179829.html) |
| Sensor táctil TTP223 | 0,98 € | — | [AliExpress](https://es.aliexpress.com/item/1005010044065554.html) |
| MPU6050 (GY-521) | 1,80 € | — | [AliExpress](https://es.aliexpress.com/item/1005007989748804.html) |
| Amplificador MAX98357A *(opcional)* | 0,80 € | — | [AliExpress](https://es.aliexpress.com/item/1005008455683459.html) |
| Microaltavoz con caja 8 Ω *(opcional)* | 1,17 € | 2030 8 Ω | [AliExpress](https://es.aliexpress.com/item/1005006144805292.html) |
| Cables Dupont (40 uds) | 1,04 € | Según tipo | [AliExpress](https://es.aliexpress.com/item/1005008318568091.html) |

## Por qué estos componentes

- **ESP32-C3 y no S3:** dibujar dos ojos en una pantalla pequeña necesita poca potencia. La C3 es más pequeña y barata.
- **TTP223:** la C3 no tiene pines táctiles; este módulo detecta el dedo a través de la carcasa.
- **MPU6050:** el táctil solo detecta contacto; el acelerómetro distingue un toque suave de un golpe, una sacudida o que lo pongan boca abajo.
- **MAX98357A + microaltavoz con caja:** la ESP32 no puede mover un altavoz directamente; el amplificador recibe el sonido por I2S. La caja cerrada hace que suene mejor un altavoz tan pequeño.
