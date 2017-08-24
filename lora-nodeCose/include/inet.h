#ifndef INET_H
#define INET_H

#include "stdint.h"

/* definition to expand macro then apply to pragma message */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "= "  VALUE(var)
#pragma message(VAR_NAME_VALUE(BYTE_ORDER))

#ifdef  __cplusplus
extern "C" {
#endif

/* #if BYTE_ORDER == BIG_ENDIAN
#define lwip_htons(x) (x)
#define lwip_ntohs(x) (x)
#define lwip_htonl(x) (x)
#define lwip_ntohl(x) (x)
#define PP_HTONS(x) (x)
#define PP_NTOHS(x) (x)
#define PP_HTONL(x) (x)
#define PP_NTOHL(x) (x)
#else // BYTE_ORDER != BIG_ENDIAN  */
#ifndef lwip_htons
uint16_t lwip_htons(uint16_t x);
#endif
#define lwip_ntohs(x) lwip_htons(x)

#ifndef lwip_htonl
uint32_t lwip_htonl(uint32_t x);
#endif
#define lwip_ntohl(x) lwip_htonl(x)

/* Provide usual function names as macros for users, but this can be turned off */
#ifndef LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#define htons(x) lwip_htons(x)
#define ntohs(x) lwip_ntohs(x)
#define htonl(x) lwip_htonl(x)
#define ntohl(x) lwip_ntohl(x)
#endif

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

//#endif /* BYTE_ORDER == BIG_ENDIAN */

#ifdef  __cplusplus
}
#endif

#endif //INET_H