import pgzrun
import serial
import numpy as np
from pygame import Rect

ser = serial.Serial('COM4', 115200, timeout=1)
print('Opening port:', str(ser.name))

WIDTH = 640
HEIGHT = 480

def update():
    selection_endline = 'c' + '\n'
    ser.write(selection_endline.encode())

def draw():
    reds = np.zeros((60, 80), dtype=np.uint8)
    greens = np.zeros((60, 80), dtype=np.uint8)
    blues = np.zeros((60, 80), dtype=np.uint8)
    
    for t in range(4800):
        try:
            dat_str = ser.readline()
            if dat_str:
                data = dat_str.decode().strip().split()
                if len(data) == 4:
                    i, r, g, b = map(int, data)
                    row = i // 80
                    col = i % 80
                    if row < 60 and col < 80:
                        reds[row][col] = r
                        greens[row][col] = g
                        blues[row][col] = b
        except:
            continue

    screen.fill((0, 0, 0))  
    
    scale = 8
    for y in range(60):
        for x in range(80):
            color = (reds[y][x], greens[y][x], blues[y][x])
            screen.draw.filled_rect(
                Rect((x * scale, y * scale), (scale, scale)), 
                color
            )

pgzrun.go()
