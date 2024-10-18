import os
import subprocess
import time
import csv
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

# tests = [
#     # circles
#     [
#         'circles_fill_color',
#         ["shapes", "--shape", "circle"],
#         [100, 500, 1000, 2000, 4000],
#     ],
#     [
#         'circles_fill_alpha',
#         ["shapes", "--shape", "circle", "--translucent"],
#         [100, 500, 1000, 2000, 4000],
#     ],
#     [
#         'circles_fill_textured',
#         ["shapes", "--shape", "circle", "--textured"],
#         [100, 500, 1000, 2000, 4000],
#     ],
#     [
#         'circles_stroke',
#         ["shapes", "--shape", "circle", "--stroke"],
#         [100, 500, 1000, 2000, 4000],
#     ],
#     # full screen fills
#     [
#         'full_screen_quads',
#         ["full_screen_quads"],
#         [10, 100, 200, 500],
#     ],
#     [
#         'full_screen_quads_alpha',
#         ["full_screen_quads", "--translucent"],
#         [10, 100, 200, 500],
#     ],
#     [
#         'full_screen_quads_textured',
#         ["full_screen_quads", "--textured"],
#         [10, 100, 200, 500],
#     ],
#     # images
#     [
#         'image_batches_10',
#         ["image_batches", "--image-count", "10"],
#         [100, 500, 1000],
#     ],
#     # tiger
#     [
#         'tiger',
#         ["svg"],
#         [],
#     ],
# ]

shape_tests = [
    [
        'circles_fill_color',
        ["shapes", "--shape", "circle"],
    ],
    [
        'circles_fill_alpha',
        ["shapes", "--shape", "circle", "--translucent"],
    ],
    [
        'circles_fill_textured',
        ["shapes", "--shape", "circle", "--textured"],
    ],
    [
        'circles_stroke',
        ["shapes", "--shape", "circle", "--stroke"],
    ],
]

fullscreen_fill_tests = [
    [
        'full_screen_quads',
        ["full_screen_quads"],
    ],
    [
        'full_screen_quads_alpha',
        ["full_screen_quads", "--translucent"],
    ],
    [
        'full_screen_quads_textured',
        ["full_screen_quads", "--textured"],
    ],
]

image_tests = [
    [
        'image_batches_10',
        ["image_batches", "--image-count", "10"],
    ],
]

svg_tests = [
    [
        'tiger',
        ["svg"],
    ],
]

def run_collect(tests, counts, outName):
    with open(outName, "w") as f:
        print('count; gpu smp ; gpu min; gpu max; gpu avg; gpu std; cpu smp ; cpu min; cpu max; cpu avg; cpu std ; pre smp ; pre min; pre max; pre avg; pre std ; ', file=f)

        for test in tests:
            print("running test " + test[0])
            print(test[0], file=f)

            if len(counts):
                for count in counts:
                    print(str(count) + ";", end="", file=f)
                    res = subprocess.run(["./bin/driver",
                                          "--auto", "120",
                                          "--csv",
                                          *test[1],
                                          "-c", str(count)],
                                          stdout=subprocess.PIPE)

                    if len(res.stdout.decode()) == 0:
                        print("error running test " + test[0])
                        print(res)

                    print(res.stdout.decode(), end="")
                    print(res.stdout.decode(), end="", file=f)

            else:
                print("1 ;", end="", file=f)
                res = subprocess.run(["./bin/driver",
                                      "--auto", "120",
                                      "--csv",
                                      *test[1]],
                                      stdout=subprocess.PIPE)

                if len(res.stdout.decode()) == 0:
                    print("error running test " + test[0])
                    print(res)

                print(res.stdout.decode(), end="")
                print(res.stdout.decode(), end="", file=f)

class series_data:
    def __init__(self, name):
        self.name = name
        self.x = []
        self.y = []

def plot_for_n(inName, heading='gpu avg'):
    series = []

    with open(inName, "r") as f:
        reader = csv.reader(f, delimiter=';', skipinitialspace=True)
        header = next(reader)

        headingIndex = header.index(heading)

        data = series_data('')
        for row in reader:
            if len(row) == 1:
                if len(data.y) != 0:
                    series.append(data)
                data = series_data(row[0])
            else:
                data.x.append(float(row[0]))
                data.y.append(float(row[headingIndex]))

    if len(data.y) != 0:
        series.append(data)

    for data in series:
        plt.plot(data.x, data.y, '-o', label=data.name)

    plt.ylabel(heading + ' (ms)')
    plt.legend(loc="upper left")
    plt.show()

def load_label_values_for_count(inName, count, heading='gpu avg'):
    labels = []
    values = []

    with open(inName, "r") as f:
        reader = csv.reader(f, delimiter=';', skipinitialspace=True)
        header = next(reader)

        headingIndex = header.index(heading)

        name = ''
        for row in reader:
            if len(row) == 1:
                name = row[0]
            else:
                if int(row[0]) == count:
                    labels.append(name)
                    values.append(row[headingIndex])

    values = [ float(e) for e in values]
    return (labels, values)

def plot_for_test(inName, count, heading='gpu avg'):

    labels, values = load_label_values_for_count(inName, count, heading)

    cmap = plt.cm.get_cmap('tab10', len(values))
    colors = [cmap(i) for i in range(0, len(values))]

    plt.bar(labels, values, color=colors)
    ax = plt.gca()
    ax.set_xticklabels(labels, rotation = 45, horizontalalignment='right')
    plt.ylabel(heading+' (ms)')
    plt.figtext(0.1, 0.9, '--count = ' + str(count), transform = ax.transAxes)
    plt.tight_layout()
    plt.show()


def plot_test_diffs(inName1, inName2, count, heading='gpu avg'):

    labels1, values1 = load_label_values_for_count(inName1, count, heading)
    labels2, values2 = load_label_values_for_count(inName2, count, heading)

    if labels1 != labels2:
        print('error: comparing datasets for different tests')
        return

    x = np.arange(len(labels1))
    width = 0.2
    fig, ax = plt.subplots()
    rects1 = ax.bar(x - width/2, values1, width, label=Path(inName1).stem)
    rects2 = ax.bar(x + width/2, values2, width, label=Path(inName2).stem)

    ax.set_xticks(x)
    ax.set_xticklabels(labels1, rotation = 45, horizontalalignment='right')
    ax.set_ylabel(heading + ' (ms)')
    ax.legend(loc="upper left", title = '--count = ' + str(count))

    fig.tight_layout()
    plt.show()
