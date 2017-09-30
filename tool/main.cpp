#include "IRsend.h"
#include "IRsend_test.h"
#include <iostream>
using namespace std;

int main (void) {

    IRsendTest irsend(4);
    IRrecv irrecv(4);
    irsend.begin();
    irsend.reset();

    uint16_t gc_test[71] = {38000, 1, 1, 342, 172, 21, 22, 21, 21, 21, 65, 21, 21,
                          21, 22, 21, 22, 21, 21, 21, 22, 21, 65, 21, 65, 21,
                          22, 21, 65, 21, 65, 21, 65, 21, 65, 21, 65, 21, 65,
                          21, 22, 21, 22, 21, 21, 21, 22, 21, 22, 21, 65, 21,
                          22, 21, 21, 21, 65, 21, 65, 21, 65, 21, 64, 22, 65,
                          21, 22, 21, 65, 21, 1519};
    irsend.sendGC(gc_test, 71);
    irsend.makeDecodeResult();

    irrecv.decodeNEC(&irsend.capture);

    cout << "Code type " << irsend.capture.decode_type << endl;
    cout << "Code bits " << irsend.capture.bits << endl;
    cout << "Code value " << std::hex << irsend.capture.value << endl;
    cout << "Code address " << irsend.capture.address << endl;
    cout << "Code command " << irsend.capture.command << endl;

    return 0;
}
