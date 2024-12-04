import os
import csv
import sys
import shutil

def load_tsv(name):
    data = {}
    file = open(name)
    
    reader = csv.reader(file, delimiter='\t')
    for row in reader:
        if row[0] in data:
            data[row[0]] = data[row[0]] + row[1:]
        else:
            data[row[0]] = row[1:]
    
    file.close()
    return data


def load_csv(name):
    data = []
    file = open(name)
    
    reader = csv.reader(file, delimiter=',')
    for row in reader:
        data.append(row[1])
    
    file.close()
    return data


def write(data, name):
    index = 0
    file = open(name, "w+")
    
    for ele in data:
        index += 1
        file.write("{},{}\n".format(index, ele))
    
    file.close()


dirName = sys.argv[1]
dataName = dirName.split('/')[-2]

train = load_tsv(dirName + dataName + "_TRAIN.tsv")
test = load_tsv(dirName + dataName + "_TEST.tsv")

keys = set(train.keys()).union(set(test.keys()))
for key in keys:
    data = train[key] + test[key]
    write(data, dirName + key + ".csv")
    
os.remove(dirName + dataName + "_TRAIN.tsv")
os.remove(dirName + dataName + "_TEST.tsv")
os.remove(dirName + "README.md")

data = []
for name in os.listdir(dirName):
    file = dirName + "/" + name
    data += load_csv(file)

write(data, "{}/{}.csv".format("/".join(e for e in dirName.split('/')[:-2]), dataName))
shutil.rmtree(dirName)
