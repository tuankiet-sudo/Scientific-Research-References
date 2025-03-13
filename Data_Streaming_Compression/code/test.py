import csv
import numpy as np
# Specify the path to your CSV file
DEGREE = 6
FILE = 'data/UCR-time-series/Crop.csv'

time, value = [], []
with open(FILE, mode='r') as file:
    reader = csv.reader(file)
    # Read each row
    for row in reader:
        time.append(float(row[0]))
        value.append(float(row[1]))

coeffs = np.polyfit(np.array(range(len(value))), np.array(value), DEGREE)
polynomial = np.poly1d(coeffs)

y_fit = polynomial(np.array(range(len(value))))

print(np.max(np.abs(y_fit - value)))

# Writing to the CSV file
with open("test.csv", mode='w', newline='') as file:
    writer = csv.writer(file)
    for i in range(len(value)):
        writer.writerow([int(time[i]), float(y_fit[i])])