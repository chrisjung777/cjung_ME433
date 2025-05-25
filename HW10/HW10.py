import csv
import numpy as np
import matplotlib.pyplot as plt

file_names = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
window_size = 100 

def moving_average(data, window):
    return np.convolve(data, np.ones(window)/window, mode='same')

for file in file_names:
    t = []
    y = []

    with open(file, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            try:
                t_val = float(row[0])
                y_val = float(row[1])
                t.append(t_val)
                y.append(y_val)
            except:
                continue

    t = np.array(t)
    y = np.array(y)

    y_filtered = moving_average(y, window_size)

    dt = np.mean(np.diff(t))
    fs = 1.0 / dt

    plt.figure(figsize=(10, 4))
    plt.plot(t, y, color='black', label='Unfiltered')
    plt.plot(t, y_filtered, color='red', label='Moving Avg Filtered')
    plt.xlabel('Time (s)')
    plt.ylabel('Signal')
    plt.title(f"{file[:-4]}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    outname = file.replace('.csv', '_filtered_plot.png')
    plt.savefig(outname)
    plt.close()

    print(f"Processed and saved: {outname}")
