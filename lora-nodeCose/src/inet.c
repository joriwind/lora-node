#include "inet.h"

//#if BYTE_ORDER == LITTLE_ENDIAN

#if !defined(lwip_htons)
/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
uint16_t lwip_htons(uint16_t n)
{
  return (uint16_t)PP_HTONS(n);
}
#endif /* lwip_htons */

#if !defined(lwip_htonl)
/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
uint32_t lwip_htonl(uint32_t n)
{
  return (uint32_t)PP_HTONL(n);
}
#endif /* lwip_htonl */

//#endif /* BYTE_ORDER == LITTLE_ENDIAN */

