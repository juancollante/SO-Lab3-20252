# Guion pedagógico — Parte 1 (paralelización de π)

Este archivo contiene el guion algorítmico para la sección 0:20–1:50 del video. Está pensado para que lo leas en voz alta ante el docente. Hay dos variantes:

- Versión de lectura rápida: las líneas que debes pronunciar están marcadas con `>>>`.
- Versión HTML: misma lectura pero con las frases a decir envueltas en `<span style="color:red">...</span>` para que se muestren en rojo en notebooks o editores que rendericen HTML.

---

## Versión (lectura rápida)

0:00–0:20 — Presentación (20 s)

>>> Hola, soy Juan Pablo Collante, curso Sistemas Operativos. En este video explicaré la estrategia algorítmica que usé para paralelizar el cálculo de π y cómo sincronizo un hilo trabajador en el ejercicio de Fibonacci. Mostraré una demo corta y comentaré la medición de tiempos.

0:20–1:10 — Explicación Parte 1 (pi_p.c) — enfoque algorítmico (50 s)

1) Propósito del particionado (10 s)

>>> El objetivo del particionado es convertir una única tarea grande —el barrido de n puntos— en T sub-tareas independientes que puedan ejecutarse en paralelo sin solaparse. Cada sub-tarea recibe un subconjunto disjunto del dominio de iteración para garantizar que no haya duplicación ni huecos en el cálculo.

2) Qué representan `chunk`, `start` y `end` en términos algorítmicos (15 s)

>>> Algorítmicamente, `chunk` es el tamaño de cada subdominio: cuántas iteraciones asigno a cada trabajador. `start` y `end` son los límites del subdominio para el hilo j: definen la porción contigua del dominio global que ese hilo procesará. El ajuste del último subdominio (donde end toma el resto) es una corrección clásica para cubrir correctamente el dominio cuando n no es múltiplo de T; esto asegura cobertura completa y preserva corrección.

3) Reducción local y por qué es importante (15 s)

>>> Cada hilo realiza una reducción local: acumula su propio subtotal en una variable privada. Esta estrategia evita sincronización fina dentro del bucle—es decir, no se accede a una estructura compartida en cada iteración—lo que reduce la contención y mejora el rendimiento. Solo al final se realiza una reducción global: sumar las parciales ya calculadas por cada hilo.

4) Punto de recolección y sincronización (10 s)

>>> El hilo principal actúa como reductor final. Usa `pthread_join` para esperar a cada trabajador y, al recibir su subtotal, acumula en una suma global. Así se obtiene la integral total. `pthread_join` no solo sincroniza término de ejecución, sino que define el punto donde se realiza la reducción atómica desde la visión algorítmica del problema.

5) Qué medimos y por qué (5 s)

>>> Medimos solo la sección de cálculo paralela con un temporizador monotónico para cuantificar el rendimiento del núcleo computacional, evitando incluir setup u operaciones de E/S en la medición.



### 1:10–1:50 — Demostración corta Parte 1 (40 s)

>>> Ahora realizo una demostración breve con `n` pequeño para mostrar comportamiento y tiempos en pantalla: primero la versión serial y luego la paralela con 4 hilos.

> Ejecuta: `./pi_s 1000000`

>>> Observa la salida: la aproximación a π y la línea `Time (seconds): ...`. Este valor será nuestro tiempo de referencia para comparar con la versión paralela.

> Ejecuta: `./pi_p 1000000 4`

>>> Observa `Threads: 4` y el tiempo reportado. Compara ambos tiempos: si el paralelo es más rápido, ahí tienes un speedup; si no, recuerda que con `n` pequeño el overhead de crear/gestionar hilos puede enmascarar la ventaja del paralelismo. Para resultados definitivos usaré `n = 2000000000` en mi equipo y reportaré las medianas.

### 1:50–2:30 — Explicación Parte 2 (fibonacci.c) — 40 s

- Visual: editor con `fibonacci.c` abierto, resalta `main`, la estructura `FibArgs`, la llamada a `pthread_create` y `pthread_join`.

>>> Algorítmicamente, la idea es delegar en un trabajador la tarea de construir la secuencia: el `main` reserva un bloque de memoria para la salida y pasa al trabajador un descriptor (puntero + N). El trabajador escribe directamente en ese bloque, evitando copias extra. Finalmente `main` espera al trabajador con `pthread_join` y solo entonces lee e imprime los resultados.

>>> Esta estrategia minimiza comunicaciones y sincronizaciones: hay un único escritor (el trabajador) y el lector (`main`) solo accede después de la sincronización por `join`, por lo que no se requieren locks adicionales.

### 2:30–3:00 — Demostración Parte 2 — 30 s

> Ejecuta: `./fibonacci 15`

>>> Muestra la salida en pantalla: `0 1 1 2 3 5 8 13 21 34 55 89 144 233 377`. Explica brevemente que la salida coincide con los primeros 15 términos de Fibonacci y que la implementación evita condiciones de carrera por el uso de `pthread_join`.

### 3:00–4:15 — Análisis de resultados (Speedup) — 75 s

>>> Ahora muestro el gráfico de Speedup en el notebook. Recuerda que Speedup se define como `S(N) = T_s / T_p(N)` y la eficiencia como `E(N) = S(N) / N`.

>>> Primero comparo `T_p(1)` frente a `T_s`: idealmente deberían ser similares, pero `T_p(1)` puede ser algo mayor por overhead de gestión de hilos; esto ilustra la importancia de que n sea suficientemente grande para amortizar ese overhead.

>>> Luego muestro el speedup máximo observado y lo contrasto con el número de núcleos físicos (en mi máquina son 8). Si el speedup supera ligeramente 8, lo explico por hyperthreading o por variaciones en la medición; pero en general la ganancia real tiende a limitarse por ancho de banda de memoria y por la fracción serial del algoritmo (Amdahl).

>>> Finalmente explico por qué la eficiencia cae con N: aumentan overheads relacionados con creación de hilos, mayor contention en caches y en el bus de memoria, y la parte estrictamente serial del algoritmo impide escalado perfecto.

### 4:15–4:50 — Conclusión y cierre — 35 s

>>> Resumen: la paralelización mediante particionado del dominio y reducciones locales es una estrategia efectiva para este problema; para obtener resultados reproducibles conviene ejecutar varias repeticiones y usar la mediana. La implementación de Fibonacci demuestra cómo pasar un puntero compartido y sincronizar la lectura con `pthread_join`.

>>> En el notebook incluyo scripts y pasos para reproducir los experimentos en su equipo y un CSV con los tiempos medidos; revisen esos archivos para ver los números reales.

### 4:50–5:00 — Pantallazo final con enlace al notebook (opcional)

>>> Cierro mostrando en pantalla el enlace al notebook y agradezco el tiempo del revisor.

---

Fin del guion.

