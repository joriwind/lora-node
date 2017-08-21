#ifndef __BOARD_H__
#define __BOARD_H__

#include "mbed.h"
#include "system/timer.h"
//#include "debug.h"
#include "system/utilities.h"
#include "sx1272-hal.h"

#define USE_BAND_868
#define NDEBUG

extern SX1272MB2xAS Radio;

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInit( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardGetBatteryLevel( void );

#endif //__BOARD_H__