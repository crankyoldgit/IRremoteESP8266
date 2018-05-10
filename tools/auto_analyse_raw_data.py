#!/usr/bin/python
"""Attempt an automatic analysis of IRremoteESP8266's Raw data output.
   Makes suggestions on key values and tried to break down the message
   into likely chuncks."""
#
# Copyright 2018 David Conran
import argparse
import sys


class Defs(object):
  """House the parameters for a message."""

  # pylint: disable=too-many-instance-attributes

  def __init__(self, margin):
    self.hdr_mark = None
    self.hdr_space = None
    self.bit_mark = None
    self.zero_space = None
    self.one_space = None
    self.gaps = []
    self.margin = margin
    self.marks = []
    self.spaces = []

  def process_data(self, data):
    """Determine the values from the given data."""
    count = 0
    for usecs in data:
      count = count + 1
      if is_odd(count):
        self.marks.append(usecs)
      else:
        self.spaces.append(usecs)
    self.marks = self.reduce_list(self.marks)
    self.spaces = self.reduce_list(self.spaces)

  def reduce_list(self, items):
    """Reduce the list of numbers into buckets that are atleast margin apart."""
    result = []
    last = -1
    for item in sorted(items, reverse=True):
      if last == -1 or item < last - self.margin:
        result.append(item)
        last = item
    return result


def usec_compare(seen, expected, margin):
  """Compare two usec values and see if they match within a
     subtrative margin."""
  return seen <= expected and seen > expected - margin


def usec_compares(usecs, expecteds, margin):
  """Compare a usec value to a list of values and return True
     if they are within a subtractive margin."""
  for expected in expecteds:
    if usec_compare(usecs, expected, margin):
      return True
  return False


def add_bit(so_far, bit):
  """Add a bit to the end of the bits collected so far. """
  if bit == "reset":
    return ""
  sys.stdout.write(str(bit))  # This effectively displays in LSB first order.
  return so_far + str(bit)  # Storing it in MSB first order.


def is_odd(num):
  """Is the num odd? i.e. Not even."""
  return num % 2


def display_bin_value(binary_str, output=sys.stdout):
  """Display common representations of the suppied binary string."""
  num = int(binary_str, 2)
  bits = len(binary_str)
  rev_binary_str = binary_str[::-1]
  rev_num = int(rev_binary_str, 2)
  output.write("\n  Bits: %d\n" % bits)
  output.write("  Hex:  %s (MSB first)\n" % "0x{0:0{1}X}".format(num, bits / 4))
  output.write(
      "        %s (LSB first)\n" % "0x{0:0{1}X}".format(rev_num, bits / 4))
  output.write("  Dec:  %s (MSB first)\n" % num)
  output.write("        %s (LSB first)\n" % rev_num)
  output.write("  Bin:  0b%s (MSB first)\n" % binary_str)
  output.write("        0b%s (LSB first)\n" % rev_binary_str)


def add_data_code(bin_str):
  """Add the common "data" sequence of code to send the bulk of a message."""
  code = []
  code.append("    // Data")
  code.append("    // e.g. data = 0x%X, nbits = %d" % (int(bin_str, 2),
                                                       len(bin_str)))
  code.append("    sendData(BIT_MARK, ONE_SPACE, BIT_MARK, ZERO_SPACE, data, "
              "nbits, true);")
  code.append("    // Footer")
  code.append("    mark(BIT_MARK);")
  return code


def convert_rawdata(data_str):
  """Parse a C++ rawdata declaration into a list of values."""
  start = data_str.find('{')
  end = data_str.find('}')
  data_str = data_str[start + 1:end]
  return [int(x.strip()) for x in data_str.split(',')]


