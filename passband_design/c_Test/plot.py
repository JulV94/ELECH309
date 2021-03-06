#! /usr/bin/python

# Passband filter design

import json
import numpy
from matplotlib import pyplot
from math import pi, sin, floor

def plot(data, ref, signals):
    offseted_ref = []
    endTime = data["sampleNb"] * 1/data["sample_freq"]
    time = numpy.linspace(0, endTime, data["sampleNb"])

    for item in ref:
        offseted_ref.append(item-(1<<data["adc_resolution"])/2)

    #pyplot.plot(time, offseted_ref, label="Input sine "+str(data["input_freq"])+" Hz")
    pyplot.plot(time, offseted_ref, label="Signal d'entrée")
    #for i in range(len(signals)):
    #    pyplot.plot(time, signals[i], label="filter "+str(i+1))
    pyplot.plot(time, signals[0], label="Filtre 900 Hz")
    pyplot.plot(time, signals[1], label="Filtre 1100 Hz")
    pyplot.grid()
    pyplot.legend(loc='upper left')
    pyplot.xlabel('t (s)')
    pyplot.ylabel('Amplitude')
    pyplot.title("Trame en sortie des filtres")
    pyplot.ylim(-600, 870)
    #pyplot.show()
    pyplot.savefig("test.png")

def main():
    data = {"sampleNb":20000, "sample_freq":15000, "input_freq":900, "adc_resolution":10}
    with open('filter_input.json') as fi:
        input = json.load(fi)
    with open('filter_1.json') as f1:
        filter1 = json.load(f1)
    with open('filter_2.json') as f2:
        filter2 = json.load(f2)

    plot(data, input, [filter1, filter2])

if __name__ == "__main__":
    main()
