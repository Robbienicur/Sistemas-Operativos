# La Casa de Hojas

Crucigrama hecho en C para la materia de Sistemas Operativos. La idea es resolverlo, pero hay un detalle: cada cierto tiempo una de las palabras que no has adivinado se cambia por otra distinta (con su pista nueva), así que toca apurarse.

## Cómo compilar

Necesitas gcc y la libreria de pthread (en Linux y Mac vienen por defecto). Desde la carpeta del proyecto:

```
gcc main.c -o casadehojas -pthread
```

## Cómo correrlo

```
./casadehojas
```

Te aparecen las instrucciones del juego, le das ENTER y arranca.

## Cómo se juega

El tablero te muestra las casillas vacias con `[_]` y al lado de cada palabra hay un numero. Para adivinar:

1. Escribe el numero de la palabra que quieres intentar.
2. Te aparece la pista y cuantas letras tiene.
3. Escribes tu respuesta. Da igual mayusculas o minusculas.

Si aciertas, las letras quedan puestas en el tablero y esa palabra ya no la cambia el cronometro. Si fallas, simplemente sigues con las demas.

## El truco del cronometro

En segundo plano hay un hilo que cada **45 a 60 segundos** (aleatorio) elige una palabra que todavia no hayas adivinado y la cambia por su alternativa. Te avisa con un beep y un mensaje de notificacion, y el tablero se vuelve a dibujar con las pistas actualizadas. Las palabras que ya tienes adivinadas no se tocan.

Por ejemplo: empezaste con QUESO como pista "Lacteo amarillo" y de repente cambia a FRENO con pista "Dispositivo para reducir la velocidad de un coche". Si ya habias adivinado QUESO, no pasa nada con esa.

## Cuando termina

Cuando tienes todas las palabras adivinadas el juego se acaba y muestra el tablero completo.
