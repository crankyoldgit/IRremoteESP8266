#!/usr/bin/python3
"""Automaticly get data from ESP8266's Raw data output and calculating average time for zero, one and space"""
#
# Copyright 2024 Andrey Kravchenko (StellaLupus)
import serial
import serial.tools
import serial.tools.list_ports


def get_port():
    """Allow user to select com port to connect device"""
    ports = sorted(serial.tools.list_ports.comports())
    print("Available ports to listing:")
    for id, port_info in enumerate(ports):
        print("{}. - {}: {} [{}]".format(id, port_info.device,
                                         port_info.description,
                                         port_info.hwid))
    print("Select port: ", end="")
    select_id = int(input())
    if select_id < len(ports) and select_id >= 0:
        return ports[select_id]
    else:
        print("Unrecognized port number")
        return get_port()


ZERO_T = 756
ONE_T = 2149
SPACE_T = 752
PRECISION = 0.25


def get_bit_from_interv(value: int):
    """Returning zero or one from value or 2 if not recognized"""
    if value > ZERO_T - ZERO_T * PRECISION and value < ZERO_T + ZERO_T * PRECISION:
        return 0
    elif value > ONE_T - ONE_T * PRECISION and value < ONE_T + ONE_T * PRECISION:
        return 1
    else:
        return 2


def bit_list_to_int(bit_list):
    """Convert list of bits to int"""
    out = 0
    for bit in bit_list:
        out = (out << 1) | bit
    return out


def main():
    """Main method"""
    port = get_port()
    print("Selected port:" + port.device)

    ser = serial.Serial(
        port=port.device,
        baudrate=115200,
    )

    ZERO_SUM = 0
    ZERO_COUNT = 0
    ONE_SUM = 0
    ONE_CNT = 0
    SPACE_SUM = 0
    SPACE_CNT = 0

    while True:
        try:
            data_str = ser.readline().decode()
        except:
            continue
        if "uint16_t rawData" in data_str:
            data_str_array = str(data_str[data_str.index("{") +
                                          1:data_str.index("}")]).split(",")
            data = [int(i.strip()) for i in data_str_array]
            data = data[2:]
            clear_data = [i for idi, i in enumerate(data) if idi % 2 == 1]
            bit_data = [get_bit_from_interv(i) for i in data]
            clear_bit_data = [get_bit_from_interv(i) for i in clear_data]

            for idd, d in enumerate(data):
                if idd % 2 == 0:
                    SPACE_SUM += d
                    SPACE_CNT += 1
                else:
                    if get_bit_from_interv(d) == 0:
                        ZERO_SUM += d
                        ZERO_COUNT += 1
                    elif get_bit_from_interv(d) == 1:
                        ONE_SUM += d
                        ONE_CNT += 1

            print("Bit data = " + "".join([str(i) for i in bit_data]))
            print("Clear bit data = " +
                  "".join([str(i) for i in clear_bit_data]))


if __name__ == "__main__":
    main()
