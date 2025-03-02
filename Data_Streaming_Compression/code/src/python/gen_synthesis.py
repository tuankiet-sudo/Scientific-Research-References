import csv
import random
import numpy as np

LENGTH = 105
NOISE = 20

# # GEN QUADRATIC
# FILE = "data/Synthesis/quadratic.csv"
# with open(FILE, 'w', newline='') as file:
#     writter = csv.writer(file, delimiter=',')
#     for i in range(LENGTH):
#         value = 0.47*(i**2) + 52.7*i + 5.8 + random.randint(-NOISE, NOISE)
#         writter.writerow([i, value])
    

# GEN CUBIC
FILE = "data/Synthesis/cubic.csv"
with open(FILE, 'w', newline='') as file:
    writter = csv.writer(file, delimiter=',')
    for i in range(LENGTH):
        value = 0.333*(i**3) -62.5*(i**2) + 3750*i - 5000 + random.randint(-NOISE, NOISE)
        writter.writerow([i, value])