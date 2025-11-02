
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import accuracy_score
from joblib import dump
from sklearn.preprocessing import StandardScaler
import os

"""
 Input:
 S: Array de semillas
 hiperparámetros: (a, η, h, Tmax)

 Output: 
 M*: Mejor exactitud

"""
# Parámetros
S = [26, 27]
alpha = 0.001
eta = 0.001
h = (64, 32)
Tmax = 600 

n_data = [5000, 10000, 20000, 40000]
DATA_DIR = "./data"
MODELS_DIR = "./modelos/secuencial"
os.makedirs(MODELS_DIR, exist_ok=True)


# Para testeo rápido
TESTING = True

if TESTING:
    S = [26, 27]
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


for n in n_data: # Esto no es paralelizable, solo para automatización experimental
    print("#"*26, f"Expermientación con N = [{n}]", "#"*26)
    path_data_n = os.path.join(DATA_DIR, f"{n}")
    X_path_n = os.path.join(path_data_n, "X.npy")
    y_path_n = os.path.join(path_data_n, "y.npy")
    X = np.load(X_path_n)
    y = np.load(y_path_n)
    
    best_model = None
    best_acc = 0.0
    best_s = None
    for s in S: # Esto si es paralelizable
        print("*"*12, f"Expermientación con S = [{s}]", "*"*12)
        model, acc = entrenar_modelo_con_semilla(X, y, s=s, h=h, eta=eta, alpha=alpha, Tmax=Tmax)
        if acc>best_acc:
            best_acc = acc
            best_model = model
            best_s = s
    print(f"\nEl mejor modelo se alcanza con {[best_s]}")
    print(f"Best acuracy {[best_acc]}")

    model_filename = f"MLP_best_N[{n}]_seed_[{best_s}]_acc_[{best_acc:.5f}].joblib"
    model_path = os.path.join(MODELS_DIR, model_filename)
    dump(best_model, model_path)
        


