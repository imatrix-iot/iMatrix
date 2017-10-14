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

#ifndef SENSOR_RECORDER_H_
#define SENSOR_RECORDER_H_

/** @file sensors.h
 *
 *	Defines for sensor data
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
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
void reset_sensors(void);
void init_sensors(void);
uint16_t add_sensor( control_sensor_block_t *ctrl_blk, uint16_t *entry_no );
void sample_sensors( wiced_time_t current_time );
void set_sensor_arg( uint16_t control_no, uint16_t arg );
void load_config_defaults_generic_scb( uint16_t arg );
void print_sensors(void);
#endif /* SENSOR_RECORDER_H_ */
