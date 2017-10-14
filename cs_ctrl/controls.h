/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file controls.h
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#ifndef CONTROLS_H_
#define CONTROLS_H_


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
void reset_controls(void);
void init_controls(void);
uint16_t add_control( control_sensor_block_t *ctrl_blk, uint16_t *entry_no );
void process_controls( wiced_time_t current_time );
void set_control_arg( uint16_t control_no, uint16_t arg );
void load_config_defaults_generic_ccb( uint16_t arg );

#endif /* CONTROLS_H_ */
