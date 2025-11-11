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
mpirun --oversubscribe -np  9 ./main --testing
```

Si desea ejecutar todos los ejemplos ejecute: 
```bash
mpirun --oversubscribe -np  4 ./main
```


# Experimentos
Set `TESTING=False` in `proyecto_v1.py` and `proyecto_v2.py`, then `GENERATE_ONE = FALSE` in `data.py` to run the experiments.
```bash
chmod +x run_experiments.sh
./run_experiments.sh
```


# Data
La data está estructurada de la sigueinte forma, dónde el núemero representa la cantidad de datos. Si se quiere modificar los features revise `K_VALUES` y `MULTIPLIERS` de `data.py`.

!Nota: Es relevante debido a que N debe ser múltiplo de raíz de p.



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
