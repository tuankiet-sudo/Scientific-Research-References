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
            origin_data.append([int(line[0]), float(line[1])])
        
        origin_data = sorted(origin_data, key=lambda a_entry: a_entry[0]) 
                
    with open(approx, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            approx_data.append([int(line[0]), float(line[1])])
        
        approx_data = sorted(approx_data, key=lambda a_entry: a_entry[0]) 
    
    return np.array(origin_data)[:, 1], np.array(approx_data)[:, 1]


def load_monitor(compress, decompress):
    c_data = []
    d_data = []
    
    with open(compress, 'r') as file:
        csvFile = csv.reader(file)
        next(csvFile, None)
        for line in csvFile:
            c_data.append([int(line[2]), int(line[3])])
                    
    with open(decompress, 'r') as file:
        csvFile = csv.reader(file)
        next(csvFile, None)
        for line in csvFile:
            d_data.append([int(line[2]), int(line[3])])
        
    return np.array(c_data), np.array(d_data)


def rmse(origin_data, approx_data):
    return np.sqrt(np.square(origin_data - approx_data).mean())


def mse(origin_data, approx_data):
    return np.square(origin_data - approx_data).mean()


def mae(origin_data, approx_data):
    return np.abs(origin_data - approx_data).mean()


def maxdiff(origin_data, approx_data):
    index = np.argmax(np.abs(origin_data - approx_data))
    return np.max(np.abs(origin_data - approx_data)), index, origin_data[index], approx_data[index]


def mindiff(origin_data, approx_data):
    return np.min(np.abs(origin_data - approx_data))
   
   
def compressratio(origin, approx):
    return os.path.getsize(origin) / os.path.getsize(approx)
    

if __name__ == "__main__":
    ORIGIN = sys.argv[1]
    APPROX = sys.argv[2]
    C_MONITOR = sys.argv[3]
    D_MONITOR = sys.argv[4]
    COMPRESS = sys.argv[5]
    
    origin_data, approx_data = load(ORIGIN, APPROX)
    c_data, d_data = load_monitor(C_MONITOR, D_MONITOR)
    count_min = min(origin_data.shape[0], approx_data.shape[0])
    
    origin_data = origin_data[:count_min]
    approx_data = approx_data[:count_min]
    
    print("MSE:", mse(origin_data, approx_data))
    print("RMSE:", rmse(origin_data, approx_data))
    # print("MAE:", mae(origin_data, approx_data))
    print("Max Diff:", maxdiff(origin_data, approx_data))
    print("Min Diff:", mindiff(origin_data, approx_data))
    print("Compress Ratio:", compressratio(ORIGIN, COMPRESS))
    # print("c_max_vsz:", np.max(c_data[:,0]))
    # print("c_max_rss:", np.max(c_data[:,1]))
    # print("d_max_vsz:", np.max(d_data[:,0]))
    # print("d_max_rss:", np.max(d_data[:,1]))
    