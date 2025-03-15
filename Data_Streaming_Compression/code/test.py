import csv
import numpy as np
# Specify the path to your CSV file
DEGREE = 6
INPUT = 'data/UCR-time-series/Crop.csv'
OUTPUT = 'test_2.csv'

time, value = [], []
with open(INPUT, mode='r') as file:
    reader = csv.reader(file)
    # Read each row
    for row in reader:
        time.append(float(row[0]))
        value.append(float(row[1]))

x = ((2 * (np.array(time) - min(time))) / (max(time)-min(time))) - 1
# x = np.array(range(len(value)))
y = np.array(value)
coeffs = np.polyfit(x, y, DEGREE)
polynomial = np.poly1d(coeffs)

y_fit = polynomial(x)

print("max_e", np.max(np.abs(y_fit - value)))
print("mse:", np.square(y_fit - value).mean())

# Writing to the CSV file
with open(OUTPUT, mode='w', newline='') as file:
    writer = csv.writer(file)
    for i in range(len(value)):
        writer.writerow([int(time[i]), float(y_fit[i])])