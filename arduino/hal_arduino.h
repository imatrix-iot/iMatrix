/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file hal_arduino.h
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#ifndef HAL_ARDUINO_H_
#define HAL_ARDUINO_H_


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
void init_arduino(void);
void configure_arduino(void);
void process_arduino( wiced_time_t current_time );
uint16_t update_arduino_control(uint16_t arg, void *value );
void init_arduino_sensor( uint16_t arg );
uint16_t sample_arduino_sensor(uint16_t arg, void *value );
void init_arduino_control( uint16_t arg );
uint16_t update_arduino(uint16_t arg, void *value );
uint16_t set_arduino(uint16_t arg, void *value );
void print_arduino( uint16_t arg );

#endif /* HAL_ARDUINO_H_ */
