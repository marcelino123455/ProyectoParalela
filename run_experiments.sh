#!/bin/bash

# check if the ./data exists
if [ ! -d "data" ]; then
  echo "There is no data, running data.py"
  python data.py
  echo "Data generation complete"
else
  echo "Data already exists."
fi

processes=(2 4 6 8 16 32)
versions=(1 2)

for v in "${versions[@]}"; do
  for p in "${processes[@]}"; do
    echo "Running version ${v} with ${p} processes..."
    mpirun -np "${p}" python "proyecto_v${v}.py"
    echo "-----------------------------------------"
  done
done
