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


# Data

La data está estructurada de la sigueinte forma, dónde el núemero representa la cantidad de datos. Si se quiere modificar los features revise `n_features` de `data.py`.

```bash
proyecto/
├── data/
│   ├── 5000/
│   └── 10000/
```



