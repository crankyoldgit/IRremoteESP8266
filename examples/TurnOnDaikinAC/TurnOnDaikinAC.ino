
#include <ESPIRDaikinESP.h>

IRDaikinESP dakinir(D1);

void setup(){
  dakinir.begin();
  Serial.begin(115200);
}


void loop(){
  Serial.println("Sending...");
  dakinir.setFan(1);
  dakinir.setMode(DAIKIN_COOL);
  dakinir.setTemp(25);
  dakinir.send();
  delay(5000);
}

