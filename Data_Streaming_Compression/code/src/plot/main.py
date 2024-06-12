import sys
import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


class PlotObject:
    def __init__(self, legend, time, value) -> None:
        self.legend = legend
        self.time = time
        self.value = value
        

def gen_plot_object(legend, file):
    df = pd.read_csv(file, header=None)
    return PlotObject(legend, df[0], df[1])


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
    with open(sys.argv[1], mode="r") as f:
        config = json.load(f)
        
    objs = [gen_plot_object(data['legend'], data['file']) for data in config['data']]
    plot((config['width'], config['height']), config['title'], config['type'], objs)