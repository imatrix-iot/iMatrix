/*
 * Copyright 2017, Sierra Telecom. All Rights Reserved.
 *
 * This software, associated documentation and materials ("Software"),
 * is owned by Sierra Telecom ("Sierra") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Sierra
 * reserves the right to make changes to the Software without notice. Sierra
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Sierra does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Sierra product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Sierra's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Sierra against all liability.
 */

/** @file .c
 *
 *  Created on: October 1, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "wiced.h"
#include "storage.h"
#include "device/icb_def.h"
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
 ******************************************************/\
 imx_imatrix_init_config_t imatrix_init_config;
control_sensor_data_t cd[ MAX_NO_CONTROLS ] CCMSRAM;
control_sensor_data_t sd[ MAX_NO_SENSORS ] CCMSRAM;
iMatrix_Control_Block_t icb CCMSRAM;
IOT_Device_Config_t device_config CCMSRAM;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief Initialize all the storage - CCMSRAM is uninitialized
  * @param  None
  * @retval : None
  */

void init_storage(void)
{

    memset( &imatrix_init_config, 0x00, sizeof( imx_imatrix_init_config_t ) );
    memset( &cd, 0x00, sizeof( cd ) );
    memset( &sd, 0x00, sizeof( sd ) );
    memset( &icb, 0x00, sizeof( iMatrix_Control_Block_t ) );
    memset( &device_config, 0x00, sizeof( IOT_Device_Config_t ) );

}
