#!/usr/bin/python

import matplotlib.pyplot as plt
from math import sqrt

def genPosReference(a, d, speed, dt):
    if (speed*speed/a < d):
        # Trapèze
        if (dt < speed/a):
            # Acceleration part
            return a*dt*dt/2
        elif (dt < d/speed):
            # Flat speed part
            return speed*dt - speed*speed/(2*a)
        else:
            # Deceleration part
            new_dt = dt - d/speed
            return d - speed*speed/(2*a) + speed*new_dt - a*new_dt*new_dt/2
            #return speed*dt - speed*speed/(2*a)
    else:
        # Triangle
        if (dt*dt < d/a):
            # Acceleration part
            return a*dt*dt/2
        else:
            # Deceleration part
            new_dt = dt - sqrt(d/a)
            return d/2 + a*sqrt(d/a)*new_dt - a*new_dt*new_dt/2

x = []
y = []

d = 0.6
i = 0

pos = genPosReference(0.5, d, 0.4, i)
while (pos < d and i < 3):
    x.append(i)
    y.append(pos)
    i += 0.01
    pos = genPosReference(0.5, d, 0.4, i)

plt.plot(x, y)
plt.show()
