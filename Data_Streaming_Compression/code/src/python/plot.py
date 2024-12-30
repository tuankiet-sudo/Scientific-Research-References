import os
import sys
import csv
import numpy as np
import matplotlib.pyplot as plt


def load(file, index):
    data = []
    time = []

    with open(file, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            time.append(int(line[0]))
            data.append(float(line[index]))
        
    return np.array(time[:200]), np.array(data[:200])        

if __name__ == "__main__":
    
    plt.figure(figsize=(16, 8))
    for i in range(1, len(sys.argv) - 1, 2):
        legend = sys.argv[i]
        time, data = load(sys.argv[i+1], 1)
        if i != 1:
            plt.plot(time, data, label=legend, color="blue")
        elif i == 1:
            plt.plot(time, data-20, color="red")
            plt.plot(time, data+20, color="red")
            plt.plot(time, data, label=legend, color="purple")
        
    plt.legend()
    plt.show()