# Laboratorio 3: Programación Concurrente con Pthreads

## 1. Objetivos

* Aplicar los conceptos teóricos de creación (`pthread_create`) y sincronización (`pthread_join`) de hilos POSIX.
* Implementar la paralelización de una aplicación serial existente (Cálculo de $\pi$) para evaluar la mejora en el rendimiento.
* Desarrollar una solución multihilo para un problema de generación de datos (Secuencia de Fibonacci).
* Medir y analizar el impacto del paralelismo mediante el cálculo de métricas de desempeño como **Speedup** y **Eficiencia**.

---

## 2. Prerrequisitos

Se requiere la revisión previa del material teórico de la [Sesión de Laboratorio 4](https://github.com/udea-so/sesiones_lab_2025-2/tree/main/lab3/sesion-4). El estudiante debe estar familiarizado con:
* La distinción conceptual entre concurrencia y paralelismo.
* El modelo de memoria compartida en programas multihilo.
* Las funciones de la API de Pthreads: `pthread_create`, `pthread_join` y `pthread_exit`.
* Métodos para la transferencia de argumentos a hilos y la gestión de valores de retorno.

---

## 3. Instrucciones Generales

1.  **Fork del Repositorio:** Realizar un `fork` de este repositorio a su cuenta personal de GitHub.
2.  **Clonación Local:** Clonar el repositorio *forkeado* a su entorno de desarrollo local.
3.  **Desarrollo de Tareas:** Completar las dos partes del laboratorio creando los archivos fuente solicitados dentro del directorio clonado.
4.  **Formato de Entrega:** El entregable será un único archivo Jupyter Notebook (`analisis.ipynb`) que contenga el análisis de ambas partes, tal como se describe en la sección "Entregable".

---

## 4. Parte 1: Paralelización del Cálculo de $\pi$

### Contexto: Integración Numérica

El archivo `pi.c` calcula el valor de $\pi$ utilizando un método de integración numérica. El algoritmo se basa en la integral definida $\int_0^1 \frac{4}{1+x^2} dx = \pi$.

El código aproxima el área bajo esta curva dividiéndola en `n` rectángulos (método de la regla del punto medio) y sumando sus áreas. El bucle `for` en la función `CalcPi` representa el núcleo computacional de la aplicación:

```c
// extracto de pi.c
double CalcPi(int n)
{
    const double fH   = 1.0 / (double) n;
    double fSum = 0.0;
    double fX;
    int i;

    // Bucle principal para paralelizar
    for (i = 0; i < n; i += 1)
    {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    return fH * fSum;
}
```

### Actividades a Realizar (Parte 1)

1. **Crear `pi_p.c`**: Genere una copia de `pi.c` y nómbrela `pi_p.c`.
2. **Paralelizar `CalcPi`**: Modifique `pi_p.c` para paralelizar el bucle for mediante Pthreads.
   * **Estrategia (Data Parallelism)**: La función main debe ser modificada para recibir el número de hilos ($T$) como argumento de línea de comandos.
   * El rango total de iteraciones (de `0` a `n-1`) debe ser particionado entre los $T$ hilos. Cada hilo será responsable de calcular una **suma parcial** correspondiente a su sub-rango de iteraciones.
   * **Gestión de Resultados Parciales**: Se debe evitar el uso de `mutex` dentro del bucle para no introducir alta contención. En su lugar, cada hilo debe calcular su suma en una variable local y retornar dicho valor parcial al hilo principal (p.ej., vía `pthread_join`).
   * **Sincronización**: El hilo `main` debe crear los $T$ hilos y sincronizar su finalización mediante `pthread_join`.
   * **Resultado Final**: Una vez que todos los hilos han retornado sus sumas parciales, el hilo main debe agregar estos resultados y multiplicar la suma total por `fH` para obtener la aproximación final de $\pi$.
3. **Compilación y Evaluación**: 
   * **Compile la versión serial**: 
   
     ```Bash
     gcc -o pi_s pi.c -lm
     ```

   * **Compile la versión paralela**: 
   
     ```Bash
     gcc -o pi_p pi_p.c -lpthread -lm
     ```

   Incorpore instrumentación para la medición de tiempo (p.ej., `GetTime()`) en ambos archivos para medir exclusivamente el tiempo de ejecución de la función `CalcPi`.

## 5. Parte 2: Generador de Secuencia de Fibonacci

### Contexto

El segundo ejercicio consiste en generar la secuencia de Fibonacci en un hilo de trabajo. La secuencia se define recursivamente como:
* $f_0 = 0$
* $f_1 = 1$
* $f_n = f_{n-1} + f_{n-2}$

### Actividades a Realizar (Parte 2)

1. **Crear `fibonacci.c`**: Cree un nuevo archivo fuente con este nombre.
2. **Implementación del Algoritmo**:
   * El programa recibirá por línea de comandos el número $N$ de elementos de Fibonacci a generar.
   El hilo `main` será responsable de asignar memoria dinámica (p.ej., `malloc`) para un arreglo compartido de tamaño $N$.
   * Posteriormente, `main` instanciará un único **hilo trabajador**. Se debe pasar a este hilo un puntero al arreglo compartido y el valor de $N$.
   * El **hilo trabajador** calculará los $N$ números de la secuencia y los almacenará secuencialmente en el arreglo compartido.
   * El **hilo principal** debe bloquearse (`pthread_join`) hasta que el trabajador complete su ejecución.Tras la finalización del hilo trabajador (confirmada por `pthread_join`), el hilo principal imprimirá la secuencia de Fibonacci contenida en el arreglo compartido.
3. **Compilación**: 

   * **Compile el programa**: 
   
     ```Bash
     gcc -o fibonacci fibonacci.c -lpthread
     ```

   * **Ejecute para verificar**: 
     
     ```Bash
     ./fibonacci 10 (debe imprimir los primeros 10 números).
     ```

## 6. Entregable: `analisis.ipynb`

El entregable consiste en un único Jupyter Notebook (`analisis.ipynb`) que contenga los resultados y el análisis de ambas partes. El notebook debe estar estructurado de la siguiente manera:

### Sección 1: Análisis de $\pi$

1. **Evaluación de $T_s$ (Tiempo Serial)**:  Reporte el tiempo de ejecución de `./pi_s` (con `n = 2000000000`). Este valor será $T_s$.
2. **Evaluación de $T_p$ (Tiempo Paralelo)**:  Ejecute `./pi_p` (con el mismo n) variando el número de hilos ($N = 1, 2, 4, 8, ...$ hasta $2 \times$ el número de núcleos de su CPU) y reporte los tiempos $T_p(N)$.
3. **Tabla de Resultados**: Presente una tabla con las métricas de rendimiento calculadas:

   |N (Hilos)|$T_p$​ (segundos)|Speedup $(T_s​/T_p)$​|Eficiencia ($Speedup/N$)|
   |---|---|---|---|
   |1|...|...|...|
   |2|...|...|...|
   |4|...|...|...|
   |8|...|...|...|
   |16|...|...|...|

4. **Gráfico de Speedup**: Incluya un gráfico de líneas (N Hilos vs. Speedup).

5. **Análisis de Resultados (Parte 1):**
   * Realice una comparación entre el rendimiento de $T_p(1)$ y $T_s$. Explique cualquier discrepancia (overhead).
   * Analice el Speedup máximo alcanzado. ¿Cómo se compara con el número de núcleos físicos de su sistema?
   * Describa la tendencia de la eficiencia a medida que $N$ incrementa y explique las causas de dicho comportamiento.

### Sección 2: Análisis de Fibonacci

1. **Resultados de Ejecución**: Incluya la salida de su programa `./fibonacci 15`.
2. **Análisis del Diseño (Parte 2)**:
   * Describa el mecanismo utilizado para transferir datos (el puntero al arreglo y $N$) del hilo principal al hilo trabajador.
   * Explique el rol de `pthread_join` como mecanismo de sincronización en este problema, asegurando que `main` no acceda a los resultados antes de que sean generados.

## 7. Video de Sustentación

Adicionalmente al Jupyter Notebook, deberá grabar un video corto (máximo 5 minutos) explicando el procedimiento realizado.
* **Contenido del Video**:
  1. **Explicación del Código (Parte 1)**: Muestre brevemente la estrategia de paralelización en `pi_p.c` (cómo dividió el bucle y recolectó los resultados)
  2. **Explicación del Código (Parte 2)**: Muestre cómo implementó el paso del arreglo compartido y la sincronización en `fibonacci.c`.
  3. **Demostración**: Ejecute `pi_s`, `pi_p` (con 1 hilo y $N$ hilos) y `fibonacci`.
  4. **Análisis**: Muestre su gráfico de Speedup y explique brevemente sus conclusiones sobre el rendimiento.
* **Entrega**: 
  * Suba el video a YouTube (puede configurarlo como "No listado").
  * Incluya el enlace al video en la primera celda (celda de texto) de su Jupyter Notebook `analisis.ipynb`.

## 8. Referencias

* Remzi H. Arpaci-Dusseau & Andrea C. Arpaci-Dusseau. *Operating Systems: Three Easy Pieces*.
  * **Capítulo 26**: Threads Intro [[pdf]](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-intro.pdf)
  * **Capítulo 27**: Thread API [[pdf]](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf)
  * **Diapositivas de apoyo**: Interlude: Thread_API [[pdf]](27.Interlude_Thread_API.pdf) 

