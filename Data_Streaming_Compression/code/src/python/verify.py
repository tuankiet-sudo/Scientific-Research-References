import csv

ORIGIN = "data/input/synthesis/quadratic.csv" 
APPROX = "data/output/synthesis/lf_appro.csv"

if __name__ == "__main__":
    origin = []
    approx = []
    
    with open(ORIGIN, mode="r") as f:
        reader = csv.reader(f)
        for row in reader:
            origin.append(float(row[1]))
            
    with open(APPROX, mode="r") as f:
        reader = csv.reader(f)
        for row in reader:
            approx.append(float(row[1]))
    
    diff = [abs(origin[i]-approx[i]) for i in range(len(approx))]
    print(max(diff))