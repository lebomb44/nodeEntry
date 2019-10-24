#include <CnC.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"

const char nodeName[] PROGMEM = "entry";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "val";
const char cmdGetName[] PROGMEM = "get";
const char cmdSetName[] PROGMEM = "set";

const char pingName[] PROGMEM = "ping";
const char nfcFirmwareVersionName[] PROGMEM = "nfcFirmwareVersion";
const char nfcTargetIDName[] PROGMEM = "nfcTargetID";
const char nfcKeyName[] PROGMEM = "nfcKey";
const char nfcAuthenticateBlockName[] PROGMEM = "nfcAuthenticateBlock";
const char nfcReadBlockName[] PROGMEM = "nfcReadBlock";
const char nfcReadTargetName[] PROGMEM = "nfcReadTarget";

uint32_t previousTime_1s = 0;
uint32_t previousTime_10s = 0;
uint32_t currentTime = 0;

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);

uint8_t nfcUID[7] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t nfcUIDLength = 0;  
uint8_t nfcKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, currentTime); }
void nfcFirmwareVersion_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(nfcFirmwareVersionName, nfc.getFirmwareVersion()); }
void nfcTargetID_cmdGet(int arg_cnt, char **args) {
  for(uint8_t i=0; i<7; i++) { nfcUID[i] = 0; }
  nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength);
  cnc_print_cmdGet_tbd(nfcTargetIDName);
  for(uint8_t i=0; i<7; i++) { Serial.print(nfcUID[i], HEX); }
  Serial.println(); Serial.flush();
}
void nfcKey_cmdGet(int arg_cnt, char **args) {
  cnc_print_cmdGet_tbd(nfcKeyName);
  for(uint8_t i=0; i<6; i++) { Serial.print(nfcKey[i], HEX); }
  Serial.println(); Serial.flush();
}
void nfcKey_cmdSet(int arg_cnt, char **args) {
  char strkey_[3] = {0,0,0};
  if(4 == arg_cnt) {
    for(uint8_t i=0; i<6; i++) {
      strkey_[0] = args[3][2*i];
      strkey_[1] = args[3][(2*i)+1];
      strkey_[2] = 0;
      nfcKey[i] = strtoul(strkey_, NULL, 16));
    }
  }
}
     
void setup() {
  Serial.begin(115200);
  cncInit(nodeName);
  cnc_hkName_set(hkName);
  cnc_cmdGetName_set(cmdGetName);
  cnc_cmdSetName_set(cmdSetName);
  cnc_sepName_set(sepName);
  cnc_cmdGet_Add(pingName, ping_cmdGet);
  cnc_cmdGet_Add(nfcFirmwareVersionName, nfcFirmwareVersion_cmdGet);
  cnc_cmdGet_Add(nfcTargetIDName, nfcTargetID_cmdGet);
  cnc_cmdGet_Add(nfcKeyName, nfcKey_cmdGet);
  cnc_cmdSet_Add(nfcKeyName, nfcKey_cmdSet);
  //cnc_cmdGet_Add(nfcReadBlockName, nfcReadBlock);
  //cnc_cmdGet_Add(nfcReadTargetName, nfcReadTarget);
  
  previousTime_1s = millis();
  previousTime_10s = previousTime_1s;
  
  nfc.begin();
  nfc.SAMConfig();
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
