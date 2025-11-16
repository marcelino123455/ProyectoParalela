#!/bin/bash

# check if the ./data exists
if [ ! -d "data" ]; then
  echo "There is no data, running data.py"
  python data.py
  echo "Data generation complete"
else
  echo "Data already exists."
fi

K_VALUES=(1 2 4 7 8 10)
processes=()
for k in "${K_VALUES[@]}"; do
  processes+=($((k * k)))
done

# N_VALUES=("78400" "235200" "156800" "6350400" "2116800" "705600" "19051200" "57153600")

N_VALUES=("18")

for n in "${N_VALUES[@]}"; do
  echo "Entering quickSort${n}..."
  cd "quickSort" || exit 1

  echo "----------------------------------------"
  echo "Compiling quicksort_mpi for N=${n} ..."
  mpicc quicksort_mpi.c -o quicksort_mpi -lm
  
  DATA_FILE="../data/${n}/chars.txt"

  if [ ! -f "$DATA_FILE" ]; then
    echo "ERROR: File $DATA_FILE does not exist"
    exit 1
  fi

    echo "Running tests for N=${n}"
  echo "Using input file: $DATA_FILE"
  echo "----------------------------------------"

  for p in "${processes[@]}"; do
    echo "➡ Running with ${p} processes..."
    mpirun --oversubscribe -np "${p}" ./quicksort_mpi "${n}" "${DATA_FILE}"
    echo "----------------------------------------"
  done
  
done
