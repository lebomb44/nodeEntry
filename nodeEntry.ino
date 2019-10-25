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
const char nfcWriteBlockName[] PROGMEM = "nfcWriteBlock";
const char nfcFormatName[] PROGMEM = "nfcFormat";
const char nfcModeName[] PROGMEM = "nfcMode";
const char nfcTagName[] PROGMEM = "nfcTag";

uint32_t previousTime_1s = 0;
uint32_t previousTime_10s = 0;
uint32_t currentTime = 0;

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);

uint8_t nfcUID[7] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t nfcUIDLength = 0;
uint8_t nfcKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t nfcMode = 1;

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
      nfcKey[i] = strtoul(strkey_, NULL, 16);
    }
  }
}
void nfcAuthenticateBlock_cmdGet(int arg_cnt, char **args) {
  if(5 == arg_cnt) {
    uint32_t blockNumber_ = strtoul(args[3], NULL, 10);
    uint8_t keyNumber_ = strtoul(args[4], NULL, 10);
    cnc_print_cmdGet_u32(nfcAuthenticateBlockName, nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, blockNumber_, keyNumber_, nfcKey));
  }
  else { cnc_print_cmdGet_tbd(nfcAuthenticateBlockName); Serial.println("ARG_ERROR"); Serial.flush(); }
}
void nfcReadBlock_cmdGet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    uint32_t blockNumber_ = strtoul(args[3], NULL, 10);
    uint8_t data_[16] = {0};
    if(nfc.mifareclassic_ReadDataBlock(blockNumber_, data_)) {
      cnc_print_cmdGet_tbd(nfcReadBlockName);
      for(uint8_t i=0; i<16; i++) { Serial.print(data_[i], HEX); }
      Serial.println(); Serial.flush();
    }
    else { cnc_print_cmdGet_tbd(nfcReadBlockName); Serial.println("READ_ERROR"); Serial.flush(); }
  }
  else { cnc_print_cmdGet_tbd(nfcReadBlockName); Serial.println("ARG_ERROR"); Serial.flush(); }
}
void nfcWriteBlock_cmdGet(int arg_cnt, char **args) {
  if(5 == arg_cnt) {
    uint32_t blockNumber_ = strtoul(args[3], NULL, 10);
    uint8_t data_[16] = {0};
    char strdata_[3] = {0,0,0};
    for(uint8_t i=0; i<16; i++) {
      strdata_[0] = args[4][2*i];
      strdata_[1] = args[4][(2*i)+1];
      strdata_[2] = 0;
      data_[i] = strtoul(strdata_, NULL, 16);
    }
    if(nfc.mifareclassic_WriteDataBlock(blockNumber_, data_)) {
      cnc_print_cmdGet_tbd(nfcWriteBlockName);
      Serial.println("OK"); Serial.flush();
    }
    else { cnc_print_cmdGet_tbd(nfcReadBlockName); Serial.println("WRITE_ERROR"); Serial.flush(); }
  }
  else { cnc_print_cmdGet_tbd(nfcReadBlockName); Serial.println("ARG_ERROR"); Serial.flush(); }
}
void nfcFormat_cmdGet(int arg_cnt, char **args) {
  if(5 == arg_cnt) {
    uint8_t surname_[16] = {0};
    uint8_t name_[16] = {0};
    uint8_t data_[16] = {0};
    char strdata_[3] = {0,0,0};

    /* Surname */
    for(uint8_t i=0; i<16; i++) { surname_[i] = 0 }
    for(uint8_t i=0; i<16; i++) {
      if(0 == args[3][2*i]) { break; }
      strdata_[0] = args[3][2*i];
      strdata_[1] = args[3][(2*i)+1];
      strdata_[2] = 0;
      surname_[i] = strtoul(strdata_, NULL, 16);
    }
    /* Name */
    for(uint8_t i=0; i<16; i++) { name_[i] = 0 }
    for(uint8_t i=0; i<16; i++) {
      if(0 == args[4][2*i]) { break; }
      strdata_[0] = args[4][2*i];
      strdata_[1] = args[4][(2*i)+1];
      strdata_[2] = 0;
      name_[i] = strtoul(strdata_, NULL, 16);
    }
    /* Data */
    for(uint8_t i=0; i<16; i++) { data_[i] = 0 }
    /* Keys */
    for(uint8_t i=0; i<6; i++) { keys_[i] = nfcKey[i]; keys_[i+10] = nfcKey[i]+1; }
    key[6] = 0xFF; key[7] = 0x07; key[8] = 0x80; key[9] = 0x69;
    /* Write all blocks to tag */
    for(uint8_t i=0; i<16; i++) {
      if(0 == nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, i, 0, nfcKey)) {
        cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("AUT_ERROR "); Serial.println(i/4); Serial.flush(); return;
      }
      if(0 != i) {
        if(0 == nfc.mifareclassic_WriteDataBlock(i, surname_)) {
          cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("SURNAME_ERROR "); Serial.println(i); Serial.flush(); return;
        }
      }
      if(0 == nfc.mifareclassic_WriteDataBlock(i+1, name_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("NAME_ERROR "); Serial.println(i); Serial.flush(); return;
      }
      if(0 == nfc.mifareclassic_WriteDataBlock(i+2, data_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("DATA_ERROR "); Serial.println(i); Serial.flush(); return;
      }
      if(0 == nfc.mifareclassic_WriteDataBlock(i+3, keys_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("KEYS_ERROR "); Serial.println(i); Serial.flush(); return;
      }
      cnc_print_cmdGet_tbd(nfcFormatName); Serial.print("OK "); Serial.println(i); Serial.flush(); return;
    }
    cnc_print_cmdGet_tbd(nfcFormatName); Serial.println("OK"); Serial.flush(); return;
  }
  else { cnc_print_cmdGet_tbd(nfcFormatName); Serial.println("ARG_ERROR"); Serial.flush(); }
}
void nfcMode_cmdGet(int arg_cnt, char **args) {
  cnc_print_cmdGet_tbd(nfcModeName, nfcMode);
}
void nfcMode_cmdSet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    nfcMode = strtoul(args[3], NULL, 16);
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
  cnc_cmdGet_Add(nfcAuthenticateBlockName, nfcAuthenticateBlock_cmdGet)
  cnc_cmdGet_Add(nfcReadBlockName, nfcReadBlock_cmdGet);
  cnc_cmdGet_Add(nfcWriteBlockName, nfcWriteBlock_cmdGet);
  cnc_cmdGet_Add(nfcFormatName, nfcFormat_cmdGet);
  cnc_cmdGet_Add(nfcModeName, nfcMode_cmdGet);
  cnc_cmdSet_Add(nfcModeName, nfcMode_cmdSet);

  previousTime_1s = millis();
  previousTime_10s = previousTime_1s;
  
  nfc.begin();
  nfc.SAMConfig();
}

void loop() {
  currentTime = millis(); cncPoll();
  /* HK @ 1.0Hz */
  if((uint32_t)(currentTime - previousTime_1s) >= 1000) {
    if(1 == nfcMode) {
      uint8_t uid_[7] = {0};
      for(uint8_t i=0; i<7; i++) { uid_[i] = 0; }
      if(1 == nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength)) {
        /* Authenticate on sector 1 using KEY_A */
        if(1 == nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, 1, 0, nfcKey)) {
          uint8_t surname_[17] = {0};
          if(1 == nfc.mifareclassic_ReadDataBlock(4, surname_)) {
            uint8_t name_[17] = {0};
            if(1 == nfc.mifareclassic_ReadDataBlock(5, name_)) {
              cnc_print_cmdGet_tbd(nfcTagName); Serial.print(surname_); Serial.print(" "); Serial.println(name_); Serial.flush();
            }
          }
        }
      }
    }
    previousTime_1s = currentTime;
  }
  /* HK @ 0.1Hz */
  if((uint32_t)(currentTime - previousTime_10s) >= 10000) {
    previousTime_10s = currentTime;
  }
}
