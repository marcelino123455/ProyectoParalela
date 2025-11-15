import numpy as np
import os
import string
import argparse
import math
from math import gcd

# Configuración
parser = argparse.ArgumentParser(description="Generador de caracteres aleatorios")
parser.add_argument("--testing", type=str, default="False")
args = parser.parse_args()
TESTING = args.testing.lower() == "true"

np.random.seed(26)  
DATA_DIR = "./data"
os.makedirs(DATA_DIR, exist_ok=True)

"""
(I) p = k^2
(II) N sea divisible por k y k^2
=> N = k multiplo y k^2 multiplo
=> Basta con N = multiplo de k^2
Usaremos potencias de 2
"""
K_VALUES = [1, 2, 4, 7, 8, 10]
P_VALUES = [k**2 for k in K_VALUES]

def mcm(a, b):
    return a * b // gcd(a, b)

def mcm_list(values):
    result = 1
    for v in values:
        result = mcm(result, v)
    return result

mcm_total = mcm_list(P_VALUES)
print(f"MCM de todos los k² = {mcm_total}")

if TESTING:
    n_samples = [18]
else:
    start = 2
    end = 6
    # For N chiquitos
    MULTIPLIERS = [1, 2, 3] 
    MULTIPLIERS += [3**i for i in range(start, end + 1)]


if TESTING:
    VALID_N = [18]   
else: 
    VALID_N = [mcm_total * m for m in MULTIPLIERS]



CHARSET = string.ascii_letters
# +string.digits

for n_sample in VALID_N:
    chars = np.random.choice(list(CHARSET), n_sample)
    final_path_data = os.path.join(DATA_DIR, f"{n_sample}")
    os.makedirs(final_path_data, exist_ok=True)
    
    file_path = os.path.join(final_path_data, "chars.txt")
    with open(file_path, "w", encoding="utf-8") as f:
        for c in chars:
            c = c.lower()
            f.write(c + "\n")
    print(f"Archivo generado: {file_path}")
    


