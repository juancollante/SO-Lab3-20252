# Instrucciones paso a paso para ejecutar las pruebas del guion pedagógico

Este documento describe exactamente qué hacer (comandos en orden) para compilar, ejecutar las demos rápidas del video y realizar la batería de experimentos, guardando los tiempos en CSV para el notebook `analisis.ipynb`.

---

## 0) Abrir terminal y situarse en el repo

```bash
cd /mnt/e/UNIVERSIDAD/universidad/Sistemas\ operativos/Laboratorios/SO-Lab3-20252
pwd  # confirma que estás en el directorio correcto
```

## 1) Compilar los programas (recomendado: optimizado)

Compila las tres piezas con optimizaciones. Esto ayuda a obtener tiempos más estables:

```bash
gcc -O2 -march=native -o pi_s pi.c -lm
gcc -O2 -march=native -o pi_p pi_p.c -lpthread -lm
gcc -O2 -march=native -o fibonacci fibonacci.c -lpthread
```

Si tu compilador no soporta `-march=native`, puedes omitirlo.

## 2) Demo rápida para el video (usa `n = 1_000_000`)

Estos son los comandos exactos que puedes ejecutar en la grabación (rápidos):

```bash
# Serial
./pi_s 1000000

# Paralelo con 4 hilos
./pi_p 1000000 4

# Fibonacci (ejemplo corto)
./fibonacci 15
```

Observa y anota las líneas `Time (seconds): ...` que imprimen `pi_s` y `pi_p`.

---

## 3) Preparar y ejecutar la batería de experimentos reales

**Advertencia:** `n = 2000000000` (2e9) tarda mucho. Ejecuta en tu equipo y planifica tiempo. Recomiendo ejecutar cada punto 3–5 repeticiones y tomar la mediana.

### a) Obtener núcleos y definir lista de hilos

```bash
ncores=$(nproc --all)
echo "Núcleos disponibles (nproc) = $ncores"
# lista de hilos: 1,2,4,8,... hasta 2*ncores
```

### b) Usar el script automatizado (recomendado)

He incluido un script `run_experiments.sh` en el repo. Para usarlo:

1. Dale permiso de ejecución:

```bash
chmod +x run_experiments.sh
```

2. Edita el script si quieres cambiar `N` o `REPS` (valores por defecto están en el script). Por defecto el script usa `N=2000000000` y `REPS=3`.

3. Ejecuta:

```bash
./run_experiments.sh
```

El script genera `pi_times_real.csv` con columnas `threads,t_sec` y mostrará salida por pantalla.

> Si quieres probar más rápido antes de lanzar N grande, edita `run_experiments.sh` y pon `N=10000000` (1e7).

---

## 4) Contenido del `run_experiments.sh` (para referencia)

Si prefieres crear el script manualmente, este es su contenido completo (ya puesto en `run_experiments.sh`):

```bash
#!/usr/bin/env bash
set -euo pipefail

# Configuración (edita aquí si quieres)
N=2000000000     # número de intervals (modificar para pruebas rápidas)
REPS=3           # repeticiones por punto
OUT=pi_times_real.csv

# Calcula la lista de hilos: 1,2,4,8,... hasta 2*nproc
ncores=$(nproc --all)
max_threads=$(( ncores * 2 ))

threads_list=(1)
t=1
while [ $t -lt $max_threads ]; do
  t=$(( t * 2 ))
  threads_list+=( $t )
done
if [ "${threads_list[-1]}" -lt "$max_threads" ]; then
  threads_list+=( $max_threads )
fi

echo "threads,t_sec" > "$OUT"

for T in "${threads_list[@]}"; do
  echo "Running threads=$T"
  # archivo temporal con los tiempos de las REPS ejecuciones
  tmpfile=$(mktemp)
  for r in $(seq 1 $REPS); do
    # Ejecuta y captura la salida
    out=$(./pi_p "$N" "$T" 2>&1)
    echo "$out"
    # Extrae la línea con Time (seconds): y guarda solo el número
    tline=$(echo "$out" | grep "Time (seconds)" || true)
    if [ -z "$tline" ]; then
      echo "Warning: no time line captured for threads=$T (rep $r)" >&2
      continue
    fi
    tval=$(echo "$tline" | sed 's/.*: //' )
    echo "$tval" >> "$tmpfile"
  done
  # Calcula mediana con python
  median=$(python3 - <<PY
import sys
vals = [float(x.strip()) for x in open('$tmpfile') if x.strip()]
vals.sort()
if len(vals)==0:
    print('0')
else:
    n=len(vals)
    if n%2==1:
        print(vals[n//2])
    else:
        print((vals[n//2-1]+vals[n//2])/2.0)
PY
)
  echo "$T,$median" >> "$OUT"
  rm -f "$tmpfile"
done

echo "Results written to $OUT"
```

---

## 5) Convertir `pi_times_real.csv` en tabla y gráfico (ejemplo rápido por terminal)

```bash
python3 - <<PY
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('pi_times_real.csv')
T_s = df.loc[df['threads']==1, 't_sec'].values[0]
df['speedup'] = T_s / df['t_sec']
df['efficiency'] = df['speedup'] / df['threads']
print(df)
# Opcional: guardar tabla con speedup/efficiency
# df.to_csv('pi_summary.csv', index=False)
PY
```

Para graficar, abre `analisis.ipynb` y ejecuta la celda de gráficos (ya incluida en el notebook).

---

## 6) Sugerencias para medidas estables

- Compila con `-O2 -march=native`.
- Fija la afinidad a un núcleo con `taskset` si necesitas repetir en un único núcleo:

```bash
taskset -c 0 ./pi_s 1000000
```

- Para reducir variabilidad de frecuencias, si tienes `cpupower` puedes poner el governor en `performance` (requiere sudo):

```bash
sudo cpupower frequency-set -g performance
```

- Repite experimentos varias veces y usa la mediana.

---

## 7) Problemas comunes

- "No se imprime Time (seconds)": recompila el binario `pi_p` (asegura que estás ejecutando la versión actual).
- "Ejecución muy lenta": usa `N` más pequeño para pruebas rápidas o revisa carga del sistema.
- "Overflow en Fibonacci": `unsigned long long` soporta hasta F93; para N>93 necesitarás big integers.

---

Si quieres, puedo también:
- A) Añadir `run_experiments.sh` al repositorio (ya lo hice) y marcarlo ejecutable si me lo pides.
- B) Ejecutar aquí una prueba corta con `N=10000000` y generar un `pi_times_real.csv` de ejemplo para que veas el flujo.

Indica si quieres que haga A o B (o ambas). 
