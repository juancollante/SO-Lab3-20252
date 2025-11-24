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
