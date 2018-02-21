#!/bin/bash
# Attempt an automatic analysis of IRremoteESP8266's Raw data output.
# Makes suggestions on key values and tried to break down the message
# into likely chuncks.
#
# Copyright 2017 David Conran

function isDigits()
{
  [[ "$1" =~ ^[0-9]+$ ]]
}

function maxFromList()
{
  max=-1
  for i in $*; do
    if [[ $max -lt $i ]]; then
      max=$i
    fi
  done
  echo $max
}

function cullList()
{
  high_mark=$1
  shift
  for i in $*; do
    if [[ $i -lt $high_mark ]]; then
      echo $i
    fi
  done
}

function reduceList()
{
  list=$*
  max=$(maxFromList $*)
  while [[ $max -gt 0 ]]; do
    echo "$max"
    list=$(cullList $((max - RANGE)) $list)
    max=$(maxFromList $list)
  done
}

function listLength()
{
  echo $#
}

function isHdrMark()
{
  [[ $1 -le $HDR_MARK && $1 -gt $((HDR_MARK - RANGE)) ]]
}

function isBitMark()
{
  [[ $1 -le $BIT_MARK && $1 -gt $((BIT_MARK - RANGE)) ]]
}

function isHdrSpace()
{
  [[ $1 -le $HDR_SPACE && $1 -gt $((HDR_SPACE - RANGE)) ]]
}

function isZeroSpace()
{
  [[ $1 -le $ZERO_SPACE && $1 -gt $((ZERO_SPACE - RANGE)) ]]
}

function isOneSpace()
{
  [[ $1 -le $ONE_SPACE && $1 -gt $((ONE_SPACE - RANGE)) ]]
}

function isGap()
{
  for i in $GAP_LIST; do
    if [[ $1 -le $i && $1 -gt $((i - RANGE)) ]]; then
      return 0
    fi
  done
  return 1
}

function addBit()
{
  if [[ ${1} == "reset" ]]; then
    binary_value=""
    bits=0
    return
  fi
  echo -n "${1}"  # This effectively displays in LSB first order.
  bits=$((bits + 1))
  total_bits=$((total_bits + 1))
  binary_value="${binary_value}${1}"  # Storing it in MSB first order.
}

function isOdd()
{
  [[ $(($1 % 2)) -eq 1 ]]
}

function usage()
{
  cat >&2 << EOF
Usage: $0 [-r grouping_range] [-g]
  Reads an IRremoteESP8266 rawData declaration from STDIN and tries to
  analyse it.

  Args:
    -r grouping_range
        Max number of milli-seconds difference between values
        to consider it the same value. (Default: ${RANGE})
    -g
        Produce a C++ code outline to aid making an IRsend function.

  Example input:
    uint16_t rawbuf[37] = {
        7930, 3952, 494, 1482, 520, 1482, 494, 1508,
        494, 520, 494, 1482, 494, 520, 494, 1482,
        494, 1482, 494, 3978, 494, 520, 494, 520,
        494, 520, 494, 520, 520, 520, 494, 520,
        494, 520, 494, 520, 494};
EOF
  exit 1
}

function binToBase()
{
  bc -q << EOF
obase=${2}
ibase=2
${1}
EOF
}

function displayBinaryValue()
{
  [[ -z ${1} ]] && return  # Nothing to display
  reversed=$(echo ${1} | rev)  # Convert the binary value to LSB first
  echo "  Bits: ${bits}"
  echo "  Hex:  0x$(binToBase ${1} 16) (MSB first)"
  echo "        0x$(binToBase ${reversed} 16) (LSB first)"
  echo "  Dec:  $(binToBase ${1} 10) (MSB first)"
  echo "        $(binToBase ${reversed} 10) (LSB first)"
  echo "  Bin:  ${1} (MSB first)"
  echo "        ${reversed} (LSB first)"
  if [[ "${1}" == "${last_binary_value}" ]]; then
    echo "  Note: Value is the same as the last one. Could be a repeated message."
  fi
}

function addCode() {
  CODE=$(echo "${CODE}"; echo "${*}")
}

