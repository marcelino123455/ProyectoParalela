#!/bin/bash

# check if the ./data exists
if [ ! -d "data" ]; then
  echo "There is no data, running data.py"
  python data.py
  echo "Data generation complete"
else
  echo "Data already exists."
fi

K_VALUES=(1 2 3 4 5 6)
processes=()
for k in "${K_VALUES[@]}"; do
  processes+=($((k * k)))
done

versions=(3)

for v in "${versions[@]}"; do
  echo "Compiling version v${v}..."
  mpic++ -o "./v${v}/main" "./v${v}/main.cpp"
  
  echo "Entering v${v}..."
  cd "./v${v}" || exit 1

  for p in "${processes[@]}"; do
    echo "Running version ${v} with ${p} processes..."
    mpirun -np "${p}" ".//main"
    echo "-----------------------------------------"
  done

  echo "Saliending xd v${v}..."
  cd ".."
done
