#!/usr/bin/python
"""Attempt an automatic analysis of IRremoteESP8266's Raw data output.
   Makes suggestions on key values and tried to break down the message
   into likely chunks."""
#
# Copyright 2018 David Conran
import argparse
import sys


class RawIRMessage():
  """Basic analyse functions & structure for raw IR messages."""

  # pylint: disable=too-many-instance-attributes

  def __init__(self, margin, timings, output=sys.stdout, verbose=True):
    self.hdr_mark = None
    self.hdr_space = None
    self.bit_mark = None
    self.zero_space = None
    self.one_space = None
    self.gaps = []
    self.margin = margin
    self.marks = []
    self.mark_buckets = {}
    self.spaces = []
    self.space_buckets = {}
    self.output = output
    self.verbose = verbose
    self.rawlen = len(timings)
    if self.rawlen <= 3:
      raise ValueError("Too few message timings supplied.")
    self.timings = timings
    self._generate_timing_candidates()
    self._calc_values()

  def _generate_timing_candidates(self):
    """Determine the likely values from the given data."""
    count = 0
    for usecs in self.timings:
      count = count + 1
      if count % 2:
        self.marks.append(usecs)
      else:
        self.spaces.append(usecs)
    self.marks, self.mark_buckets = self.reduce_list(self.marks)
    self.spaces, self.space_buckets = self.reduce_list(self.spaces)

  def reduce_list(self, items):
    """Reduce a list of numbers into buckets that are at least margin apart."""
    result = []
    last = -1
    buckets = {}
    for item in sorted(items, reverse=True):
      if last == -1 or item < last - self.margin:
        result.append(item)
        last = item
        buckets[last] = [item]
      else:
        buckets[last].append(item)
    return result, buckets

  def _usec_compare(self, seen, expected):
    """Compare two usec values and see if they match within a
       subtractive margin."""
    return expected - self.margin < seen <= expected

  def _usec_compares(self, usecs, expecteds):
    """Compare a usec value to a list of values and return True
       if they are within a subtractive margin."""
    for expected in expecteds:
      if self._usec_compare(usecs, expected):
        return True
    return False

  def display_binary(self, binary_str):
    """Display common representations of the suppied binary string."""
    num = int(binary_str, 2)
    bits = len(binary_str)
    rev_binary_str = binary_str[::-1]
    rev_num = int(rev_binary_str, 2)
    self.output.write("\n  Bits: %d\n"
                      "  Hex:  %s (MSB first)\n"
                      "        %s (LSB first)\n"
                      "  Dec:  %s (MSB first)\n"
                      "        %s (LSB first)\n"
                      "  Bin:  0b%s (MSB first)\n"
                      "        0b%s (LSB first)\n" %
                      (bits, ("0x{0:0%dX}" % (bits / 4)).format(num),
                       ("0x{0:0%dX}" % (bits / 4)).format(rev_num), num,
                       rev_num, binary_str, rev_binary_str))

  def add_data_code(self, bin_str, name="", footer=True):
    """Add the common "data" sequence of code to send the bulk of a message."""
    # pylint: disable=no-self-use
    code = []
    nbits = len(bin_str)
    code.append("    // Data")
    code.append("    // e.g. data = 0x%X, nbits = %d" % (int(bin_str, 2),
                                                         nbits))
    code.append("    sendData(k%sBitMark, k%sOneSpace, k%sBitMark, "
                "k%sZeroSpace, data, %d, true);" %
                (name, name, name, name, nbits))
    if footer:
      code.append("    // Footer")
      code.append("    mark(k%sBitMark);" % name)
    return code

  def add_data_decode_code(self, bin_str, name="", footer=True):
    """Add the common "data" sequence code to decode the bulk of a message."""
    # pylint: disable=no-self-use
    code = []
    nbits = len(bin_str)
    code.extend([
        "",
        "  // Data",
        "  // e.g. data_result.data = 0x%X, nbits = %d" % (int(bin_str, 2),
                                                           nbits),
        "  data_result = matchData(&(results->rawbuf[offset]), %s," % nbits,
        "                          k%sBitMark, k%sOneSpace," % (name, name),
        "                          k%sBitMark, k%sZeroSpace);" % (name, name),
        "  offset += data_result.used;",
        "  if (data_result.success == false) return false;  // Fail",
        "  data <<= %s;  // Make room for the new bits of data." % nbits,
        "  data |= data_result.data;"])
    if footer:
      code.extend([
          "",
          "  // Footer",
          "  if (!matchMark(results->rawbuf[offset++], k%sBitMark))" % name,
          "    return false;"])
    return code

  def _calc_values(self):
    """Calculate the values which describe the standard timings
       for the protocol."""
    if self.verbose:
      self.output.write("Potential Mark Candidates:\n"
                        "%s\n"
                        "Potential Space Candidates:\n"
                        "%s\n" % (str(self.marks), str(self.spaces)))
    # Largest mark is likely the kHdrMark
    self.hdr_mark = self.marks[0]
    # The bit mark is likely to be the smallest mark.
    self.bit_mark = self.marks[-1]

    if self.is_space_encoded() and len(self.spaces) >= 3:
      if self.verbose and len(self.marks) > 2:
        self.output.write("DANGER: Unexpected and unused mark timings!")
      # We should have 3 space candidates at least.
      # They should be: zero_space (smallest), one_space, & hdr_space (largest)
      spaces = list(self.spaces)
      self.zero_space = spaces.pop()
      self.one_space = spaces.pop()
      self.hdr_space = spaces.pop()
      # Rest are probably message gaps
      self.gaps = spaces

  def is_space_encoded(self):
    """Make an educated guess if the message is space encoded."""
    return len(self.spaces) > len(self.marks)

  def is_hdr_mark(self, usec):
    """Is usec the header mark?"""
    return self._usec_compare(usec, self.hdr_mark)

  def is_hdr_space(self, usec):
    """Is usec the header space?"""
    return self._usec_compare(usec, self.hdr_space)

  def is_bit_mark(self, usec):
    """Is usec the bit mark?"""
    return self._usec_compare(usec, self.bit_mark)

  def is_one_space(self, usec):
    """Is usec the one space?"""
    return self._usec_compare(usec, self.one_space)

  def is_zero_space(self, usec):
    """Is usec the zero_space?"""
    return self._usec_compare(usec, self.zero_space)

  def is_gap(self, usec):
    """Is usec the a space gap?"""
    return self._usec_compares(usec, self.gaps)


