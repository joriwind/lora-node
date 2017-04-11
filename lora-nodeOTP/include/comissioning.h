#ifndef __LORA_COMMISSIONING_H__
#define __LORA_COMMISSIONING_H__

/*
 * Device identifier
 * -- Over-the-air information, same as LoRaWAN-demo72 from SEMTECH
 */
#define LORAWAN_DEV_EUI     { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 }

/*
 * Application identifier
 */
#define LORAWAN_APP_EUI { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

/*
 * Application key
 */
#define LORAWAN_APP_KEY     { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }


#endif //__LORA_COMMISSIONING_H__