# Emociones

Borrador de la máquina de estados. Se ajustará en la fase 4.

| Emoción | Se activa con | Ojos | Dura |
|---|---|---|---|
| Tranquilo | Estado por defecto | Normales, parpadeo y miradas aleatorias | — |
| Contento | Caricia (toque largo) | Arcos hacia arriba (^ ^) | 3 s |
| Sorprendido | Toque corto o golpe suave | Muy abiertos | 1,5 s |
| Enfadado | Golpe fuerte | Cejas inclinadas, ojos estrechos | 3 s |
| Mareado | Sacudida | Espirales o giro | 3 s |
| Asustado | Lo levantan | Pequeños, mirando a los lados | Mientras esté en el aire |
| Protesta | Boca abajo | Cerrados con fuerza (> <) | Mientras siga boca abajo |
| Aburrido | 2 min sin interacción | Medio cerrados, mirando abajo | Hasta que haya interacción |
| Dormido | 5 min sin interacción | Cerrados, "Zzz" | Hasta que lo toquen |

Al terminar cada emoción vuelve a **Tranquilo**.

## Prioridades

Si pasan dos cosas a la vez, gana la de mayor prioridad:

1. Boca abajo
2. Levantado
3. Golpe fuerte / sacudida
4. Toque / caricia / golpe suave
5. Aburrido / dormido