def avg_list(items):
  """Return the average of a list of numbers."""
  if items:
    return int(sum(items) / len(items))
  return 0


def add_bit(so_far, bit, output=sys.stdout):
  """Add a bit to the end of the bits collected so far."""
  if bit == "reset":
    return ""
  output.write(str(bit))  # This effectively displays in LSB first order.
  return so_far + str(bit)  # Storing it in MSB first order.


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


def dump_constants(message, defines, name="", output=sys.stdout):
  """Dump the key constants and generate the C++ #defines."""
  hdr_mark = avg_list(message.mark_buckets[message.hdr_mark])
  bit_mark = avg_list(message.mark_buckets[message.bit_mark])
  hdr_space = avg_list(message.space_buckets[message.hdr_space])
  one_space = avg_list(message.space_buckets[message.one_space])
  zero_space = avg_list(message.space_buckets[message.zero_space])

  output.write("Guessing key value:\n"
               "k%sHdrMark   = %d\n"
               "k%sHdrSpace  = %d\n"
               "k%sBitMark   = %d\n"
               "k%sOneSpace  = %d\n"
               "k%sZeroSpace = %d\n" % (name, hdr_mark, name, hdr_space,
                                        name, bit_mark, name, one_space,
                                        name, zero_space))
  defines.append("const uint16_t k%sHdrMark = %d;" % (name, hdr_mark))
  defines.append("const uint16_t k%sBitMark = %d;" % (name, bit_mark))
  defines.append("const uint16_t k%sHdrSpace = %d;" % (name, hdr_space))
  defines.append("const uint16_t k%sOneSpace = %d;" % (name, one_space))
  defines.append("const uint16_t k%sZeroSpace = %d;" % (name, zero_space))

  avg_gaps = [avg_list(message.space_buckets[x]) for x in message.gaps]
  if len(message.gaps) == 1:
    output.write("k%sSpaceGap = %d\n" % (name, avg_gaps[0]))
    defines.append("const uint16_t k%sSpaceGap = %d;" % (name, avg_gaps[0]))
  else:
    count = 0
    for gap in avg_gaps:
      # We probably (still) have a gap in the protocol.
      count = count + 1
      output.write("k%sSpaceGap%d = %d\n" % (name, count, gap))
      defines.append("const uint16_t k%sSpaceGap%d = %d;" % (name, count, gap))


