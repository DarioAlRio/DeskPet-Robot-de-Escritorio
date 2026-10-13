# Emociones

Borrador de la máquina de estados. Se ajustará en la fase 4.

## Estilo

Todas las caras siguen las [imágenes de diseño](img): **ojos y boca rojos sobre fondo negro**, en una pantalla redonda de 240×240. Por defecto el personaje es travieso y "malvado", con ojos afilados y una sonrisa de dientes en zigzag. El rojo puede cambiar de intensidad o parpadear según la emoción.

## Caras

| Emoción | Se activa con | Cara | Dura |
|---|---|---|---|
| Normal (amenazante) | Estado por defecto | Ojos afilados, sonrisa en zigzag; parpadeo y miradas aleatorias | — |
| Sonriente (lado oscuro) | Caricia (toque largo) | Ojos curvados, sonrisa en zigzag más ancha | 3 s |
| Burlón | Doble toque | Guiña un ojo y saca la sonrisa torcida | 2 s |
| Sorprendido | Toque corto o golpe suave | Ojos redondos y boca en "o" | 1,5 s |
| Irritado | 3 toques seguidos en pocos segundos | Ceño fruncido, boca torcida, símbolo de enfado | 3 s |
| Enfadado | Golpe fuerte | Ojos estrechos, cejas muy inclinadas, boca hacia abajo | 3 s |
| Furioso | Seguir golpeándolo estando enfadado | Como enfadado, pero el rojo parpadea y tiembla | 4 s |
| Mareado | Sacudida | Ojos en espiral, boca ondulada | 3 s |
| Asustado | Lo levantan | Ojos pequeños mirando a los lados, boca temblorosa | Mientras esté en el aire |
| Protesta | Boca abajo | Ojos cerrados con fuerza (> <) y dientes apretados | Mientras siga boca abajo |
| Curioso | Lo inclinan a un lado | Ojos grandes que miran hacia el lado inclinado | Mientras siga inclinado |
| Tumbado | Lo dejan de lado | Ojos medio cerrados, de lado | Mientras siga de lado |
| Modo hacker | 5 toques seguidos (secreto) | Cara con glitch: líneas, desplazamientos, "??" | 5 s |
| Aburrido | 2 min sin interacción | Ojos medio cerrados mirando abajo, boca plana | Hasta que haya interacción |
| Dormido | 5 min sin interacción | Ojos cerrados en arco y "Zzz" | Hasta que lo toquen |
| Despertando | Tocarlo mientras duerme | Abre un ojo, luego el otro, y bosteza | 2 s |
| Arranque | Al encenderlo | Glitch que se estabiliza en la cara normal | 2 s |

Al terminar cada emoción vuelve a **Normal**. Las emociones que duran "mientras…" terminan en cuanto deja de pasar lo que las causó.

## Prioridades

Si pasan dos cosas a la vez, gana la de mayor prioridad:

1. Boca abajo
2. Levantado
3. Golpe fuerte / sacudida (enfadado, furioso, mareado)
4. Tumbado / inclinado
5. Toques (modo hacker > irritado > burlón > sonriente > sorprendido)
6. Aburrido / dormido

## Ideas para más adelante

- **Pantalla en negro de noche**, con la hora sacada de internet (NTP) por WiFi.
- **Memoria de humor:** si lo tratas mal muchas veces seguidas, pasa un rato más gruñón.
- **Cara al pasar la mano por delante:** necesitaría un sensor de distancia extra.
