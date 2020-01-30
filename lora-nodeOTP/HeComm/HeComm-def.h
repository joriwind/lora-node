#ifndef __HECOMM_DEF_H__
#define __HECOMM_DEF_H__

//Choose mode
#define HECOMM_PAD

#ifdef HECOMM_OSCOAP //Object security
#elif defined (HECOMM_PAD)   //Padding mode
#else
//should not occur
#endif

/*
 * The length of the key to use
 */
#define HECOMM_KEY_LENGTH 128

typedef enum FogCommStatus {
    HECOMM_STATUS_OK,
    HECOMM_STATUS_ERROR_KEY_LENGTH,
    HECOMM_STATUS_ERROR,
    HECOMM_STATUS_ERROR_PAD
} FogCommStatus;

#endif //__HECOMM_DEF_H__