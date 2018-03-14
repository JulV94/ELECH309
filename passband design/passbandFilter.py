#! /usr/bin/python

# Passband filter design

import json
import numpy
from matplotlib import pyplot
from math import pi, sin, floor

def filters_to_int(data):
    multiplier = 1 << data["integer_filter"]["bit_shift"]
    for filt in data["filters"]:
        for stage in filt["stages"]:
            for i in range(len(stage["denCoeff"])):
                stage["denCoeff"][i] = floor(multiplier * stage["denCoeff"][i])
            for i in range(len(stage["numCoeff"])):
                stage["numCoeff"][i] = floor(multiplier * stage["numCoeff"][i])
            stage["gain"] = floor(multiplier * stage["gain"])

def passband(inp, data):
    output = []
    multiplier = 1 << data["integer_filter"]["bit_shift"]
    for filt in data["filters"]:
        newMemory, acc = 0, 0
        myInput = inp
        for stage in filt["stages"]:
            newMemory = stage["denCoeff"][0] * myInput
            if data["integer_filter"]["is_active"]:
                newMemory = newMemory * multiplier
            for i in range(len(stage["memory"])):
                newMemory -= stage["denCoeff"][i+1] * stage["memory"][i]
            if data["integer_filter"]["is_active"]:
                newMemory = newMemory//multiplier
            acc = stage["numCoeff"][0] * newMemory
            for i in range(len(stage["memory"])):
                acc += stage["numCoeff"][i+1] * stage["memory"][i]
            if data["integer_filter"]["is_active"]:
                acc = acc//(multiplier * multiplier)
            myInput = stage["gain"] * acc
            if data["integer_filter"]["is_active"]:
                myInput = myInput//multiplier
            for i in range(len(stage["memory"])-1, 0, -1):
                stage["memory"][i] = stage["memory"][i-1]
            stage["memory"][0] = newMemory
        output.append(myInput)
    return output

def plot(data, ref, signals):
    scaled_ref=[]
    scaled_signals=[]
    for sig in signals:
        scaled_signals.append([])

    endTime = data["sampleNb"] * 1/data["sample_freq"]
    time = numpy.linspace(0, endTime, data["sampleNb"])
    for item in ref:
        scaled_ref.append(data["input_amplitude"]*((2*item/((1 << data["adc_resolution"])-1))-1))
    for i in range(len(signals)):
        for item in signals[i]:
            scaled_signals[i].append(data["input_amplitude"]*(2*item/((1 << data["adc_resolution"])-1)))

    pyplot.plot(time, scaled_ref, label="Input "+str(data["input_freq"])+" Hz")
    for i in range(len(signals)):
        pyplot.plot(time, scaled_signals[i], label=data["filters"][i]["name"])
    pyplot.grid()
    pyplot.legend(loc='upper left')
    pyplot.xlabel('t (s)')
    pyplot.ylabel('CH (V)')
    #pyplot.ylim(-2*data["input_amplitude"], 2*data["input_amplitude"])
    pyplot.show()

def main():
    with open('filters.json') as f:
        data = json.load(f)

    if data["integer_filter"]["is_active"]:
        filters_to_int(data)

    #print(data["filters"][0]["stages"][0])

    entry = []
    out = []
    for filt in data["filters"]:
        out.append([])

    for sample in range(data["sampleNb"]):
        entry.append(floor(((1 << data["adc_resolution"])-1) * (sin(2*pi*data["input_freq"]*sample/data["sample_freq"])+1)/2))
        output = passband(entry[-1], data)
        for i in range(len(output)):
            out[i].append(output[i])

    print(data["filters"][0]["stages"][0])

    plot(data, entry, out)
    #print(entry)

if __name__ == "__main__":
    main()