def parse_and_report(rawdata_str, margin, gen_code=False, output=sys.stdout):
  """Analyse the rawdata c++ definition of a IR message."""
  code_defs = []
  code_64bit = []
  defs = Defs(margin)

  # Parse the input.
  rawdata = convert_rawdata(rawdata_str)

  defs.process_data(rawdata)
  output.write("Potential Mark Candidates:\n"
               "%s\n"
               "Potential Space Candidates:\n"
               "%s\n" % (str(defs.marks), str(defs.spaces)))
  output.write("\n\nGuessing encoding type:\n")
  if len(defs.spaces) > len(defs.marks):
    output.write("Looks like it uses space encoding. Yay!\n\n"
                 "Guessing key value:\n")

    # Largest mark is likely the HDR_MARK
    defs.hdr_mark = defs.marks[0]
    output.write("HDR_MARK   = %d\n" % defs.hdr_mark)
    code_defs.append("#define HDR_MARK %dU" % defs.hdr_mark)
    # The mark bit is likely to be the smallest.
    defs.bit_mark = defs.marks[-1]
    output.write("BIT_MARK   = %d\n" % defs.bit_mark)
    code_defs.append("#define BIT_MARK %dU" % defs.bit_mark)

    gap = 0
    defs.gaps = []
    while len(defs.spaces) > 3:
      # We probably (still) have a gap in the protocol.
      gap = gap + 1
      space = defs.spaces.remove()
      defs.gaps.append(space)
      output.write("SPACE_GAP%d = %d\n" % (gap, space))
      code_defs.append("#define SPACE_GAP%d = %dU" % (gap, space))

    # We should have 3 space candidates left.
    # They should be zero_space (smallest), one_space, & hdr_space (largest)
    defs.zero_space = defs.spaces.pop()
    defs.one_space = defs.spaces.pop()
    defs.hdr_space = defs.spaces.pop()
    code_defs.append("#define HDR_SPACE %dU" % defs.hdr_space)
    code_defs.append("#define ONE_SPACE %dU" % defs.one_space)
    code_defs.append("#define ZERO_SPACE %dU" % defs.zero_space)
    output.write("HDR_SPACE  = %d\n"
                 "ONE_SPACE  = %d\n"
                 "ZERO_SPACE = %d\n" % (defs.hdr_space, defs.one_space,
                                        defs.zero_space))
  else:
    output.write("Sorry, it looks like it is Mark encoded. "
                 "I can't do that yet. Exiting.\n")
    sys.exit(1)
  total_bits = decode_data(rawdata, defs, code_defs, code_64bit, output)
  if gen_code:
    generate_code(code_defs, code_64bit, total_bits, output)


def decode_data(data, defs, code_defs, code_64bit, output=sys.stdout):
  """Decode the data sequence with the given values in mind."""
  # pylint: disable=too-many-branches,too-many-statements

  # Now we have likely candidates for the key values, go through the original
  # sequence and break it up and indicate accordingly.

  output.write("\nDecoding protocol based on analysis so far:\n\n")
  last = ""
  count = 1
  total_bits = ""
  binary_value = add_bit("", "reset")

  code_64bit.extend([
      "// Function should be safe up to 64 bits.",
      "void IRsend::sendXYZ(const uint64_t data, const uint16_t"
      " nbits, const uint16_t repeat) {",
      "  enableIROut(38);  // A guess. Most common frequency.",
      "  for (uint16_t r = 0; r <= repeat; r++) {"
  ])

  for usecs in data:
    if (usec_compare(usecs, defs.hdr_mark, defs.margin) and is_odd(count) and
        not usec_compare(usecs, defs.bit_mark, defs.margin)):
      last = "HM"
      if binary_value:
        display_bin_value(binary_value)
        total_bits = total_bits + binary_value
        output.write(last)
      binary_value = add_bit(binary_value, "reset")
      output.write("HDR_MARK+")
      code_64bit.extend(["    // Header", "    mark(HDR_MARK);"])
    elif (usec_compare(usecs, defs.hdr_space, defs.margin) and
          not usec_compare(usecs, defs.one_space, defs.margin)):
      if last != "HM":
        if binary_value:
          display_bin_value(binary_value)
          total_bits = total_bits + binary_value
          code_64bit.extend(add_data_code(binary_value))
        binary_value = add_bit(binary_value, "reset")
        output.write("UNEXPECTED->")
      last = "HS"
      output.write("HDR_SPACE+")
      code_64bit.append("    space(HDR_SPACE);")
    elif usec_compare(usecs, defs.bit_mark, defs.margin) and is_odd(count):
      if last != "HS" and last != "BS":
        output.write("BIT_MARK(UNEXPECTED)")
      last = "BM"
    elif usec_compare(usecs, defs.zero_space, defs.margin):
      if last != "BM":
        output.write("ZERO_SPACE(UNEXPECTED)")
      last = "BS"
      binary_value = add_bit(binary_value, 0)
    elif usec_compare(usecs, defs.one_space, defs.margin):
      if last != "BM":
        output.write("ONE_SPACE(UNEXPECTED)")
      last = "BS"
      binary_value = add_bit(binary_value, 1)
    elif usec_compares(usecs, defs.gaps, defs.margin):
      if last != "BM":
        output.write("UNEXPECTED->")
      last = "GS"
      output.write(" GAP(%d)" % usecs)
      display_bin_value(binary_value)
      code_64bit.extend(add_data_code(binary_value))
      code_64bit.append("    space(SPACE_GAP);")
      total_bits = total_bits + binary_value
      binary_value = add_bit(binary_value, "reset")
    else:
      output.write("UNKNOWN(%d)" % usecs)
      last = "UNK"
    count = count + 1
  display_bin_value(binary_value)
  code_64bit.extend(add_data_code(binary_value))
  code_64bit.extend([
      "    space(100000);  // A 100% made up guess of the gap"
      " between messages.", "  }", "}"
  ])

  total_bits = total_bits + binary_value
  output.write("Total Nr. of suspected bits: %d\n" % len(total_bits))
  code_defs.append("#define XYZ_BITS %dU" % len(total_bits))
  if len(total_bits) > 64:
    code_defs.append("#define XYZ_STATE_LENGTH %dU" % (len(total_bits) / 8))
  return total_bits


