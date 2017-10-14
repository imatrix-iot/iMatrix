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

/** @file imatrix.c
 *
 *  Created on: October 8, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "imatrix.h"
#include "storage.h"
#include "device/config.h"
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
 ******************************************************/
extern iMatrix_Control_Block_t icb;
extern IOT_Device_Config_t device_config;
extern imx_imatrix_init_config_t imatrix_init_config;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Initialize the iMatrix System
  * @param  flag to set up to run in background
  * @retval : status of initialization
  */
imx_status_t imx_init( imx_imatrix_init_config_t *init_config, bool override_config, bool run_in_background )
{

    init_storage();     // Start clean
    /*
     * Save user defined product information to local storage
     */
    memcpy( &imatrix_init_config, init_config, sizeof( imx_imatrix_init_config_t ) );
    /*
     * Load current config or factory default if none stored
     */

    imatrix_load_config();
    if( ( device_config.application_loaded == false ) || ( override_config == true ) ) {
        /*
         * Copy factory entries to device_config
         */

    }
    /*
     * Use the provided parameters to set up the system
     */
    if( run_in_background ) {
        /*
         * Spawn the imx_process as a background process
         */
        icb.running_in_background = true;
    }

    return IMX_NO_ERROR;
}
imx_status_t imx_process(void)
{
    return IMX_NO_ERROR;
}
imx_status_t imx_deinit(void)
{
    if( icb.running_in_background == true ) {
        /*
         * Shut down background process
         */
        ;
    }
    return IMX_NO_ERROR;
}
