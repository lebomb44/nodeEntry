#include <CnC.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

#define LIGHT_PIN 13

const char nodeName[] PROGMEM = "entry";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "val";
const char cmdGetName[] PROGMEM = "get";
const char cmdSetName[] PROGMEM = "set";

const char pingName[] PROGMEM = "ping";
const char lightModeName[] PROGMEM = "lightMode";
const char nfcFirmwareVersionName[] PROGMEM = "nfcFirmwareVersion";
const char nfcTargetIDName[] PROGMEM = "nfcTargetID";
const char nfcKeyName[] PROGMEM = "nfcKey";
const char nfcAuthenticateBlockName[] PROGMEM = "nfcAuthenticateBlock";
const char nfcReadBlockName[] PROGMEM = "nfcReadBlock";
const char nfcWriteBlockName[] PROGMEM = "nfcWriteBlock";
const char nfcDumpName[] PROGMEM = "nfcDump";
const char nfcFormatName[] PROGMEM = "nfcFormat";
const char nfcModeName[] PROGMEM = "nfcMode";
const char nfcTagName[] PROGMEM = "nfcTag";

uint32_t previousTime_1s = 0;
uint32_t previousTime_10s = 0;
uint32_t previousTime_light = 0;
uint32_t currentTime = 0;

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);

uint8_t lightMode = 0;
uint8_t nfcUID[7] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t nfcUIDLength = 0;
uint8_t nfcKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t nfcMode = 1;

