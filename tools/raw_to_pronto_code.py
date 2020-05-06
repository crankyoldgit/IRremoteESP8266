#!/usr/bin/python
"""Convert IRremoteESP8266's Raw data output into Pronto Code."""
#
# Copyright 2020 David Conran
import argparse
import sys
from auto_analyse_raw_data import convert_rawdata, add_rawdata_args, get_rawdata


def parse_and_report(rawdata_str, hertz, end_usecs, verbose, output=sys.stdout):
  """Analyse the rawdata c++ definition of a IR message."""

  # Parse the input.
  rawdata = convert_rawdata(rawdata_str)
  if verbose:
    output.write("Found %d timing entries.\n" % len(rawdata))

  # Do we need to pad out the rawdata to make it even in length?
  if end_usecs > 0 and len(rawdata) % 2 == 1:
    rawdata.append(end_usecs)

  result = ["0000"]
  # Work out the frequency code.
  pronto_freq = int(1000000.0 / (hertz * 0.241246))
  if verbose:
    output.write("Pronto frequency is %X (%d Hz).\n" % (pronto_freq, hertz))
  result.append("%04X" % pronto_freq)
  period = 1000000.0 / max(1, hertz)
  if verbose:
    output.write("Pronto period is %f uSecs.\n" % period)
  # Add the lengths to the code.
  result.append("%04X" % int(len(rawdata) / 2))  # Initial burst
  result.append("%04X" % 0)  # Repeat (not used by this program)

  # Add the data.
  if verbose:
    output.write("Raw data: %s " % rawdata)
  for i in rawdata:
    result.append("%04X" % int(i / period))
  output.write("Pronto code = '%s'\n" % " ".join(result))


def main():
  """Parse the commandline arguments and call the method."""
  arg_parser = argparse.ArgumentParser(
      description="Read an IRremoteESP8266 rawData declaration and tries to "
      "convert it in to a Pronto code.",
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  arg_parser.add_argument(
      "--hz",
      "--hertz",
      type=int,
      help="Frequency of the protocol to use in code generation. E.g. 38000Hz",
      dest="hertz",
      required=True)
  arg_parser.add_argument(
      "--gap",
      "--endgap",
      type=int,
      help="Nr. of uSeconds of gap to add to the end of the message.",
      dest="usecs",
      default=100000)
  arg_parser.add_argument(
      "-v",
      "--verbose",
      help="Increase output verbosity",
      action="store_true",
      dest="verbose",
      default=False)
  add_rawdata_args(arg_parser)
  arg_options = arg_parser.parse_args()
  parse_and_report(get_rawdata(arg_options), arg_options.hertz,
                   arg_options.usecs, arg_options.verbose)


if __name__ == '__main__':
  main()
