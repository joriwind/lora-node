#include "obj-sec.h"
#include "cose.h"
#include "string.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define OBJ_SEC_KEYSIZE 16

/* unsigned long  heapSize(); */


#define INIT_KEY {0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1};

static byte key[OBJ_SEC_KEYSIZE] = INIT_KEY;

cn_cbor_context ctx;

void objsec_init(cn_cbor_context context){
    //memcpy(key, INIT_KEY, OBJ_SEC_KEYSIZE);
    ctx = context;
}

void objsec_set_key(uint8_t *k){
    memcpy(key, k, OBJ_SEC_KEYSIZE);
    printf("Objsec: key set\r\n");
}

int16_t encrypt(uint8_t *buffer, uint16_t bufferSz, const uint8_t *message, size_t len) {
  //HCOSE_ENCRYPT  COSE_Encrypt_Init(COSE_INIT_FLAGS flags, CBOR_CONTEXT_COMMA cose_errback * perr);
  //bool COSE_Encrypt_SetContent(HCOSE_ENCRYPT cose, const byte * rgbContent, size_t cbContent, cose_errback * errp);
  //bool COSE_Encrypt_encrypt(HCOSE_ENCRYPT cose, const byte * pbKey, size_t cbKey, cose_errback * perror);
  cose_errback err;
  HCOSE_ENCRYPT objcose;
  uint8_t temp[1] = {""};
  size_t temp_len = 0;

  cn_cbor_errback cnerr;
  cn_cbor * algorithm;
  /* 
  //Check heap size
  unsigned long h = heapSize();
  printf("Size of heap: %lu\n", h);
  if(h <= 730){
    printf("Heap not large enough: %lu \n", h);
    return 0;
  } */

  /* INIT FLAGS
  COSE_INIT_FLAGS_NONE=0,
	COSE_INIT_FLAGS_DETACHED_CONTENT=1,
	COSE_INIT_FLAGS_NO_CBOR_TAG=2,
	COSE_INIT_FLAGS_ZERO_FORM=4
  */
  objcose = COSE_Encrypt_Init(COSE_INIT_FLAGS_NONE, &ctx, &err);
  
  if( objcose == NULL ) {
    PRINTF("Error in init cose: %i\n", err.err);
    return -1;
  }
  printf("Init done!\n");

  if( !COSE_Encrypt_SetContent(objcose, message, len, &err)){
    PRINTF("Error in set content cose: %i\n",err.err);
    goto errorReturn;
  }
  printf("Setcontent done!\n");


  algorithm = cn_cbor_int_create(COSE_Algorithm_AES_CCM_16_64_128, &ctx, &cnerr);
  if(algorithm == NULL){
    PRINTF("Could not allocate algorithm object: %u\n", cnerr.err);
  }

  if(!COSE_Encrypt_map_put_int(objcose, COSE_Header_Algorithm, algorithm, COSE_PROTECT_ONLY, &err)){
    PRINTF("Error in setting algorithm %i\n", err.err);
    goto errorReturn;
  }
  PRINTF("Algorithm set!\n");

  //Setting AAD
  if(!COSE_Encrypt_SetExternal(objcose, temp, temp_len, &err)){
    PRINTF("Error in setting AAD %i\n", err.err);
    goto errorReturn;
  }
  PRINTF("AAD set\n");

  if( !COSE_Encrypt_encrypt(objcose, key, OBJ_SEC_KEYSIZE, &err)){
    PRINTF("Error in encrypt cose: %i\n", err.err);
    goto errorReturn;
  }
  printf("Encrypt done!\n");


  size_t size = COSE_Encode((HCOSE) objcose, buffer, 0, bufferSz);
  printf("Buffer filled\n");
  COSE_Encrypt_Free(objcose);
  printf("Objcose released!\n");
  return size;

errorReturn:
  if (objcose != NULL) {
    printf("Releasing objcose\n");
    COSE_Encrypt_Free(objcose);
    printf("Objcose released!\n");
  }
  return -1;

}

/* unsigned long  heapSize()
{
   char   stackVariable;
   void   *heap;
   unsigned long result;
   heap  = malloc(4);
   result  = &stackVariable - (char *)heap;
   free(heap);
   return result;
} */