def parse_and_report(rawdata_str, margin, gen_code=False, name="",
                     output=sys.stdout):
  """Analyse the rawdata c++ definition of a IR message."""
  defines = []
  code = {}
  code["send"] = []
  code["recv"] = []

  # Parse the input.
  rawdata = convert_rawdata(rawdata_str)

  output.write("Found %d timing entries.\n" % len(rawdata))

  message = RawIRMessage(margin, rawdata, output)
  output.write("\nGuessing encoding type:\n")
  if message.is_space_encoded():
    output.write("Looks like it uses space encoding. Yay!\n\n")
    dump_constants(message, defines, name, output)
  else:
    output.write("Sorry, it looks like it is Mark encoded. "
                 "I can't do that yet. Exiting.\n")
    sys.exit(1)
  total_bits = decode_data(message, defines, code, name, output)
  if gen_code:
    generate_code(defines, code, total_bits, name, output)


def decode_data(message, defines, code, name="", output=sys.stdout):
  """Decode the data sequence with the given values in mind."""
  # pylint: disable=too-many-branches,too-many-statements

  # Now we have likely candidates for the key values, go through the original
  # sequence and break it up and indicate accordingly.

  output.write("\nDecoding protocol based on analysis so far:\n\n")
  state = ""
  count = 1
  total_bits = ""
  binary_value = add_bit("", "reset")
  if name:
    def_name = name
  else:
    def_name = "TBD"

  code["send"].extend([
      "#if SEND_%s" % def_name.upper(),
      "// Function should be safe up to 64 bits.",
      "void IRsend::send%s(const uint64_t data, const uint16_t"
      " nbits, const uint16_t repeat) {" % def_name,
      "  enableIROut(38);  // A guess. Most common frequency.",
      "  for (uint16_t r = 0; r <= repeat; r++) {"
  ])
  code["recv"].extend([
      "#if DECODE_%s" % def_name.upper(),
      "// Function should be safe up to 64 bits.",
      "bool IRrecv::decode%s(decode_results *results, const uint16_t nbits,"
      " const bool strict) {" % def_name,
      "  if (results->rawlen < 2 * nbits + k%sOverhead)" % name,
      "    return false;  // Too short a message to match.",
      "  if (strict && nbits != k%sBits)" % name,
      "    return false;",
      "",
      "  uint16_t offset = kStartOffset;",
      "  uint64_t data = 0;",
      "  match_result_t data_result;"
  ])

  for usec in message.timings:
    if (message.is_hdr_mark(usec) and count % 2 and
        not message.is_bit_mark(usec)):
      state = "HM"
      if binary_value:
        message.display_binary(binary_value)
        code["send"].extend(message.add_data_code(binary_value, name, False))
        code["recv"].extend(message.add_data_decode_code(binary_value, name,
                                                         False))
        total_bits = total_bits + binary_value
      binary_value = add_bit(binary_value, "reset")
      output.write("k%sHdrMark+" % name)
      code["send"].extend(["    // Header", "    mark(k%sHdrMark);" % name])
      code["recv"].extend([
          "",
          "  // Header",
          "  if (!matchMark(results->rawbuf[offset++], k%sHdrMark))" % name,
          "    return false;"])

    elif message.is_hdr_space(usec) and not message.is_one_space(usec):
      if state != "HM":
        if binary_value:
          message.display_binary(binary_value)
          total_bits = total_bits + binary_value
          code["send"].extend(message.add_data_code(binary_value, name))
          code["recv"].extend(message.add_data_decode_code(binary_value, name))
        binary_value = add_bit(binary_value, "reset")
        output.write("UNEXPECTED->")
      state = "HS"
      output.write("k%sHdrSpace+" % name)
      code["send"].append("    space(k%sHdrSpace);" % name)
      code["recv"].extend([
          "  if (!matchSpace(results->rawbuf[offset++], k%sHdrSpace))" % name,
          "    return false;"])
    elif message.is_bit_mark(usec) and count % 2:
      if state not in ("HS", "BS"):
        output.write("k%sBitMark(UNEXPECTED)" % name)
      state = "BM"
    elif message.is_zero_space(usec):
      if state != "BM":
        output.write("k%sZeroSpace(UNEXPECTED)" % name)
      state = "BS"
      binary_value = add_bit(binary_value, 0, output)
    elif message.is_one_space(usec):
      if state != "BM":
        output.write("k%sOneSpace(UNEXPECTED)" % name)
      state = "BS"
      binary_value = add_bit(binary_value, 1, output)
    elif message.is_gap(usec):
      if state != "BM":
        output.write("UNEXPECTED->")
      state = "GS"
      output.write("GAP(%d)" % usec)
      if binary_value:
        message.display_binary(binary_value)
        code["send"].extend(message.add_data_code(binary_value, name))
        code["recv"].extend(message.add_data_decode_code(binary_value, name))
      else:
        code["send"].extend(["    // Gap", "    mark(k%sBitMark);" % name])
        code["recv"].extend([
            "",
            "  // Gap",
            "  if (!matchMark(results->rawbuf[offset++], k%sBitMark))" % name,
            "    return false;"])
      code["send"].append("    space(k%sSpaceGap);" % name)
      code["recv"].extend([
          "  if (!matchSpace(results->rawbuf[offset++], k%sSpaceGap))" % name,
          "    return false;"])
      total_bits = total_bits + binary_value
      binary_value = add_bit(binary_value, "reset")
    else:
      output.write("UNKNOWN(%d)" % usec)
      state = "UNK"
    count = count + 1
  if binary_value:
    message.display_binary(binary_value)
    code["send"].extend(message.add_data_code(binary_value, name))
    code["recv"].extend(message.add_data_decode_code(binary_value, name))
  code["send"].extend([
      "    space(kDefaultMessageGap);  // A 100% made up guess of the gap"
      " between messages.",
      "  }",
      "}",
      "#endif  // SEND_%s" % def_name.upper()])
  code["recv"].extend([
      "",
      "  // Success",
      "  results->decode_type = decode_type_t::%s;" % def_name.upper(),
      "  results->bits = nbits;",
      "  results->value = data;",
      "  results->command = 0;",
      "  results->address = 0;",
      "  return true;",
      "}",
      "#endif  // DECODE_%s" % def_name.upper()])

  total_bits = total_bits + binary_value
  output.write("\nTotal Nr. of suspected bits: %d\n" % len(total_bits))
  defines.append("const uint16_t k%sBits = %d;"
                 "  // Move to IRremoteESP8266.h" % (name, len(total_bits)))
  if len(total_bits) > 64:
    defines.append("const uint16_t k%sStateLength = %d;"
                   "  // Move to IRremoteESP8266.h" %
                   (name, len(total_bits) / 8))
  defines.append("const uint16_t k%sOverhead = %d;" %
                 (name, message.rawlen - 2 * len(total_bits)))
  return total_bits


