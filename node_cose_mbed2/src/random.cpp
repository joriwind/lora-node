

#include "random.h"
#include <stdio.h>
#include <stdint.h>
#include "mbed.h"
#include "stm32l0xx_hal_rng.h"
//#include "stm32l0xx_hal_def.h"

RNG_HandleTypeDef hrng;

void seed_random(){
    __HAL_RCC_RNG_CLK_ENABLE ();
    printf("Seeding???\r\n");
    uint8_t err;

    err = HAL_RNG_Init(&hrng);
    if (0 != err){
        printf("Could not initialise rng: %x\r\n", err);
    }
}

uint16_t get_16bit(){
    printf("Getting rng 16bit...\r\n");
    uint32_t rn;
    uint8_t err;

    err = HAL_RNG_GenerateRandomNumber(&hrng, &rn);
    if (0 != err){
        printf("Could not generate rn from rng: %x\r\n", err);

    }
    printf("RN: %lu\r\n", rn);

    return rn;
}