#!/usr/bin/python
# Attempt an automatic analysis of IRremoteESP8266's Raw data output.
# Makes suggestions on key values and tried to break down the message
# into likely chuncks.
#
# Copyright 2018 David Conran
import argparse
import sys

def usecCompare(seen, expected, margin):
  return (seen <= expected and seen > expected - margin)

def usecCompares(usecs, expecteds, margin):
  for expected in expecteds:
    if usecCompare(usecs, expected, margin):
      return True
  return False

def addBit(so_far, bit):
  global binary_value
  if bit == "reset":
    return ""
  sys.stdout.write(str(bit))  # This effectively displays in LSB first order.
  return so_far + str(bit)  # Storing it in MSB first order.

def isOdd(num):
  return num % 2

def displayBinaryValue(binary_str, output=sys.stdout):
  num = int(binary_str, 2)
  bits = len(binary_str)
  rev_binary_str = binary_str[::-1]
  rev_num = int(rev_binary_str, 2)
  output.write("\n  Bits: %d\n" % bits)
  output.write("  Hex:  %s (MSB first)\n" % "0x{0:0{1}X}".format(num, bits / 4))
  output.write("        %s (LSB first)\n" % "0x{0:0{1}X}".format(rev_num,
                                                              bits / 4))
  output.write("  Dec:  %s (MSB first)\n" % num)
  output.write("        %s (LSB first)\n" % rev_num)
  output.write("  Bin:  0b%s (MSB first)\n" % binary_str)
  output.write("        0b%s (LSB first)\n" % rev_binary_str)

def reduceList(items, margin):
  result = []
  last = -1
  for item in sorted(items, reverse=True):
    if last == -1 or item < last - margin:
      result.append(item)
      last = item
  return result

