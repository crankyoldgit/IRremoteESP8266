#include "IRsend.h"
#include "IRsend_test.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h> /* strtoumax */
#include <stdbool.h>


using namespace std;


void str_to_uint16(char *str, uint16_t *res)
{
  char *end;
  errno = 0;
  intmax_t val = strtoimax(str, &end, 10);
  if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
    return;
  *res = (uint16_t) val;
}


int main (int argc, char * argv[]) {


    if (argc != 2) { 
        cout << "Use main [global_code]" << endl;
	return 0;
    }

    uint16_t gc_test[150];
    int index = 0;

    char * pch;
    pch = strtok (argv[1],",");
    while (pch != NULL)
    {
        str_to_uint16(pch, &gc_test[index]);
        pch = strtok (NULL, " ,");
	index++;
    }

    IRsendTest irsend(4);
    IRrecv irrecv(4);
    irsend.begin();
    irsend.reset();


    irsend.sendGC(gc_test, index);
    irsend.makeDecodeResult();
    irrecv.decode(&irsend.capture);

    cout << "length " << index << endl;
    cout << "Code type " << irsend.capture.decode_type << endl;
    cout << "Code bits " << irsend.capture.bits << endl;
    cout << "Code value " << std::hex << irsend.capture.value << endl;
    cout << "Code address " << irsend.capture.address << endl;
    cout << "Code command " << irsend.capture.command << endl;

    return 0;
}
