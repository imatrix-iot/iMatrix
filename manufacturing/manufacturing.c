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
 * If no EULA applies, Sierra hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Sierra's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Sierra.
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
 * so agrees to indemnity Sierra against all liability.
 */

/** @file manufacturing.c
 *
 *  Created on: May 7, 2017
 *      Author: greg.phillips
 *
 *      Notes for Manufacturing
 *
 *      Generate a build that does not have the GLOBAL_DEFINES += MAC_ADDRESS_SET_BY_HOST defined as we want to use the MAC from the OTP in the Wi Fi Chip
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"

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
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Manufacturing Test - Run the test requested
  * @param  Test or Function required
  * @retval : None
  */

void mfg_test( uint16_t arg )
{
    UNUSED_PARAMETER(arg);
    uint32_t test_no;
    char *token, *foo;

    /*
     * Get the Test / Function requested and execute appropriate
     *
     * Valid Test / Function
     *
     * 0    - Notify iMatrix Production Management System Test 0 started    - First Power Up and in test fixture testing
     * 1    - Notify iMatrix Production Management System Test 0 completed
     *
     * 2    - Notify iMatrix Production Management System Test 1 started    - Download latest Production Code
     * 3    - Notify iMatrix Production Management System Test 1 completed
     *
     * 4    - Notify iMatrix Production Management System Test 2 started    - Commence Final Production Testing
     * 5    - Notify iMatrix Production Management System Test 2 completed
     *
     *
     * 9    - Set to self provisioning mode
     * 10   - Set to Client Wi Fi Mode - ensure SSID are set
     */

    token = strtok(NULL, " " ); // Get start if any
    if( token ) {
        if( strcmp( token, "?" ) == 0x00 ) {
            /*
             * Print Help
             */
            imx_printf( "0    - Notify iMatrix Production Management System Test 0 started    - First Power Up and in test fixture testing\r\n" );
            imx_printf( "1    - Notify iMatrix Production Management System Test 0 completed\r\n" );
            imx_printf( "2    - Notify iMatrix Production Management System Test 1 started    - Download latest Production Code\r\n" );
            imx_printf( "3    - Notify iMatrix Production Management System Test 1 completed\r\n" );
            imx_printf( "4    - Notify iMatrix Production Management System Test 2 started    - Commence Final Production Testing\r\n" );
            imx_printf( "5    - Notify iMatrix Production Management System Test 2 completed\r\n" );
            imx_printf( "9    - Set to self provisioning mode, on next reboot\r\n" );
            imx_printf( "10   - Set to Client Wi-Fi Mode - ensure SSID are set, on next reboot\r\n" );
            return;
        } else if( strncmp( token, "0x", 2 ) == 0 )
            test_no = strtoul( &token[ 2 ], &foo, 16 );
        else
            test_no = strtoul( token, &foo, 10 );
    }
    else {
        imx_printf( "Test / Function number needed.\r\n" );
        return;
    }

    imx_printf( "Manufacturing Test/Function: %lu\r\n", test_no );
    switch( test_no ) {
        case 0 :
            break;
        case 1 :
            break;
        case 2 :
            break;
        case 3 :
            break;
        case 4 :
            break;
        case 5 :
            break;
        case 9 :    // Set to self provisioning mode
            imx_printf( "Setting to AP Mode - Provisioning\r\n" );
            device_config.AP_setup_mode = true;
            imatrix_save_config();
            break;
        case 10 :    // Set to self provisioning mode
            imx_printf( "Setting to Client Mode - Normal Network Connection assumed\r\n" );
            device_config.AP_setup_mode = false;
            imatrix_save_config();
            break;
    }
}