def ParseAndReport(rawdata_str, margin, gen_code=False, output=sys.stdout):
  marks = []
  spaces = []
  codeDefines = []
  code64Bit = []
  # Clean up the input
  start = rawdata_str.find('{')
  end = rawdata_str.find('}')
  rawdata_str = rawdata_str[start + 1:end]
  rawData = map(lambda x: int(x.strip()), rawdata_str.split(','))

  # Parse the input.
  count = 0
  for usecs in rawData:
    count = count + 1
    if isOdd(count):
      marks.append(usecs)
    else:
      spaces.append(usecs)

  mark_candidates = reduceList(marks, margin)
  space_candidates = reduceList(spaces, margin)
  output.write("Potential Mark Candidates (using a margin of %d):\n" % margin)
  output.write(str(mark_candidates))
  output.write("\n")
  output.write("Potential Space Candidates (using a margin of  %d):\n" % margin)
  output.write(str(space_candidates))
  output.write("\n\nGuessing encoding type:\n")
  if len(space_candidates) > len(mark_candidates):
    output.write("Looks like it uses space encoding. Yay!\n\n")
    output.write("Guessing key value:\n")

    # Largest mark is likely the HDR_MARK
    hdr_mark = mark_candidates[0]
    output.write("HDR_MARK   = %d\n" % hdr_mark)
    codeDefines.append("#define HDR_MARK %dU" % hdr_mark)
    # The mark bit is likely to be the smallest.
    bit_mark = mark_candidates[-1]
    output.write("BIT_MARK   = %d\n" % bit_mark)
    codeDefines.append("#define BIT_MARK %dU" % bit_mark)

    gap = 0
    gap_candidates = []
    while len(space_candidates) > 3:
      # We probably (still) have a gap in the protocol.
      gap = gap + 1
      space = space_candidates.remove()
      gap_candidates.append(space)
      output.write("SPACE_GAP%d = %d\n" % (gap, space))
      codeDefines.append("#define SPACE_GAP%d = %dU" % (gap, space))


    # We should have 3 space candidates left.
    # They should be zero_space (smallest), one_space, & hdr_space (largest)
    zero_space = space_candidates.pop()
    one_space = space_candidates.pop()
    hdr_space = space_candidates.pop()
    output.write("HDR_SPACE  = %d\n" % hdr_space)
    codeDefines.append("#define HDR_SPACE %dU" % hdr_space)
    output.write("ONE_SPACE  = %d\n" % one_space)
    codeDefines.append("#define ONE_SPACE %dU" % one_space)
    output.write("ZERO_SPACE = %d\n" % zero_space)
    codeDefines.append("#define ZERO_SPACE %dU" % zero_space)
  else:
    output.write("Sorry, it looks like it is Mark encoded. "
                     "I can't do that yet. Exiting.\n")
    sys.exit(1)


  # Now we have likely candidates for the key values, go through the original
  # sequence and break it up and indicate accordingly.

  output.write("\nDecoding protocol based on analysis so far:\n\n")
  last = ""
  count = 1
  data_count = 1
  last_binary_value = ""
  bits = 0
  total_bits = ""
  binary_value = addBit("", "reset")

  code64Bit.append("// Function should be safe upto 64 bits.")
  code64Bit.append("void IRsend::sendXYZ(const uint64_t data, const uint16_t"
                   " nbits, const uint16_t repeat) {")
  code64Bit.append("  enableIROut(38000);  // A guess. Most common frequency.")
  code64Bit.append("  for (uint16_t r = 0; r <= repeat; r++) {")

  for usecs in rawData:
    if (usecCompare(usecs, hdr_mark, margin) and
        isOdd(count) and not usecCompare(usecs, bit_mark, margin)):
      last = "HM"
      if len(binary_value) != 0:
        displayBinaryValue(binary_value)
        total_bits = total_bits + binary_value
        output.write(last)
      binary_value = addBit(binary_value, "reset")
      output.write("HDR_MARK+")
      code64Bit.append("    // Header")
      code64Bit.append("    mark(HDR_MARK);")
    elif (usecCompare(usecs, hdr_space, margin) and
          not usecCompare(usecs, one_space, margin)):
      if last != "HM":
        if len(binary_value) != 0:
          displayBinaryValue(binary_value)
          total_bits = total_bits + binary_value
          code64Bit.extend(addDataCode(binary_value))
        binary_value = addBit(binary_value, "reset")
        output.write("UNEXPECTED->")
      last = "HS"
      output.write("HDR_SPACE+")
      code64Bit.append("    space(HDR_SPACE);")
    elif (usecCompare(usecs, bit_mark, margin) and isOdd(count)):
      if last != "HS" and last != "BS":
        output.write("BIT_MARK(UNEXPECTED)")
      last = "BM"
    elif usecCompare(usecs, zero_space, margin):
      if last != "BM":
        output.write("ZERO_SPACE(UNEXPECTED)")
      last= "BS"
      binary_value = addBit(binary_value, 0)
    elif usecCompare(usecs, one_space, margin):
      if last != "BM":
        output.write("ONE_SPACE(UNEXPECTED)")
      last = "BS"
      binary_value = addBit(binary_value, 1)
    elif usecCompares(usecs, gap_candidates, margin):
      if last != "BM":
        output.write("UNEXPECTED->")
      last = "GS"
      output.write(" GAP(%d)" % usecs)
      displayBinaryValue(binary_value)
      code64Bit.extend(addDataCode(binary_value))
      code64Bit.append("    space(SPACE_GAP);")
      total_bits = total_bits + binary_value
      binary_value = addBit(binary_value, "reset")
    else:
      output.write("UNKNOWN(%d)" % usecs)
      last = "UNK"
    count = count + 1
  displayBinaryValue(binary_value)
  code64Bit.extend(addDataCode(binary_value))
  code64Bit.append("    space(100000);  // A 100% made up guess of the gap"
                   " between messages.")
  code64Bit.append("  }")
  code64Bit.append("}")

  total_bits = total_bits + binary_value
  output.write("Total Nr. of suspected bits: %d\n" % len(total_bits))
  codeDefines.append("#define XYZ_BITS %dU" % len(total_bits))
  if len(total_bits) > 64:
    codeDefines.append("#define XYZ_STATE_LENGTH %dU" % (len(total_bits) / 8))

  if gen_code:
    output.write("\nGenerating a VERY rough code outline:\n\n"
                 "// WARNING: This probably isn't directly usable."
                 " It's a guide only.\n")
    for line in codeDefines:
      output.write("%s\n" % line)

    if len(total_bits) > 64:  # Will it fit in a uint64_t?
      output.write("// DANGER: More than 64 bits detected. A uint64_t for "
                   "'data' won't work!\n")
    # Display the "normal" version's code incase there are some oddities in it.
    for line in code64Bit:
      output.write("%s\n" % line)

    if len(total_bits) > 64:  # Will it fit in a uint64_t?
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
                   "}\n" % ", 0x".join("%02X" % int(total_bits[i:i + 8], 2)
                       for i in range(0, len(total_bits), 8)))

def addDataCode(bin_str):
  code = []
  code.append("    // Data")
  code.append("    // e.g. data = 0x%X, nbits = %d" % (int(bin_str, 2),
                                                       len(bin_str)))
  code.append("    sendData(BIT_MARK, ONE_SPACE, BIT_MARK, ZERO_SPACE, data, "
              "nbits, true);")
  code.append("    // Footer")
  code.append("    mark(BIT_MARK);")
  return code

# Main program

class Args:
  pass

options = Args()
parser = argparse.ArgumentParser(
    description="Read an IRremoteESP8266 rawData declaration and tries to "
                "analyse it.",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-r", "--range", type=int,
    help="Max number of micro-seconds difference between values to consider it "
         "the same value.",
    dest="margin",
    default=200)
parser.add_argument("rawData",
    help="A rawData line from IRrecvDumpV2. e.g. 'uint16_t rawbuf[37] = {7930, "
         "3952, 494, 1482, 520, 1482, 494, 1508, 494, 520, 494, 1482, 494, 520,"
         " 494, 1482, 494, 1482, 494, 3978, 494, 520, 494, 520, 494, 520, 494, "
         "520, 520, 520, 494, 520, 494, 520, 494, 520, 494};'")
parser.add_argument("--code", action="store_true", default=False,
                    dest="gen_code",
                    help="Produce a C++ code outline to aid making an IRsend "
                         "function.")
parser.parse_args(namespace=options)

ParseAndReport(options.rawData, options.margin, options.gen_code)
