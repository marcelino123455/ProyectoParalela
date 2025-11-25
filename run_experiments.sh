#!/bin/bash

# check if the ./data exists


if [ ! -d "data_dispersa" ]; then
  source env_paralela/bin/activate
  echo "There is no data_dispersa, running data_dispersa.py"
  python3 data_dispersa.py
  python3 data.py
  echo "Data_dispersa generation complete"
else
  echo "Data_dispersa already exists."
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
    mpirun -np "${p}" "./main"
    echo "-----------------------------------------"
  done

  echo "Saliending xd v${v}..."
  cd ".."
done
