//
//  Determine which cryptographic library we are goig to be usig
//

#define USE_MBED_TLS

#if defined(USE_MBED_TLS)
#if defined(USE_OPEN_SSL) || defined(USE_BCRYPT)
#error Only Define One Crypto Package
#endif
#elif defined(USE_BCRYPT)
#if defined(USE_OPENSSL)
#error Only Define One Crypto Package
#endif
#elif !defined(USE_OPEN_SSL)
#define USE_OPEN_SSL
#endif

//
//  Define which AES CCM algorithms are being used
//

#define USE_AES_CCM_16_64_128
//#define USE_AES_CCM_16_64_256
//#define USE_AES_CCM_64_64_128
//#define USE_AES_CCM_64_64_256
//#define USE_AES_CCM_16_128_128
//#define USE_AES_CCM_16_128_256
//#define USE_AES_CCM_64_128_128
/*#define USE_AES_CCM_64_128_256*/

#define INCLUDE_AES_CCM

//
//  Define which HMAC-SHA algorithms are being used
//

#define USE_HMAC_256_64
//#define USE_HMAC_256_256
/*#define USE_HMAC_384_384
#define USE_HMAC_512_512*/
#if defined(USE_HMAC_256_64) || defined(USE_HMAC_256_256) || defined(USE_HMAC_384_384) || defined(USE_HMAC_512_512)
#define USE_HMAC
#endif



//#define USE_COUNTER_SIGNATURES
