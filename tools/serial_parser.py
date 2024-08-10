import enum
import serial
import serial.tools
import serial.tools.list_ports

def substring_after(str: str, searchStr: str):
   return str[str.index(searchStr) + len(searchStr):]

def getPort():
    ports = sorted(serial.tools.list_ports.comports())
    print("Available ports to listing:")
    for id, portInfo in enumerate(ports):
       print("{}. - {}: {} [{}]".format(id, portInfo.device, portInfo.description, portInfo.hwid))
    print("Select port: ", end="")
    selectId = int(input())
    if selectId < len(ports) and selectId >= 0:
       return ports[selectId]
    else:
       print("Unrecognized port number")
       return getPort()
    
zero = 756
one = 2149
space = 752
interv = 0.2

def getBitFromInterv(value: int):
   if(value > zero - zero * interv and value < zero + zero * interv):
      return 0
   elif(value > one - one * interv and value < one + one * interv):
      return 1
   else:
      return 2

def bitListToInt(bitlist):
    out = 0
    for bit in bitlist:
        out = (out << 1) | bit
    return out


def main():
    port = getPort()
    print("Selected port:" + port.device)

    ser = serial.Serial(
       port=port.device,
       baudrate=115200,
    )

    zSum = 0
    zCnt = 0
    oSum = 0
    oCnt = 0
    sSum = 0
    sCnt = 0

    while True:
       try:
         dataStr = ser.readline().decode()
       except:
          continue
       #print(dataStr)
       if("uint16_t rawData" in dataStr):
        dataStrArray = str(dataStr[dataStr.index('{') + 1:dataStr.index('}')]).split(",")
        data = [int(i.strip()) for i in dataStrArray]
        data = data[2:]
        clearData = [i for idi, i in enumerate(data) if idi % 2 == 1]
        bitData = [getBitFromInterv(i) for i in data]
        clearBitData = [getBitFromInterv(i) for i in clearData]



        for idd, d in enumerate(data):
            if(idd % 2 == 0):
               sSum += d
               sCnt += 1
            else:
                if(getBitFromInterv(d) == 0):
                    zSum += d
                    zCnt += 1
                elif(getBitFromInterv(d) == 1):
                    oSum += d
                    oCnt += 1

        # print("zero = " + str(zSum/zCnt) + " one = " + str(oSum/oCnt) + " space = " + str(sSum/sCnt))\
        
        

        # print("Data = ", end="")
        # print(data)
        # print("0b"+ "".join([str(i) for i in bitData]))
        # print("ClearData = ", end="")
        #print(clearData)
        print("".join([str(i) for i in clearBitData]))
        

if __name__ == '__main__':
  main()