import numpy as np
from sklearn.datasets import make_classification
import os

# Configuración

np.random.seed(26)
GENERATE_ONE = True
DATA_DIR = "./data"
os.makedirs(DATA_DIR, exist_ok=True)

if GENERATE_ONE:
    n_samples = [20000, 40000]
else:
    n_samples = [5000, 10000, 20000, 40000]


n_features = 20
n_clases = 2

for n_sample in n_samples:

    X = np.random.rand(n_sample, n_features)
    y = np.random.randint(0, n_clases, size=n_sample)

    print("X.shape: ", X.shape)
    print("y.shape: ", y.shape)
    
    final_path_data = os.path.join(DATA_DIR, f"{n_sample}")
    os.makedirs(final_path_data, exist_ok=True)

    np.save(os.path.join(final_path_data, f"X.npy"), X)
    np.save(os.path.join(final_path_data, f"y.npy"), y)


