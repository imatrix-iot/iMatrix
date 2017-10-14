/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file arduino_spi.h
 *
 *  Created on: Feb 18, 2017
 *      Author: greg.phillips
 */

#ifndef ARDUINO_SPI_H_
#define ARDUINO_SPI_H_


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
uint16_t init_arduino_spi(void);
uint16_t read_smart_arduino( uint32_t start_register, uint16_t count, void *ptr );
uint16_t write_smart_arduino( uint32_t start_register, uint16_t count, void *ptr );
uint16_t read_arduino( uint16_t count, void *ptr );
uint16_t write_arduino( uint16_t count, void *ptr );
#endif /* ARDUINO_SPI_H_ */
