// Copyright 2022 Mateusz Bronk

#ifndef TEST_UT_UTILS_H_
#define TEST_UT_UTILS_H_

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>


std::string bytesToHexString(const std::vector<uint8_t>& value) {
    std::ostringstream os;
    os << std::hex << std::setfill('0');  // set the stream to hex with 0 fill

    std::for_each(std::begin(value), std::end(value), [&os] (int i) {
        os << std::setw(2) << std::uppercase << static_cast<int>(i);
    });
    return os.str();
}

std::vector<uint8_t> hexStringToBytes(const std::string& hex) {
  std::vector<uint8_t> bytes;
  bytes.reserve(hex.length() / 2);

  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string nextByte = hex.substr(i, 2);
    uint8_t byte = static_cast<uint8_t>(strtol(nextByte.c_str(), nullptr, 16));
    bytes.emplace_back(byte);
  }
  return bytes;
}

#endif  // TEST_UT_UTILS_H_