void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, currentTime); }
void lightMode_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(lightModeName, lightMode); }
void lightMode_cmdSet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    lightMode = strtoul(args[3], NULL, 16);
  }
}
void nfcFirmwareVersion_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(nfcFirmwareVersionName, nfc.getFirmwareVersion()); }
void nfcTargetID_cmdGet(int arg_cnt, char **args) {
  for(uint8_t i=0; i<7; i++) { nfcUID[i] = 0; }
  nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength);
  cnc_print_cmdGet_tbd(nfcTargetIDName);
  for(uint8_t i=0; i<7; i++) { cnc_Serial_get()->print(nfcUID[i]/16, HEX); cnc_Serial_get()->print(nfcUID[i]%16, HEX); }
  cnc_Serial_get()->println(); cnc_Serial_get()->flush();
}
void nfcKey_cmdGet(int arg_cnt, char **args) {
  cnc_print_cmdGet_tbd(nfcKeyName);
  for(uint8_t i=0; i<6; i++) { cnc_Serial_get()->print(nfcKey[i]/16, HEX); cnc_Serial_get()->print(nfcKey[i]%16, HEX); }
  cnc_Serial_get()->println(); cnc_Serial_get()->flush();
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
  else { cnc_print_cmdGet_tbd(nfcAuthenticateBlockName); cnc_Serial_get()->println("ARG_ERROR"); cnc_Serial_get()->flush(); }
}
void nfcReadBlock_cmdGet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    uint32_t blockNumber_ = strtoul(args[3], NULL, 10);
    uint8_t data_[16] = {0};
    if(1 == nfc.mifareclassic_ReadDataBlock(blockNumber_, data_)) {
      cnc_print_cmdGet_tbd(nfcReadBlockName);
      for(uint8_t i=0; i<16; i++) { cnc_Serial_get()->print(data_[i]/16, HEX); cnc_Serial_get()->print(data_[i]%16, HEX); }
      cnc_Serial_get()->print(" ");
      for(uint8_t i=0; i<16; i++) { if(isPrintable(data_[i])) { cnc_Serial_get()->print(data_[i]); } else { cnc_Serial_get()->print("."); } }
      cnc_Serial_get()->println(); cnc_Serial_get()->flush();
    }
    else { cnc_print_cmdGet_tbd(nfcReadBlockName); cnc_Serial_get()->println("READ_ERROR"); cnc_Serial_get()->flush(); }
  }
  else { cnc_print_cmdGet_tbd(nfcReadBlockName); cnc_Serial_get()->println("ARG_ERROR"); cnc_Serial_get()->flush(); }
}
void nfcWriteBlock_cmdGet(int arg_cnt, char **args) {
  if(5 == arg_cnt) {
    uint32_t blockNumber_ = strtoul(args[3], NULL, 10);
    uint8_t data_[16] = {0};

    for(uint8_t i=0; i<16; i++) { data_[i] = 0; }
    for(uint8_t i=0; i<16; i++) {
      if(0 == args[4][i]) { break; }
      data_[i] = args[4][i];
    }
    if(1 == nfc.mifareclassic_WriteDataBlock(blockNumber_, data_)) {
      cnc_print_cmdGet_tbd(nfcWriteBlockName);
      cnc_Serial_get()->println("OK"); cnc_Serial_get()->flush();
    }
    else { cnc_print_cmdGet_tbd(nfcWriteBlockName); cnc_Serial_get()->println("WRITE_ERROR"); cnc_Serial_get()->flush(); }
  }
  else { cnc_print_cmdGet_tbd(nfcWriteBlockName); cnc_Serial_get()->println("ARG_ERROR"); cnc_Serial_get()->flush(); }
}
void nfcDump_cmdGet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    uint8_t keyNumber_ = strtoul(args[3], NULL, 10);
    /* Read all blocks from tag */
    for(uint8_t i=0; i<16; i++) {
      cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->print("SECTOR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush();
      if(0 == nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, 4*i, keyNumber_, nfcKey)) {
          cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->print("AUT_ERROR "); cnc_Serial_get()->print(i); cnc_Serial_get()->print(" with key "); cnc_Serial_get()->println(keyNumber_); cnc_Serial_get()->flush(); return;
      }
      for(uint8_t j=0; j<4; j++) {
        uint8_t data_[16] = {0};
        if(0 == nfc.mifareclassic_ReadDataBlock((4*i)+j, data_)) {
          cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->print("DATA_ERROR "); cnc_Serial_get()->println((4*i)+j); cnc_Serial_get()->flush(); return;
        }
        cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->print("DATA "); cnc_Serial_get()->print((4*i)+j); cnc_Serial_get()->print(" ");
        for(uint8_t k=0; k<16; k++) {
          cnc_Serial_get()->print(data_[k]/16, HEX); cnc_Serial_get()->print(data_[k]%16, HEX);
        }
        cnc_Serial_get()->println(); cnc_Serial_get()->flush();
      }
    }
    cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->println("OK end"); cnc_Serial_get()->flush();
  }
  else { cnc_print_cmdGet_tbd(nfcDumpName); cnc_Serial_get()->println("ARG_ERROR"); cnc_Serial_get()->flush(); }
}
void nfcFormat_cmdGet(int arg_cnt, char **args) {
  if(5 == arg_cnt) {
    uint8_t surname_[16] = {0};
    uint8_t name_[16] = {0};
    uint8_t data_[16] = {0};
    uint8_t keys_[16] = {0};

    /* Surname */
    for(uint8_t i=0; i<16; i++) { surname_[i] = 0; }
    for(uint8_t i=0; i<16; i++) {
      if(0 == args[3][i]) { break; }
      surname_[i] = args[3][i];
    }
    /* Name */
    for(uint8_t i=0; i<16; i++) { name_[i] = 0; }
    for(uint8_t i=0; i<16; i++) {
      if(0 == args[4][i]) { break; }
      name_[i] = args[4][i];
    }
    /* Data */
    for(uint8_t i=0; i<16; i++) { data_[i] = 0; }
    /* Keys */
    keys_[0] = 0x1A; keys_[1] = 0xCF; keys_[2] = 0xFC; keys_[3] = 0x1D; keys_[4] = 0xEB; keys_[5] = 0x90; 
    keys_[6] = 0x78; keys_[7] = 0x77; keys_[8] = 0x88; keys_[9] = 0x44;
    keys_[10] = 0xEB; keys_[11] = 0x90; keys_[12] = 0x1A; keys_[13] = 0xCF; keys_[14] = 0xFC; keys_[15] = 0x1D; 
    /* Write all blocks to tag */
    for(uint8_t i=0; i<16; i++) {
      if(0 == nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, 4*i, 0, nfcKey)) {
        cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("AUT_ERROR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush(); return;
      }
      if(0 != i) {
        if(0 == nfc.mifareclassic_WriteDataBlock(4*i, surname_)) {
          cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("SURNAME_ERROR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush(); return;
        }
      }
      if(0 == nfc.mifareclassic_WriteDataBlock((4*i)+1, name_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("NAME_ERROR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush(); return;
      }
      if(0 == nfc.mifareclassic_WriteDataBlock((4*i)+2, data_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("DATA_ERROR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush(); return;
      }
      if(0 == nfc.mifareclassic_WriteDataBlock((4*i)+3, keys_)) {
        cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("KEYS_ERROR "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush(); return;
      }
      cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->print("OK "); cnc_Serial_get()->println(i); cnc_Serial_get()->flush();
    }
    cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->println("OK end"); cnc_Serial_get()->flush();
  }
  else { cnc_print_cmdGet_tbd(nfcFormatName); cnc_Serial_get()->println("ARG_ERROR"); cnc_Serial_get()->flush(); }
}
void nfcMode_cmdGet(int arg_cnt, char **args) {
  cnc_print_cmdGet_u32(nfcModeName, nfcMode);
}
void nfcMode_cmdSet(int arg_cnt, char **args) {
  if(4 == arg_cnt) {
    nfcMode = strtoul(args[3], NULL, 16);
  }
}

void setup() {
  cncInit(nodeName, &Serial);
  cnc_Serial_get()->begin(115200);
  cnc_hkName_set(hkName);
  cnc_cmdGetName_set(cmdGetName);
  cnc_cmdSetName_set(cmdSetName);
  cnc_sepName_set(sepName);
  cnc_cmdGet_Add(pingName, ping_cmdGet);
  cnc_cmdGet_Add(lightModeName, lightMode_cmdGet);
  cnc_cmdSet_Add(lightModeName, lightMode_cmdSet);
  cnc_cmdGet_Add(nfcFirmwareVersionName, nfcFirmwareVersion_cmdGet);
  cnc_cmdGet_Add(nfcTargetIDName, nfcTargetID_cmdGet);
  cnc_cmdGet_Add(nfcKeyName, nfcKey_cmdGet);
  cnc_cmdSet_Add(nfcKeyName, nfcKey_cmdSet);
  cnc_cmdGet_Add(nfcAuthenticateBlockName, nfcAuthenticateBlock_cmdGet);
  cnc_cmdGet_Add(nfcReadBlockName, nfcReadBlock_cmdGet);
  cnc_cmdGet_Add(nfcWriteBlockName, nfcWriteBlock_cmdGet);
  cnc_cmdGet_Add(nfcDumpName, nfcDump_cmdGet);
  cnc_cmdGet_Add(nfcFormatName, nfcFormat_cmdGet);
  cnc_cmdGet_Add(nfcModeName, nfcMode_cmdGet);
  cnc_cmdSet_Add(nfcModeName, nfcMode_cmdSet);

  previousTime_1s = millis();
  previousTime_10s = previousTime_1s;
  previousTime_light = previousTime_1s;

  nfc.begin();
  nfc.SAMConfig();
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, HIGH);
}

void loop() {
  currentTime = millis(); cncPoll();
  /* Light blink */
  if((uint32_t)(currentTime - previousTime_light) >= 100) {
    if(0 == lightMode) { digitalWrite(LIGHT_PIN, LOW); }
    else if(1 == lightMode) { digitalWrite(LIGHT_PIN, HIGH); }
    else if(2 == lightMode) { digitalWrite(LIGHT_PIN, !digitalRead(LIGHT_PIN)); }
    else { digitalWrite(LIGHT_PIN, LOW); }
    previousTime_light = currentTime;
  }
  /* HK @ 1.0Hz */
  if((uint32_t)(currentTime - previousTime_1s) >= 1000) {
    if(1 == nfcMode) {
      if(1 == nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength)) {
        /* Authenticate on sector 1 using KEY_A */
        if(1 == nfc.mifareclassic_AuthenticateBlock(nfcUID, nfcUIDLength, 4, 0, nfcKey)) {
          char surname_[17] = {0};
          if(1 == nfc.mifareclassic_ReadDataBlock(4, surname_)) {
            char name_[17] = {0};
            if(1 == nfc.mifareclassic_ReadDataBlock(5, name_)) {
              cnc_print_cmdGet_tbd(nfcTagName); cnc_Serial_get()->print(surname_); cnc_Serial_get()->print(" "); cnc_Serial_get()->println(name_); cnc_Serial_get()->flush();
            }
            else { cnc_print_cmdGet_tbd(nfcTagName); cnc_Serial_get()->println("ERROR reading block 5"); cnc_Serial_get()->flush(); }
          }
          else { cnc_print_cmdGet_tbd(nfcTagName); cnc_Serial_get()->println("ERROR reading block 4"); cnc_Serial_get()->flush(); }
        }
        else { cnc_print_cmdGet_tbd(nfcTagName); cnc_Serial_get()->println("ERROR authenticate block 4 with KEY_A"); cnc_Serial_get()->flush(); }
        while(nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, nfcUID, &nfcUIDLength)) { cncPoll(); }
      }
    }
    previousTime_1s = currentTime;
  }
  /* HK @ 0.1Hz */
  if((uint32_t)(currentTime - previousTime_10s) >= 10000) {
    previousTime_10s = currentTime;
  }
}
