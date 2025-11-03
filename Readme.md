# Instalación: 

```bash
pip install mpi4py
pip install scikit-learn

```
# Ejecución

```bash
mpirun -np 4 python proyecto_v1.py
```

# Inicialización
Por favor ejecute `data.py` para que los datos sean generados ya que están ignorador por el readme.
```bash
mpirun -np 4 python proyecto_v1.py
```
# Experimentos
Set `TESTING=False` in `proyecto_v1.py` and `proyecto_v2.py`, then `GENERATE_ONE = FALSE` in `data.py` to run the experiments.
```bash
chmod +x run_experiments.sh
./run_experiments.sh
```


# Data

La data está estructurada de la sigueinte forma, dónde el núemero representa la cantidad de datos. Si se quiere modificar los features revise `n_features` de `data.py`.

```bash
proyecto/
├── data/
│   ├── 5000/
│   └── 10000/
```
# TODO

- [ ] v3
- [ ] complexity chart
- [ ] speedup, efficiency, etc chart
- [ ] flops
