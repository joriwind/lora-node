

#include "random.h"
#include <stdio.h>
#include <stdint.h>
#include "mbed.h"
#include "stm32l0xx_hal_rng.h"
//#include "stm32l0xx_hal_def.h"

RNG_HandleTypeDef handler;

#define MAX_ATTEMPTS    20

void seed_random(){
    printf("Seeding huhu \r\n");
    uint8_t err;

    __HAL_RCC_RNG_CLK_ENABLE();
    //Setting RNG base!!
    handler.Instance = RNG;
    
    err = HAL_RNG_Init(&handler);
    if (0 != err){
        printf("Could not initialise rng: %x\r\n", err);
    }

    /* first random number generated after setting the RNGEN bit should not be used */
    HAL_RNG_GetRandomNumber(&handler);
}

uint16_t get_16bit(){
    printf("Getting rng 16bit...\r\n");
    uint32_t rn;
    uint8_t err;

    err = HAL_RNG_GenerateRandomNumber(&handler, &rn);
    if (0 != err){
        printf("Could not generate rn from rng: %x\r\n", err);
        return 0;
    }
    printf("RN: %u\r\n", (uint16_t)rn);

    return rn;
}


uint16_t get_random_in_range(uint16_t min, uint16_t max){
    printf("Getting rn in range\r\n");
    uint32_t rn;
    uint8_t err;
    uint8_t i;

    for(i = 0; i < MAX_ATTEMPTS; i++){
        err = HAL_RNG_GenerateRandomNumber(&handler, &rn);
        if (0 != err){
            printf("Could not generate rn from rng: %x\r\n", err);
            return 0;
        }
        if(rn > min && rn < max){
            printf("RN: %u after %u attempts\r\n", (uint16_t)rn, i);
            return rn;
        }
    }
    printf("Did not find suitable rn!\r\n", (uint16_t)rn);

    return 0;
}