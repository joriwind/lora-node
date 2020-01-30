#ifndef __RANDOM_H__
#define __RANDOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void seed_random();

uint16_t get_16bit();

uint16_t get_random_in_range(uint16_t min, uint16_t max);

#ifdef __cplusplus
}
#endif

#endif //__RANDOM_H__