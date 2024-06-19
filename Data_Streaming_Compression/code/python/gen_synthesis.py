import csv
import random

FILE = "data/input/synthesis/quadratic.csv"
LENGTH = 105
NOISE = 25

with open(FILE, 'w', newline='') as file:
    writter = csv.writer(file, delimiter=',')
    for i in range(LENGTH):
        # function y = -0.47x^2 + 52.7x + 5.8
        value = -0.47*(i**2) + 52.7*i + 5.8 + random.randint(-NOISE, NOISE)
        writter.writerow([i, value])
    