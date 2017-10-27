/*
 * $Copyright 2013-2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file controls.c
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "../imatrix.h"
#include "storage.h"
#include "../cli/interface.h"
#include "controls.h"
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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern control_sensor_block_t imx_controls_defaults[];
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	Load the defaults in the config for a generic device
  * @param  sensor no assigned to device
  * @retval : None
  */
void load_config_defaults_generic_ccb( uint16_t arg )
{
	memcpy( &device_config.ccb[ arg ], &imx_controls_defaults[ arg ], sizeof( control_sensor_block_t ) );
}

