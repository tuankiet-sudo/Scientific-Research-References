import os
import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

ERROR = 4.355

def load(file, index):
    data = []
    time = []

    with open(file, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            time.append(int(line[0]))
            data.append(float(line[index]))
        
    return time, data


def plot(s, e, label, time, data):
    plt.figure(figsize=(16, 8))

    for i in range(len(label)):
        plt.plot(time[i][s:e], data[i][s:e], label=label[i])

    plt.legend()
    plt.show()

if __name__ == "__main__":
    label, x, y = [], [], []
    for i in range(1, len(sys.argv) - 1, 2):
        time, value = load(sys.argv[i+1], 1)

        label.append(sys.argv[i])
        x.append(time)
        y.append(value)

    SIZE = len(x[0])
    INTERVAL = 5000

    start = 0
    while start < SIZE:
        plot(start, start+INTERVAL, label, x, y)
        start += INTERVAL