function addDataCode() {
  addCode "    // Data #${data_count}"
  if [[ "${binary_value}" == "${last_binary_value}" ]]; then
    addCode "    // CAUTION: data value appears to be a duplicate."
    addCode "    //          This could be a repeated message."
  fi
  addCode "    // e.g. data = 0x$(binToBase ${binary_value} 16), nbits = ${bits}"
  addCode "$(bitSizeWarning ${bits} '    ')"
  addCode "    sendData(BIT_MARK, ONE_SPACE, BIT_MARK, ZERO_SPACE, data, nbits, true);"
  addCode "    // Footer #${data_count}"
  addCode "    mark(BIT_MARK);"
  data_count=$((data_count + 1))
  last_binary_value=$binary_value
}

function bitSizeWarning() {
  # $1 is the nr of bits. $2 is what to indent with.
  if [[ ${1} -gt 64 ]]; then
    echo "${2}// DANGER: More than 64 bits detected. A uint64_t for data won't work!"
    echo "${2}// DANGER: Try using alternative AirCon version below!"
  fi
}

# Main program

RANGE=200
OUTPUT_CODE=""

while getopts "r:g" opt; do
  case $opt in
    r)
      if isDigits $OPTARG; then
        RANGE=$OPTARG
      else
        echo "Error: grouping_range is not a positive integer." >&2
        usage
      fi
      ;;
    g)
      DISPLAY_CODE="yes"
      ;;
    *)
      usage
      ;;
  esac
done
shift $((OPTIND-1))

