#!/usr/bin/env python

# Based on https://github.com/lakshayg/google_benchmark_plot

"""Script to visualize google-benchmark output"""
from __future__ import print_function
import argparse
import io
import math
import sys
import logging
import json
import pandas as pd
import matplotlib.pyplot as plt
import pathlib
from PIL import Image

logging.basicConfig(format="[%(levelname)s] %(message)s")

METRICS = [
    "real_time",
    "cpu_time",
    "bytes_per_second",
    "items_per_second",
    "iterations",
]
TRANSFORMS = {"": lambda x: x, "inverse": lambda x: 1.0 / x}


def get_default_ylabel(args):
    """Compute default ylabel for commandline args"""
    label = ""
    if args.transform == "":
        label = args.metric
    else:
        label = args.transform + "(" + args.metric + ")"
    if args.relative_to is not None:
        label += " relative to %s" % args.relative_to
    return label


def parse_args():
    """Parse commandline arguments"""
    parser = argparse.ArgumentParser(description="Visualize google-benchmark output")
    parser.add_argument(
        "-f",
        metavar="FILE",
        type=argparse.FileType("r"),
        default=sys.stdin,
        dest="file",
        help="path to file containing the csv or json benchmark data",
    )
    parser.add_argument(
        "-m",
        metavar="METRIC",
        choices=METRICS,
        default=METRICS[0],
        dest="metric",
        help="metric to plot on the y-axis, valid choices are: %s" % ", ".join(METRICS),
    )
    parser.add_argument(
        "-t",
        metavar="TRANSFORM",
        choices=TRANSFORMS.keys(),
        default="",
        help="transform to apply to the chosen metric, valid choices are: %s"
        % ", ".join(list(TRANSFORMS)),
        dest="transform",
    )
    parser.add_argument(
        "-r",
        metavar="RELATIVE_TO",
        type=str,
        default=None,
        dest="relative_to",
        help="plot metrics relative to this label",
    )
    parser.add_argument(
        "--xlabel", type=str, default="input size", help="label of the x-axis"
    )
    parser.add_argument("--ylabel", type=str, help="label of the y-axis")
    parser.add_argument("--title", type=str, default="", help="title of the plot")
    parser.add_argument(
        "--logx", action="store_true", help="plot x-axis on a logarithmic scale"
    )
    parser.add_argument(
        "--logy", action="store_true", help="plot y-axis on a logarithmic scale"
    )
    parser.add_argument(
        "--output", type=str, default="", help="File in which to save the graph"
    )

    args = parser.parse_args()
    if args.ylabel is None:
        args.ylabel = get_default_ylabel(args)
    return args


def parse_input_size(name):
    splits = name.split("/")
    if len(splits) <= 2:
        return 1
    return int(splits[-1])


def read_data(args):
    """Read and process dataframe using commandline args"""
    extension = pathlib.Path(args.file.name).suffix
    try:
        if extension == ".csv":
            data = pd.read_csv(args.file, usecols=["name", args.metric])
            context = None
        elif extension == ".json":
            json_data = json.load(args.file)
            context = json_data['context']
            data = pd.DataFrame(json_data["benchmarks"])
        else:
            logging.error("Unsupported file extension '{}'".format(extension))
            exit(1)
    except ValueError:
        logging.error(
            'Could not parse the benchmark data. Did you forget "--benchmark_format=[csv|json] when running the benchmark"?'
        )
        exit(1)
    data["group"] = data["name"].apply(lambda x: x.split("/")[0])
    data["label"] = data["name"].apply(lambda x: x.split("/")[1])
    data["input"] = data["name"].apply(parse_input_size)
    data[args.metric] = data[args.metric].apply(TRANSFORMS[args.transform])
    return data, context


def plot_groups(label_groups, title, args):
    """Display the processed data"""
    for label, group in label_groups.items():
        plt.plot(group["input"], group[args.metric], label=label, marker=".")
    if args.logx:
        plt.xscale("log")
    if args.logy:
        plt.yscale("log")
    plt.xlabel(args.xlabel)
    plt.ylabel(args.ylabel)
    plt.title(title)
    plt.legend()
    file = io.BytesIO()
    plt.savefig(file)
    plt.clf()
    return file


