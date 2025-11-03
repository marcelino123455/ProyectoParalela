from mpi4py import MPI
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import accuracy_score
from joblib import dump
from sklearn.preprocessing import StandardScaler
import os
import pandas as pd


"""
 Input:
 S: Array de semillas
 hiperparámetros: (a, η, h, Tmax)

 Output: 
 M*: Mejor exactitud

"""
# Parámetros

# TODO Preguntar si es fijado o varía con p
S = np.arange(32)
alpha = 0.001
eta = 0.001
h = (64, 32)
Tmax = 100

n_data = [5000, 10000, 20000, 40000]
DATA_DIR = "./data"
MODELS_DIR = "./modelos/paralelo/v2"
TIEMPOS_DIR = "./tiempos/v2"
os.makedirs(TIEMPOS_DIR, exist_ok=True)
os.makedirs(MODELS_DIR, exist_ok=True)

# Para testeo rápido
TESTING = True

if TESTING:
    S = [26, 27, 28, 29, 30]
    n_data = [5000, 10000]
    Tmax = 100

## Funciones ##
def entrenar_modelo_con_semilla(X, y, s, h, eta, alpha, Tmax, verbose = False):
    print(f"\nEntrenando modelo con semilla {s}...")

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=s
    )

    scaler = StandardScaler()
    X_train = scaler.fit_transform(X_train)
    X_test = scaler.transform(X_test)

    model = MLPClassifier(
        hidden_layer_sizes=h,
        learning_rate_init=eta,
        alpha=alpha,
        max_iter=Tmax,
        random_state=s,
        verbose=verbose

    )

    model.fit(X_train, y_train)

    y_pred = model.predict(X_test)
    acc = accuracy_score(y_test, y_pred)

    print(f"Exactitud (accuracy) con semilla {s}: {acc:.4f}")

    return model, acc

def main():
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    p = comm.Get_size()

    tiempos = []
    for n in n_data: # Esto for no es paralelizable, solo para automatización experimental
        print("#"*26, f"Expermientación con N = [{n}]", "#"*26)

        t0 = MPI.Wtime() # T

        # Mejora 1: Usamos broadcast para X e Y para evitar el bottleneck de lectura de disco
        X = None
        y = None

        if rank == 0:
            path_data_n = os.path.join(DATA_DIR, f"{n}")
            X_path_n = os.path.join(path_data_n, "X.npy")
            y_path_n = os.path.join(path_data_n, "y.npy")
            X = np.load(X_path_n)
            y = np.load(y_path_n)

        X = comm.bcast(X, root=0)
        y = comm.bcast(y, root=0)

        # --Repartir S/p--
        S_split = np.array_split(S, p)
        seeds_local = comm.scatter(S_split, root=0)
        print(f"p [{rank}] - ({n}) seeds:", seeds_local)


        best_acc_local = 0.0
        best_model_local = None
        best_seed_local = None

        for s in seeds_local: # Esto si es paralelizable
            print("*"*12, f"Expermientación con S = [{s}]", "*"*12)
            model, acc = entrenar_modelo_con_semilla(X, y, s=s, h=h, eta=eta, alpha=alpha, Tmax=Tmax,verbose = False)
            if acc>best_acc_local:
                best_acc_local = acc
                best_model_local = model
                best_seed_local = s
        # Ya tenemos los mejores por cada proceso
        results_local = (best_acc_local, best_seed_local, best_model_local)

        # Envíamos los resulados al master
        results_all = comm.gather(results_local, root=0)

        if rank == 0:
            best_acc_global = 0.0
            best_seed_global = None
            best_rank = 0
            for i, (acc, seed, model) in enumerate(results_all):
                if acc > best_acc_global:
                    best_acc_global = acc
                    best_seed_global = seed
                    best_rank = i

            model_filename = f"MLP_best_N[{n}]_seed_[{best_seed_global}]_acc_[{best_acc_global:.5f}].joblib"
            model_path = os.path.join(MODELS_DIR, model_filename)
            dump(results_all[best_rank][2], model_path)

        # Esperamos que todos acaben para recien empezar con el siguiente N
        comm.Barrier()
        t1 = MPI.Wtime() # T
        # Comenzamos a guardar los resultados
        if rank == 0:
            # El tiempo lo mide el master
            tiempo_total = t1 - t0
            print(f"Tiempo total para N={n}: {tiempo_total:.2f} segundos")

            tiempos.append((n, tiempo_total))
        comm.Barrier()

    if rank == 0:
        df_tiempos = pd.DataFrame(tiempos, columns=["N", "tiempo"])
        tiempors_p_dir = os.path.join(TIEMPOS_DIR, str(p))
        os.makedirs(tiempors_p_dir, exist_ok=True)

        csv_path = os.path.join(tiempors_p_dir, "tiempos.csv")
        df_tiempos.to_csv(csv_path, index=False)
        print(f"Tiempos guardados en {csv_path}")


if __name__ == "__main__":
    main()
