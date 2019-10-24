#include <CnC.h>

const char nodeName[] PROGMEM = "entry";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "val";
const char cmdGetName[] PROGMEM = "get";
const char cmdSetName[] PROGMEM = "set";

const char pingName[] PROGMEM = "ping";
const char nfcInitName[] PROGMEM = "nfcInit";
const char nfcReadBlockName[] PROGMEM = "nfcReadBlock";
const char nfcReadTargetName[] PROGMEM = "nfcReadTarget";

uint32_t previousTime_1s = 0;
uint32_t previousTime_10s = 0;
uint32_t currentTime = 0;

void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, currentTime); }
void windowWindowContact_cmdGet(int arg_cnt, char **args) { windowWindowContact.cmdGet(arg_cnt, args); }

void setup() {
  Serial.begin(115200);
  cncInit(nodeName);
  cnc_hkName_set(hkName);
  cnc_cmdGetName_set(cmdGetName);
  cnc_cmdSetName_set(cmdSetName);
  cnc_sepName_set(sepName);
  cnc_cmdGet_Add(pingName, ping_cmdGet);
  cnc_cmdSet_Add(nfcInitName, nfcInit);
  cnc_cmdGet_Add(nfcReadBlockName, nfcReadBlock);
  cnc_cmdGet_Add(nfcReadTargetName, nfcReadTarget);
  
  previousTime_1s = millis();
  previousTime_10s = previousTime_1s;
}

void loop() {
  currentTime = millis(); cncPoll();
  /* HK @ 1.0Hz */
  if((uint32_t)(currentTime - previousTime_1s) >= 1000) {
    windowWindowContact.run(true); cncPoll();
    previousTime_1s = currentTime;
  }
  /* HK @ 0.1Hz */
  if((uint32_t)(currentTime - previousTime_10s) >= 10000) {
    tempSensors.begin(); cncPoll();
    previousTime_10s = currentTime;
  }
}
