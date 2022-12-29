#!/usr/bin/env python3

import sys

printe = lambda *args, **kwargs: print(*args, **kwargs, file=sys.stderr)

if len(sys.argv) < 2:
    printe(f"Usage: {sys.argv[0]} <path/to/ParamLabels.csv>")
    sys.exit(1)

def iter_pairs():
    for line in open(sys.argv[1], "rb").read().decode("utf-8").split("\n"):
        h40, label = line.split(",")
        h40 = int(h40, 16)
        label = label.strip("\r")
        if h40 == 0:
            continue
        yield h40, label

with open("application/data/motion/ParamLabels.dat", "wb") as f:
    for h40, label in iter_pairs():
        f.write(h40.to_bytes(5, "little"))
        f.write((len(label) + 1).to_bytes(1, "little"))
        f.write(label.encode("utf-8") + b"\x00")

