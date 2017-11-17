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

function cullListHigh()
{
  high_mark=$1
  shift
  for i in $*; do
    if [[ $i -lt $high_mark ]]; then
      echo $i
    fi
  done
}

function cullListLow()
{
  low_mark=$1
  shift
  for i in $*; do
    if [[ $i -ge $low_mark ]]; then
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
    list=$(cullListHigh $((max - RANGE)) $list)
    max=$(maxFromList $list)
  done
}

function sortReverse()
{
  for i in $*; do
    echo $i
  done | sort -nr
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
  echo -n $1
  if [[ $bits -eq 0 ]]; then
    hex_value=""
    value=0
  fi
  bits=$((bits + 1))
  if [[ $((bits % 8)) -eq 1 && $bits -gt 1 ]]; then
    hex_value=$(getValue)
    value=0
  fi
  value=$(((value * 2) + $1))
}

function getValue()
{
  echo "${hex_value}$(printf %02X $value)"
}

function isOdd()
{
  [[ $(($1 % 2)) -eq 1 ]]
}

function usage()
{
  cat << EOF
Usage: $0 [grouping_range]
  Reads an IRremoteESP8266 rawData declaration from STDIN and tries to
  analyse it.

  Args:
    grouping_range: Max number of milli-seconds difference between values
                    to consider it the same value. (Default: ${DEFAULT_RANGE})

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


# Main program

DEFAULT_RANGE=200

# Check the calling arguments.
if [[ $# -gt 1 ]]; then
  usage
elif [[ $# -eq 1 ]]; then
  if isDigits $1; then
    RANGE=$1
  else
    usage
  fi
else
  RANGE=${DEFAULT_RANGE}
fi

# Parse the input.
count=1
while read line; do
  # Quick and Dirty Removal of any array declaration syntax, and any commas.
  line="$(echo ${line} | sed 's/uint.*{//i' | sed 's/,//g' | sed 's/};.*//g')"
  for msecs in ${line}; do
    if isDigits "${msecs}"; then
      orig="$orig $msecs"
      if isOdd $count; then
        marks="$marks $msecs"
      else
        spaces="$spaces $msecs"
      fi
      count=$(($count + 1))
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
  # The mark bit is likely to be the smallest.
  BIT_MARK=$(reduceList $marks | tail -1)
  echo BIT_MARK = $BIT_MARK

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
  done
  # We should have 3 space candidates left.
  # They should be ZERO_SPACE (smallest), ONE_SPACE, & HDR_SPACE (largest)
  ZERO_SPACE=$(reduceList $spaces | tail -1)
  ONE_SPACE=$(reduceList $spaces | tail -2 | head -1)
  HDR_SPACE=$(reduceList $spaces | tail -3 | head -1)
  echo HDR_SPACE = $HDR_SPACE
  echo ONE_SPACE = $ONE_SPACE
  echo ZERO_SPACE = $ZERO_SPACE
else
  echo "Sorry, it looks like it is Mark encoded. I can't do that yet. Exiting."
  exit 1
fi

# Now we have likely candidates for the key values, go through the original
# sequence and break it up and indicate accordingly.

echo
echo "Decoding protocol based on analysis so far:"
echo "[NOTE: Data bits are assumed to be in Most Significant Bit First order.]"
echo
last=""
value=0
bits=0
count=1
for msecs in $orig; do
  if isHdrMark $msecs && isOdd $count; then
    last="HM"
    if [[ $bits -ne 0 ]]; then
      echo
      echo "Value = $(getValue) ($bits bits)"
      echo -n $last
    fi
    bits=0
    echo -n "HDR_MARK+"
  elif isHdrSpace $msecs; then
    if [[ $last != "HM" ]]; then
      if [[ $bits -ne 0 ]]; then
        echo
        echo "Value = $(getValue) ($bits bits)"
      fi
      bits=0
      echo -n "UNEXPECTED->"
    fi
    last="HS"
    echo -n "HDR_SPACE+"
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
    echo "Value = $(getValue) ($bits bits)"
    bits=0
  else
    echo -n "UNKNOWN($msecs)"
    last="UNK"
  fi
  count=$((count + 1))
done
echo
echo "Value = $(getValue) ($bits bits)"
