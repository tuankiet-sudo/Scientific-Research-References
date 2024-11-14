import os
import sys
import csv
import numpy as np
import matplotlib.pyplot as plt


def load(file):
    data = []
    time = []

    with open(file, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            time.append(int(line[0]))
            data.append(float(line[1]))
        
    return np.array(time), np.array(data)        


def plot(size, title, type, objs):
    plt.figure(figsize=size)
    plt.title(title)

    for obj in objs:
        if type == "line":
            plt.plot(obj.value, label=obj.legend)
        elif type == "point":
            plt.plot(obj.value, 'o', label=obj.legend)
            
    #plt.legend(bbox_to_anchor=(-0.15, 0.15))
    plt.legend()
    plt.show()


if __name__ == "__main__":
    
    plt.figure(figsize=(16, 8))
    for i in range(1, len(sys.argv) - 1, 2):
        legend = sys.argv[i]
        time, data = load(sys.argv[i+1])
        plt.plot(time, data, label=legend)
        
    plt.legend()
    plt.show()