if [[ $# -ne 0 ]]; then
  usage
fi

if ! which bc &> /dev/null ; then
  cat << EOF
'bc' program not found. Exiting.
Suggestion: sudo apt-get install bc
EOF
  exit 2
fi

# Parse the input.
count=1
while read line; do
  # Quick and Dirty Removal of any array declaration syntax, and any commas.
  line="$(echo ${line} | sed 's/^.*uint.*{//i' | sed 's/,/ /g' | sed 's/};.*//g')"
  for msecs in ${line}; do
    if isDigits "${msecs}"; then
      orig="${orig} ${msecs}"
      if isOdd $count; then
        marks="${marks} ${msecs}"
      else
        spaces="${spaces} ${msecs}"
      fi
      count=$((count + 1))
    fi
  done
done

echo "Potential Mark Candidates (using a range of $RANGE):"
reduceList $marks
nr_mark_candidates=$(listLength $(reduceList $marks))
echo
echo "Potential Space Candidates (using a range of $RANGE):"
reduceList $spaces
nr_space_candidates=$(listLength $(reduceList $spaces))
echo
echo "Guessing encoding type:"
if [[ $nr_space_candidates -ge $nr_mark_candidates ]]; then
  echo "Looks like it uses space encoding. Yay!"
  echo
  echo "Guessing key value:"

  # Largest mark is likely the HDR_MARK
  HDR_MARK=$(reduceList $marks | head -1)
  echo HDR_MARK = $HDR_MARK
  addCode "#define HDR_MARK ${HDR_MARK}U"
  # The mark bit is likely to be the smallest.
  BIT_MARK=$(reduceList $marks | tail -1)
  echo BIT_MARK = $BIT_MARK
  addCode "#define BIT_MARK ${BIT_MARK}U"


  left=$nr_space_candidates
  gap_num=0
  GAP_LIST=""
  while [[ $left -gt 3 ]]; do
    # We probably (still) have a gap in the protocol.
    gap=$((gap + 1))
    SPACE_GAP=$(reduceList $spaces | head -$gap | tail -1)
    GAP_LIST="$GAP_LIST $SPACE_GAP"
    left=$((left - 1))
    echo SPACE_GAP${gap} = $SPACE_GAP
    addCode "#define SPACE_GAP${gap} ${SPACE_GAP}U"
  done
  # We should have 3 space candidates left.
  # They should be ZERO_SPACE (smallest), ONE_SPACE, & HDR_SPACE (largest)
  ZERO_SPACE=$(reduceList $spaces | tail -1)
  ONE_SPACE=$(reduceList $spaces | tail -2 | head -1)
  HDR_SPACE=$(reduceList $spaces | tail -3 | head -1)
  echo HDR_SPACE = $HDR_SPACE
  addCode "#define HDR_SPACE ${HDR_SPACE}U"
  echo ONE_SPACE = $ONE_SPACE
  addCode "#define ONE_SPACE ${ONE_SPACE}U"
  echo ZERO_SPACE = $ZERO_SPACE
  addCode "#define ZERO_SPACE ${ZERO_SPACE}U"
else
  echo "Sorry, it looks like it is Mark encoded. I can't do that yet. Exiting."
  exit 1
fi

# Now we have likely candidates for the key values, go through the original
# sequence and break it up and indicate accordingly.

echo
echo "Decoding protocol based on analysis so far:"
echo
last=""
count=1
data_count=1
last_binary_value=""
total_bits=0
addBit reset

addCode "// Function"
addCode "void IRsend::sendXYZ(const uint64_t data, const uint16_t nbits, const uint16_t repeat) {"
addCode "  for (uint16_t r = 0; r <= repeat; r++) {"

for msecs in $orig; do
  if isHdrMark $msecs && isOdd $count && ! isBitMark $msecs; then
    last="HM"
    if [[ $bits -ne 0 ]]; then
      echo
      displayBinaryValue ${binary_value}
      echo -n $last
    fi
    addBit reset
    echo -n "HDR_MARK+"
    addCode "    // Header #${data_count}"
    addCode "    mark(HDR_MARK);"
  elif isHdrSpace $msecs && ! isOneSpace $msecs; then
    if [[ $last != "HM" ]]; then
      if [[ $bits -ne 0 ]]; then
        echo
        displayBinaryValue ${binary_value}
      fi
      addBit reset
      echo -n "UNEXPECTED->"
    fi
    last="HS"
    echo -n "HDR_SPACE+"
    addCode "    space(HDR_SPACE);"
  elif isBitMark $msecs && isOdd $count; then
    if [[ $last != "HS" && $last != "BS" ]]; then
      echo -n "BIT_MARK(UNEXPECTED)"
    fi
    last="BM"
  elif isZeroSpace $msecs; then
    if [[ $last != "BM" ]] ; then
      echo -n "ZERO_SPACE(UNEXPECTED)"
    fi
    last="BS"
    addBit 0
  elif isOneSpace $msecs; then
    if [[ $last != "BM" ]] ; then
      echo -n "ONE_SPACE(UNEXPECTED)"
    fi
    last="BS"
    addBit 1
  elif isGap $msecs; then
    if [[ $last != "BM" ]] ; then
      echo -n "UNEXPECTED->"
    fi
    last="GS"
    echo " GAP($msecs)"
    displayBinaryValue ${binary_value}
    addDataCode
    addCode "    space($msecs);"
    addBit reset
  else
    echo -n "UNKNOWN($msecs)"
    last="UNK"
  fi
  count=$((count + 1))
done
echo
displayBinaryValue ${binary_value}
if [[ "$DISPLAY_CODE" == "yes" ]]; then
  echo
  echo "Generating a VERY rough code outline:"
  echo
  echo "// WARNING: This probably isn't directly usable. It's a guide only."
  bitSizeWarning ${total_bits}
  addDataCode
  addCode "    delay(100);  // A 100% made up guess of the gap between messages."
  addCode "  }"
  addCode "}"
  if [[ ${total_bits} -gt 64 ]]; then
    addCode "Alternative (aircon code):"
    addCode "// Alternative Function AirCon mode"
    addCode "void IRsend::sendXYZ(uint8_t data[], uint16_t nbytes, uint16_t repeat) {"
    addCode "  // nbytes should typically be $(($total_bits / 8))"
    addCode "  // data should typically be of a type: uint8_t data[$(($total_bits / 8))] = {};"
    addCode "  // data[] will is assumed to be in MSB order."
    addCode "  for (uint16_t r = 0; r <= repeat; r++) {"
    addCode "    sendGeneric(HDR_MARK, HDR_SPACE, BIT_MARK, ONE_SPACE, BIT_MARK, ZERO_SPACE, BIT_MARK"
    addCode "                100, data, nbytes, 38, true, 0, 50);"
    addCode "}"
  fi

  echo "$CODE"
fi
