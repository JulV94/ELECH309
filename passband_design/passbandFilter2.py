#! /usr/bin/python

# Passband filter design

import json
import numpy
from matplotlib import pyplot
from math import pi, sin

def filters_to_int(data):
    multiplier = 1 << data["integer_filter"]["bit_shift"]
    for filt in data["filters"]:
        for stage in filt["stages"]:
            for i in range(len(stage["denCoeff"])):
                stage["denCoeff"][i] = int(multiplier * stage["denCoeff"][i])
            for i in range(len(stage["numCoeff"])):
                stage["numCoeff"][i] = int(multiplier * stage["numCoeff"][i])
            stage["gain"] = int(multiplier * stage["gain"])

def overflow_test(value, max, context):
    if value > max:
        print(str(context) + " - Value exceeded the limit : " + str(value))

def passband(inp, data):
    limit = (1 << 32) - 1
    output = []
    multiplier = 1 << data["integer_filter"]["bit_shift"]
    for filt in data["filters"]:
        newMemory, acc = 0, 0
        myInput = inp
        for stage in filt["stages"]:
            newMemory = stage["denCoeff"][0] * myInput - stage["denCoeff"][1] * stage["memory"][0] - stage["denCoeff"][2] * stage["memory"][1]
            overflow_test(newMemory, limit, "newMemory 1")
            if data["integer_filter"]["is_active"]:
                newMemory = newMemory//multiplier
            overflow_test(newMemory, limit, "newMemory 2")
            acc = stage["numCoeff"][0] * newMemory + stage["numCoeff"][1] * stage["memory"][0] + stage["numCoeff"][2] * stage["memory"][1]
            overflow_test(acc, limit, "acc 1")
            if data["integer_filter"]["is_active"]:
                acc = acc//multiplier
            overflow_test(acc, limit, "acc 2")
            myInput = stage["gain"] * acc
            overflow_test(myInput, limit, "myInput 1")
            if data["integer_filter"]["is_active"]:
                myInput = myInput//multiplier
            overflow_test(myInput, limit, "myInput 2")
            stage["memory"][1] = stage["memory"][0]
            stage["memory"][0] = newMemory
        output.append(myInput)
    return output

def plot(data, ref, signals):
    scaled_ref=[]

    endTime = data["sampleNb"] * 1/data["sample_freq"]
    time = numpy.linspace(0, endTime, data["sampleNb"])
    for item in ref:
        scaled_ref.append(item - (1 << data["adc_resolution"])/2)

    pyplot.plot(time, scaled_ref, label="Input sine "+str(data["input_freq"])+" Hz")
    #for i in range(len(signals)):
    pyplot.plot(time, signals[1], label=data["filters"][1]["name"], color="forestgreen")
    pyplot.plot(time, signals[0], label=data["filters"][0]["name"], color="darkorange")

    pyplot.grid()
    pyplot.legend(loc='upper left')
    pyplot.xlabel('t (s)')
    pyplot.ylabel('Amplitude')
    pyplot.title("IIR digital biquad filter Direct form II")
    pyplot.ylim(-600, 870)
    #pyplot.show()
    pyplot.savefig("graphs/test.png")

def main():
    with open('filters.json') as f:
        data = json.load(f)

    if data["integer_filter"]["is_active"]:
        filters_to_int(data)

    entry = []
    out = []
    for filt in data["filters"]:
        out.append([])

    for sample in range(data["sampleNb"]):
        entry.append(int(((1 << data["adc_resolution"])-1) * (sin(2*pi*data["input_freq"]*sample/data["sample_freq"])+1)/2))
        output = passband(entry[-1], data)
        for i in range(len(output)):
            out[i].append(output[i])

    plot(data, entry, out)

if __name__ == "__main__":
    main()