def generate_code(defines, code, bits_str, name="", output=sys.stdout):
  """Output the estimated C++ code to reproduce & decode the IR message."""
  if name:
    def_name = name
  else:
    def_name = "TBD"
  output.write("\nGenerating a VERY rough code outline:\n\n"
               "// Copyright 2019 David Conran (crankyoldgit)\n"
               "// Support for %s protocol\n\n"
               '#include "IRrecv.h"\n'
               '#include "IRsend.h"\n'
               '#include "IRutils.h"\n\n'
               "// WARNING: This probably isn't directly usable."
               " It's a guide only.\n\n"
               "// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/"
               "Adding-support-for-a-new-IR-protocol\n"
               "// for details of how to include this in the library."
               "\n" % def_name)
  for line in defines:
    output.write("%s\n" % line)

  if len(bits_str) > 64:  # Will it fit in a uint64_t?
    output.write("// DANGER: More than 64 bits detected. A uint64_t for "
                 "'data' won't work!\n")
  # Display the "normal" version's send code incase there are some
  # oddities in it.
  for line in code["send"]:
    output.write("%s\n" % line)

  if len(bits_str) > 64:  # Will it fit in a uint64_t?
    output.write("\n\n#if SEND_%s\n"
                 "// Alternative >64 bit Function\n"
                 "void IRsend::send%s(uint8_t data[], uint16_t nbytes,"
                 " uint16_t repeat) {\n"
                 "  // nbytes should typically be k%sStateLength\n"
                 "  // data should typically be:\n"
                 "  //   uint8_t data[k%sStateLength] = {0x%s};\n"
                 "  // data[] is assumed to be in MSB order for this code.\n"
                 "  for (uint16_t r = 0; r <= repeat; r++) {\n"
                 "    sendGeneric(k%sHdrMark, k%sHdrSpace,\n"
                 "                k%sBitMark, k%sOneSpace,\n"
                 "                k%sBitMark, k%sZeroSpace,\n"
                 "                k%sBitMark,\n"
                 "                kDefaultMessageGap, // 100%% made-up guess at"
                 " the message gap.\n"
                 "                data, nbytes,\n"
                 "                38000, // Complete guess of the modulation"
                 " frequency.\n"
                 "                true, 0, 50);\n"
                 "  }\n"
                 "}\n"
                 "#endif  // SEND_%s\n" % (
                     def_name.upper(), def_name, name, name,
                     ", 0x".join("%02X" % int(bits_str[i:i + 8], 2)
                                 for i in range(0, len(bits_str), 8)),
                     name, name, name, name, name, name, name,
                     def_name.upper()))

  output.write("\n")
  if len(bits_str) > 64:  # Will it fit in a uint64_t?
    output.write("// DANGER: More than 64 bits detected. A uint64_t for "
                 "'data' won't work!\n")
  # Display the "normal" version's decode code incase there are some
  # oddities in it.
  for line in code["recv"]:
    output.write("%s\n" % line)


def main():
  """Parse the commandline arguments and call the method."""
  arg_parser = argparse.ArgumentParser(
      description="Read an IRremoteESP8266 rawData declaration and tries to "
      "analyse it.",
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  arg_parser.add_argument(
      "-g",
      "--code",
      action="store_true",
      default=False,
      dest="gen_code",
      help="Generate a C++ code outline to aid making an IRsend function.")
  arg_parser.add_argument(
      "-n",
      "--name",
      help="Name of the protocol/device to use in code generation. E.g. Onkyo",
      dest="name",
      default="")
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
  arg_parser.add_argument(
      "-r",
      "--range",
      type=int,
      help="Max number of micro-seconds difference between values to consider"
      " it the same value.",
      dest="margin",
      default=200)
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
  parse_and_report(data, arg_options.margin, arg_options.gen_code,
                   arg_options.name)


if __name__ == '__main__':
  main()