def plot_context(context):
    plt.figure(figsize=(20,6), dpi=80)
    plt.axis('off')

    fields = [ 'executable', 'num_cpus', 'date', 'mhz_per_cpu', 'cpu_scaling_enabled', 'library_build_type']
    k = 0
    for field in fields:
        if field not in context:
            continue
        plt.text(0, 1-0.1 * k, f"{field}: {context[field]}", fontsize=20)
        k += 1

    if 'caches' in context:
        caches = context['caches']
        for i, cache in enumerate(caches):
            try:
                plt.text(0, 1-0.1 * k, 
                         f"cache-{i}: [ Type: {cache['type']}, Level: {cache['level']}," +
                         f"Size: {cache['size']}, Num_sharing: {cache['num_sharing']} ]",
                         fontsize=20)
                k += 1
            except:
                pass

    file = io.BytesIO()
    plt.savefig(file)
    plt.clf()
    return file


def crop_surround_almost_white_border(img):
    def almost_white(color):
        for x, w in zip(color, (255,255,255)):
            if abs(w-x) / 255 > 0.1:
                return False
        return True

    nonwhite_positions = [(x,y) for x in range(img.size[0]) for y in range(img.size[1]) if not almost_white(img.getdata()[x+y*img.size[0]])]
    rect = (min([x for x,_ in nonwhite_positions]), min([y for _,y in nonwhite_positions]), max([x for x,_ in nonwhite_positions]), max([y for _,y in nonwhite_positions]))
    return img.crop(rect)


def main():
    """Entry point of the program"""
    args = parse_args()
    data, context = read_data(args)
    groups = {}
    for group, group_data in data.groupby("group"):
        groups[group] = {}
        label_groups = groups[group]
        for label, record in group_data.groupby("label"):
            label_groups[label] = record.set_index("input", drop=False)
    if args.relative_to is not None:
        for group, label_groups in groups.items():
            try:
                baseline = label_groups[args.relative_to][args.metric].copy()
            except KeyError as key:
                msg = "Key %s is not present in the benchmark output"
                logging.error(msg, str(key))
                exit(1)

            for label in label_groups:
                label_groups[label][args.metric] /= baseline

    images = {}
    num_items = 0
    max_w, max_h = 0, 0
    for group, label_groups in groups.items():
        pimg = Image.open(plot_groups(label_groups, group, args))
        images[group] = pimg
        num_items += 1
        max_w = max(max_w, pimg.size[0])
        max_h = max(max_h, pimg.size[1])

    image_output = args.output if args.output != '' else 'benckmark_output.png'
    columns = math.ceil(math.sqrt(num_items))
    rows = math.ceil(num_items / columns)
    output_image = Image.new("RGB", (max_w * columns, max_h * rows), (255,255,255))

    for i, (_, img) in enumerate(images.items()):
        row = i // columns
        col = i % columns
        off_h = max_h - img.size[1]
        output_image.paste(img, (col * max_w, row * max_h + off_h))

    if context is not None:
        context_img = crop_surround_almost_white_border(Image.open(plot_context(context)))
        if context_img.size[0] > output_image.size[0]:
            om = Image.new("RGB", (context_img.size[0], output_image.size[1]), (255,255,255))
            om.paste(output_image, ((context_img.size[0] - output_image.size[0]) // 2, 0))
            output_image = om

        vertical_gap = 50
        om = Image.new("RGB", (output_image.size[0], output_image.size[1] + context_img.size[1] + vertical_gap * 2), (255,255,255))
        om.paste(output_image, (0, 0))
        om.paste(context_img, ((om.size[0] - context_img.size[0]) // 2, output_image.size[1] + vertical_gap))
        output_image = om

    print(f"saving image to '{image_output}'")
    output_image.save(image_output)


if __name__ == "__main__":
    main()
