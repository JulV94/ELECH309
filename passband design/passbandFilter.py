#! /usr/bin/python

# Passband filter design

import json
import numpy
from matplotlib import pyplot
from math import pi, sin

def passband(inp, data):
    output = []
    for k in range(len(data["filters"])):
        newMemory, acc = 0, 0
        myInput = inp
        for i in range(len(data["filters"][k])):
            newMemory = data["filters"][k][i]["denCoeff"][0] * myInput
            for j in range(len(data["filters"][k][i]["memory"])):
                newMemory -= data["filters"][k][i]["denCoeff"][j+1] * data["filters"][k][i]["memory"][j]
            acc = data["filters"][k][i]["numCoeff"][0] * newMemory
            for j in range(len(data["filters"][k][i]["memory"])):
                acc += data["filters"][k][i]["numCoeff"][j+1] * data["filters"][k][i]["memory"][j]
            myInput = data["filters"][k][i]["gain"] * acc
            for j in range(len(data["filters"][k][i]["memory"])-1, 0, -1):
                data["filters"][k][i]["memory"][j] = data["filters"][k][i]["memory"][j-1]
            data["filters"][k][i]["memory"][0] = newMemory
        output.append(myInput)
    return output

with open('filters.json') as f:
    data = json.load(f)

entry = []
out = []
for i in range(len(data["filters"])):
    out.append([])

for sample in range(data["sampleNb"]):
    entry.append(data["input_amplitude"] * sin(2*pi*data["input_freq"]*sample/data["sample_freq"]))
    output = passband(entry[-1], data)
    for i in range(len(output)):
        out[i].append(output[i])

endTime = data["sampleNb"] * 1/data["sample_freq"]
time = numpy.linspace(0, endTime, data["sampleNb"])
pyplot.plot(time, entry, label="Input signal")
for i in range(len(out)):
    pyplot.plot(time, out[i], label="Filter "+str(i))
pyplot.grid()
pyplot.legend(loc='upper left')
pyplot.xlabel('t (s)')
pyplot.ylabel('CH (V)')
#pyplot.ylim(-2*data["input_amplitude"], 2*data["input_amplitude"])
pyplot.show()