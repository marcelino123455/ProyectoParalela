#!/bin/bash

# check if the ./data exists
if [ ! -d "data" ]; then
  echo "There is no data, running data.py"
  python data.py
  echo "Data generation complete"
else
  echo "Data already exists."
fi

processes=(1 2 4 8 16 32)

echo "Reading N values from ./data..."
N_VALUES=($(ls data | grep -v '^18$'))
echo "N_VALUES detected: ${N_VALUES[@]}"

# N_VALUES=("18")

for n in "${N_VALUES[@]}"; do
  echo "Entering quickSort${n}..."
  cd "quickSort" || exit 1

  echo "----------------------------------------"
  echo "Compiling quicksort_mpi for N=${n} ..."
  g++ -o sortSecuencial sortSecuencial.cpp 

  DATA_FILE="../data/${n}/chars.txt"

  if [ ! -f "$DATA_FILE" ]; then
    echo "ERROR: File $DATA_FILE does not exist"
    exit 1
  fi

  echo "Running tests for N=${n}"
  echo "Using input file: $DATA_FILE"
  echo "----------------------------------------"

  ./sortSecuencial "${n}"
  echo "----------------------------------------"
  
  echo "[Ending ${N}]/saliendig  XD quickSort ${n}..."
  cd ".." 

done