def generate_code(defs, normal, bits_str, output=sys.stdout):
  """Output the estimated C++ code to reproduce the IR message."""
  output.write("\nGenerating a VERY rough code outline:\n\n"
               "// WARNING: This probably isn't directly usable."
               " It's a guide only.\n")
  for line in defs:
    output.write("%s\n" % line)

  if len(bits_str) > 64:  # Will it fit in a uint64_t?
    output.write("// DANGER: More than 64 bits detected. A uint64_t for "
                 "'data' won't work!\n")
  # Display the "normal" version's code incase there are some
  # oddities in it.
  for line in normal:
    output.write("%s\n" % line)

  if len(bits_str) > 64:  # Will it fit in a uint64_t?
    output.write("\n\n// Alternative >64 bit Function\n"
                 "void IRsend::sendXYZ(uint8_t data[], uint16_t nbytes,"
                 " uint16_t repeat) {\n"
                 "  // nbytes should typically be XYZ_STATE_LENGTH\n"
                 "  // data should typically be:\n"
                 "  //   uint8_t data[XYZ_STATE_LENGTH] = {0x%s};\n"
                 "  // data[] is assumed to be in MSB order for this code.\n"
                 "  for (uint16_t r = 0; r <= repeat; r++) {\n"
                 "    sendGeneric(HDR_MARK, HDR_SPACE,\n"
                 "                BIT_MARK, ONE_SPACE,\n"
                 "                BIT_MARK, ZERO_SPACE,\n"
                 "                BIT_MARK\n"
                 "                100000, // 100%% made-up guess at the"
                 " message gap.\n"
                 "                data, nbytes,\n"
                 "                38000, // Complete guess of the modulation"
                 " frequency.\n"
                 "                true, 0, 50);\n"
                 "}\n" % ", 0x".join("%02X" % int(bits_str[i:i + 8], 2)
                                     for i in range(0, len(bits_str), 8)))


# Main program

ARG_PARSER = argparse.ArgumentParser(
    description="Read an IRremoteESP8266 rawData declaration and tries to "
    "analyse it.",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
ARG_PARSER.add_argument(
    "-g",
    "--code",
    action="store_true",
    default=False,
    dest="gen_code",
    help="Generate a C++ code outline to aid making an IRsend function.")
ARG_GROUP = ARG_PARSER.add_mutually_exclusive_group(required=True)
ARG_GROUP.add_argument(
    "rawdata",
    help="A rawData line from IRrecvDumpV2. e.g. 'uint16_t rawbuf[37] = {7930, "
    "3952, 494, 1482, 520, 1482, 494, 1508, 494, 520, 494, 1482, 494, 520,"
    " 494, 1482, 494, 1482, 494, 3978, 494, 520, 494, 520, 494, 520, 494, "
    "520, 520, 520, 494, 520, 494, 520, 494, 520, 494};'",
    nargs="?")
ARG_GROUP.add_argument(
    "-f", "--file", help="Read in a rawData line from the file.")
ARG_PARSER.add_argument(
    "-r",
    "--range",
    type=int,
    help="Max number of micro-seconds difference between values to consider it "
    "the same value.",
    dest="margin",
    default=200)
ARG_GROUP.add_argument(
    "--stdin",
    help="Read in a rawData line from STDIN.",
    action="store_true",
    default=False)
ARG_OPTIONS = ARG_PARSER.parse_args()

if ARG_OPTIONS.stdin:
  DATA = sys.stdin.read()
elif ARG_OPTIONS.file:
  with open(ARG_OPTIONS.file) as f:
    DATA = f.read()
else:
  DATA = ARG_OPTIONS.rawdata
parse_and_report(DATA, ARG_OPTIONS.margin, ARG_OPTIONS.gen_code)
