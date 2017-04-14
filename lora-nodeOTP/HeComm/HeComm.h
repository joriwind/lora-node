#ifndef __HECOMM_H__
#define __HECOMM_H__

#include "HeComm-def.h"
#include "board.h"


/* Function declaration */
FogCommStatus heCommSetSessionKey(uint8_t* sessionKey, uint8_t size);
FogCommStatus heCommEncrypt(uint8_t* buffer, uint8_t bufferSize, uint8_t* encBuffer, uint8_t encBufferSz);
FogCommStatus heCommDecrypt(uint8_t* buffer, uint8_t bufferSize, uint8_t* decBuffer, uint8_t decBufferSz);

#endif //__HECOMM_H__
