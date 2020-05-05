#!/usr/bin/python
"""Convert IRremoteESP8266's Raw data output into Pronto Code."""
#
# Copyright 2020 David Conran
import argparse
import sys

def convert_rawdata(data_str):
  """Parse a C++ rawdata declaration into a list of values."""
  start = data_str.find('{')
  end = data_str.find('}')
  if end == -1:
    end = len(data_str)
  if start > end:
    raise ValueError("Raw Data not parsible due to parentheses placement.")
  data_str = data_str[start + 1:end]
  results = []
  for timing in [x.strip() for x in data_str.split(',')]:
    try:
      results.append(int(timing))
    except ValueError:
      raise ValueError(
          "Raw Data contains a non-numeric value of '%s'." % timing)
  return results

def parse_and_report(rawdata_str, hz, verbose, output=sys.stdout):
  """Analyse the rawdata c++ definition of a IR message."""

  # Parse the input.
  rawdata = convert_rawdata(rawdata_str)
  if verbose:
    output.write("Found %d timing entries.\n" % len(rawdata))

  result = ["0000"]
  # Work out the frequency code.
  pronto_freq = int(1000000.0 / (hz * 0.241246))
  if verbose:
    output.write("Pronto frequency is %X (%d Hz).\n" % (pronto_freq, hz))
  result.append("%04X" % pronto_freq)
  period = 1000000.0/max(1, hz)
  if verbose:
    output.write("Pronto period is %f uSecs.\n" % period)
  # Add the lengths to the code.
  result.append("%04X" % len(rawdata))  # Initial burst
  result.append("%04X" % 0)  # Repeat (not used by this program)

  # Add the data.
  for i in rawdata:
    result.append("%04X" % (int(i) / period))
  output.write("Pronto code = '%s'\n" % " ".join(result))


def main():
  """Parse the commandline arguments and call the method."""
  arg_parser = argparse.ArgumentParser(
      description="Read an IRremoteESP8266 rawData declaration and tries to "
      "convert it in to a Pronto code.",
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  arg_parser.add_argument(
      "--hz",
      type=int,
      help="Frequency of the protocol to use in code generation. E.g. 38000Hz",
      dest="hz",
      required=True)
  arg_parser.add_argument(
      "-v",
      "--verbose",
      help="Increase output verbosity",
      action="store_true",
      dest="verbose",
      default=False)
  arg_group = arg_parser.add_mutually_exclusive_group(required=True)
  arg_group.add_argument(
      "rawdata",
      help="A rawData line from IRrecvDumpV2. e.g. 'uint16_t rawbuf[37] = {"
      "7930, 3952, 494, 1482, 520, 1482, 494, 1508, 494, 520, 494, 1482, 494, "
      "520, 494, 1482, 494, 1482, 494, 3978, 494, 520, 494, 520, 494, 520, "
      "494, 520, 520, 520, 494, 520, 494, 520, 494, 520, 494};'",
      nargs="?")
  arg_group.add_argument(
      "-f", "--file", help="Read in a rawData line from the file.")
  arg_group.add_argument(
      "--stdin",
      help="Read in a rawData line from STDIN.",
      action="store_true",
      default=False)
  arg_options = arg_parser.parse_args()

  if arg_options.stdin:
    data = sys.stdin.read()
  elif arg_options.file:
    with open(arg_options.file) as input_file:
      data = input_file.read()
  else:
    data = arg_options.rawdata
  parse_and_report(data, arg_options.hz, arg_options.verbose)


if __name__ == '__main__':
  main()
