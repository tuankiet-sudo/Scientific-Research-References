import os
import csv
import sys
import numpy as np


def load(origin, approx):
    origin_data = []
    approx_data = []

    with open(origin, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            origin_data.append(float(line[1]))
                    
    with open(approx, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            approx_data.append(float(line[1]))
        
    return np.array(origin_data), np.array(approx_data)


def rmse(origin_data, approx_data):
    return np.sqrt(np.square(origin_data - approx_data).mean())


def mse(origin_data, approx_data):
    return np.square(origin_data - approx_data).mean()


def maxdiff(origin_data, approx_data):
    return np.max(np.abs(origin_data - approx_data))


def mindiff(origin_data, approx_data):
    return np.min(np.abs(origin_data - approx_data))
   
   
def compressratio(origin, approx):
    return os.path.getsize(origin) / os.path.getsize(approx)
    

if __name__ == "__main__":
    ORIGIN = sys.argv[1]
    APPROX = sys.argv[2]
    COMPRESS = sys.argv[3]
    
    origin_data, approx_data = load(ORIGIN, APPROX)
    count_min = min(origin_data.shape[0], approx_data.shape[0])
    
    origin_data = origin_data[:count_min]
    approx_data = approx_data[:count_min]
    
    print("MSE:", mse(origin_data, approx_data))
    print("RMSE:", rmse(origin_data, approx_data))
    print("Max Diff:", maxdiff(origin_data, approx_data))
    print("Min Diff:", mindiff(origin_data, approx_data))
    print("Compress Ratio:", compressratio(ORIGIN, COMPRESS))