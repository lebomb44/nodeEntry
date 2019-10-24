#include <CnC.h>

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
  uint64_t uid_ = 0;
  if(nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength) {
    for(uint8_t i=0; i<7; i++) { uid_ << 8; uid_ |= nfcUID[i]; }
  }
  cnc_print_cmdGet_u64(nfcTargetIDName, 0x00FFFFFFFFFFFFFF & uid_);
}
void nfcKey_cmdGet(int arg_cnt, char **args) {
  uint64_t key_ = 0;
  for(uint8_t i=0; i<6; i++) { key_ << 8; key_ |= nfcKey[i]; }
  cnc_print_cmdGet_u64(nfcKeyName, 0x0000FFFFFFFFFFFF & key_);
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
  cnc_cmdGet_Add(nfcReadBlockName, nfcReadBlock);
  cnc_cmdGet_Add(nfcReadTargetName, nfcReadTarget);
  
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
