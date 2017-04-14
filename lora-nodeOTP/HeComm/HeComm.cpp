#include "HeComm.h"
#include "board.h"

/*
 * Key to be used during E2E communication
 */
static uint8_t heCommSKey[HECOMM_KEY_LENGTH];

#ifdef HECOMM_PAD
    FogCommStatus ospad(uint8_t* input, uint8_t inputSz, uint8_t* key, uint8_t keySz, uint8_t* output, uint8_t outputSz);
#endif

#ifdef HECOMM_OSCOAP

#endif

FogCommStatus heCommSetSessionKey(uint8_t* sessionKey, uint8_t size){
    if(size != HECOMM_KEY_LENGTH){    //Check size of key
        return HECOMM_STATUS_ERROR_KEY_LENGTH;
    }

    for(int i = 0; i< size; i++){   //Copy key into storage
        heCommSKey[i] = sessionKey[i];
    }

    return HECOMM_STATUS_OK;
}

FogCommStatus heCommEncrypt(uint8_t* buffer, uint8_t bufferSize, uint8_t* encBuffer, uint8_t encBufferSz){
#ifdef HECOMM_PAD
    return ospad(buffer, bufferSize, heCommSKey, HECOMM_KEY_LENGTH, encBuffer, encBufferSz);   

#elif

#endif
}

FogCommStatus heCommDecrypt(uint8_t* buffer, uint8_t bufferSize, uint8_t* decBuffer, uint8_t decBufferSz){
#ifdef HECOMM_PAD
    return ospad(buffer, bufferSize, heCommSKey, HECOMM_KEY_LENGTH, decBuffer, decBufferSz);   

#elif

#endif
}


#ifdef HECOMM_PAD
    FogCommStatus ospad(uint8_t* input, uint8_t inputSz, uint8_t* key, uint8_t keySz, uint8_t* output, uint8_t outputSz)
    {
        //Control checks
        if(keySz < inputSz){
            return HECOMM_STATUS_ERROR_PAD;
        }
        if(outputSz < inputSz){
            return HECOMM_STATUS_ERROR;
        }

        for(int i = 0; i < inputSz; i++){
            output[i] = input[i] ^ key[i]; //XOR message and key
        }

        return HECOMM_STATUS_OK;
    }
#endif

#ifdef HECOMM_OSCOAP


#endif


