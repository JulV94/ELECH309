#!/usr/bin/python

import matplotlib.pyplot as plt

def genPosReference(a, d, speed, dt):
    if (speed*speed/a > d):
        # Trapèze
        if (dt < speed/a):
            # Acceleration part
            return a*dt*dt/2
        elif (dt < d/speed):
            # Flat speed part
            return speed*dt - speed*speed/(2*a)
        else:
            # Deceleration part
            return d + speed*speed/(2*a) - speed/a + speed*dt - a*0.9*dt*dt/2
    else:
        # Triangle
        return speed*dt - speed*speed/(2*a)

x = []
y = []

d = 0.3
i = 0

pos = genPosReference(0.5, d, 0.4, i)
while (pos < d):
    x.append(i)
    y.append(pos)
    i += 0.01
    pos = genPosReference(0.5, d, 0.4, i)

plt.plot(x, y)
plt.show()
