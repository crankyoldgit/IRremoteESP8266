#!/usr/bin/env python3

import pathlib
import argparse
import sys
import re

BRAND_MODEL = re.compile("Brand: *(?P<brand>.+), *Model: *(?P<model>.+)")

SEARCH_EXT = [".cpp", ".h"]


def getargs():
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--markdown", help="output markdown", action="store_true")
    parser.add_argument(
        "-v", "--verbose", help="increase output verbosity", action="store_true"
    )
    parser.add_argument(
        "-a",
        "--alert",
        help="alert if a file does not have a supports section",
        action="store_true",
    )
    parser.add_argument(
        "directory",
        nargs="?",
        help="directory of the source git checkout",
        default="../",
    )
    args = parser.parse_args()
    return args


def exit(msg):
    sys.stderr.write("{}\n".format(msg))
    sys.exit(1)


def extractsupports(fn):
    supports = []
    insupports = False
    for line in fn.open():
        if not line.startswith("//"):
            continue
        line = line[2:].strip()
        if line.lower() in ["supports:", "supported:"]:
            insupports = True
            continue
        if insupports:
            match = BRAND_MODEL.match(line)
            if match:
                supports.append((match.group("brand"), match.group("model")))
            else:
                insupports = False
                continue
    return supports


def main():
    args = getargs()
    src = pathlib.Path(args.directory) / "src"
    if not src.is_dir():
        exit("Directory not valid: {}".format(str(src)))
    if args.verbose:
        print("Looking for files in: {}".format(str(src.resolve())))
    allcodes = {}
    fnnomatch = set()
    fnmatch = set()
    for fn in src.iterdir():
        if fn.suffix not in SEARCH_EXT:
            continue
        if not fn.name.startswith("ir_"):
            continue
        supports = extractsupports(fn)
        if args.alert:
            if supports:
                fnmatch.add(fn.stem)
            else:
                fnnomatch.add(fn.stem)
        for brand, model in supports:
            allcodes[brand] = allcodes.get(brand, list()) + [model]

    allbrands = list(allcodes.keys())
    allbrands.sort()

    if args.markdown:
        print("| Brand | Model |")
        print("| --- | --- |")

    for brand in allbrands:
        codes = allcodes[brand]
        codes.sort()
        if args.markdown:
            print("| **{}** | {} |".format(brand, "<BR>".join(codes)))
        else:
            print("{}:\n\t{}".format(brand, "\n\t".join(codes)))

    if args.alert:
        nosupports = fnnomatch - fnmatch
        if nosupports:
            nosupports = list(nosupports)
            nosupports.sort()
            print("The following files had no supports section:")
            for fn in nosupports:
                print("\t{}".format(fn))


if __name__ == "__main__":
    main()
