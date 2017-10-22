/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/*
 * .h
 *
 *  Created on: Jan 01, 2017
 *      Author: greg.phillips
 */

#ifndef HAL_LEDS_H_
#define HAL_LEDS__H_

/** @file hal_led.h
 *
 *	Defines for functions in hal_led.c
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
bool set_host_led( imx_led_t led, imx_led_state_t value );
void cli_set_led( uint16_t arg);
#endif /* HAL_LEDS__H_ */
