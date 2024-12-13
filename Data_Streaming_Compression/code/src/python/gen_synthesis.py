import csv
import random

FILE = "data/Synthesis/cubic.csv"
LENGTH = 105
NOISE = 0

with open(FILE, 'w', newline='') as file:
    writter = csv.writer(file, delimiter=',')
    for i in range(LENGTH):
        value = 0.008*(i**3) - 0.47*(i**2) + 52.7*i + 5.8 + random.randint(-NOISE, NOISE)
        writter.writerow([i, value])
    