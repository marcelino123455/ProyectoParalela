# Instalación: 

```bash

pip install mpi4py
pip install scikit-learn

```
# 0) Inicialización
Por favor ejecute `data.py` para generar la data.
En modo testing ejecute lo siguiente:
```bash
python3 data.py --testing True

```
Si quiere probar con toda la data ejecute:
```bash
 python3 data.py
```

# 1) Compilacion y ejecucion
Para compilar ejecute: 
```bash
mpic++ -o main main.cpp
```

Para una ejecución en modeo de testeo con N = 18 Y p = 9 (Puede ser usado para debugging): 
```bash
mpirun --oversubscribe -np  9 ./main --testing --check
```

Si desea ejecutar todos los ejemplos y verificar la correctitud del ordenamiento:
!Nota: Recomendable para pruebas N pequeños, debido a que el ordenamiento demora mucho.  
```bash
mpirun --oversubscribe -np  4 ./main --check
```
Caso contrario y deseable ejecute: 
```bash
mpirun --oversubscribe -np  4 ./main
```


# Experimentos
Before make experimentations please you have to enseure that K_VALUES are the same for `data.py` and `run_experiments.sh`.

```bash
chmod +x run_experiments.sh
./run_experiments.sh
```


# Data
La data está estructurada de la sigueinte forma, dónde el núemero representa la cantidad de datos. Si se quiere modificar los features revise `K_VALUES` y `MULTIPLIERS` de `data.py`.

!Nota: Es relevante debido a que N debe ser múltiplo de raíz de p.

# QuickSort
Añadimos `geminiQuick.cpp` el cual puede ser ejecutado de la siguiente forma:

```bash

mpic++ -o geminiQuick geminiQuick.cpp 
```

```bash
mpirun -np 16 ./geminiQuick 23619600
```
Donde el segundo numero representa la cantidad de datos. 
Tener en cuenta que la cantidad de procesadores que se deben de usar son potencias de 2. 



# Experimeentacion
La experimentación se limito a 40 procesos debido a que contamos con 40 cores en khipu en el node de acceso: 
```bash
lscpu

Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              80
On-line CPU(s) list: 0-79
Thread(s) per core:  2
Core(s) per socket:  20
Socket(s):           2
```


```bash
proyecto/
├── data/
│   ├── 14400/
│   └── 28800/
```


# TODO

- [ ] v2
- [ ] speedup, efficiency, etc chart
- [ ] Quick Sort paralelo copy page and run
- [ ] count flops


# Graficos

## Reparticion de la cuadrícula
![alt text](image.png)
