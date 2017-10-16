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
 * so agrees to indemnify Sierra against all liability.
 */
/** @file cli_set_ssid.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../device/hal_leds.h"
#include "../wifi/wifi.h"

#include "cli_set_ssid.h"
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
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief Set the SSID and drop Wi Fi so unit will come back in ST mode
  * @param  None
  * @retval : None
  */
void cli_set_ssid( uint16_t arg)
{
	UNUSED_PARAMETER(arg);
	char *token, buffer1[ IMX_SSID_LENGTH + 1 ], buffer2[ IMX_WPA2PSK_LENGTH ];

	/*
	 * Add ability to set type of security
	 */
	token = strtok(NULL, " " );
	if( token ) {
		if( strlen( token ) <= IMX_SSID_LENGTH ) {
			strcpy( buffer1, token );
		} else {
			cli_print( "SSID name too long\r\n" );
			return;
		}
		token = strtok(NULL, " " );
		if( token ) {
			if( strlen( token ) <= IMX_WPA2PSK_LENGTH ) {
				strcpy( buffer2, token );
			} else {
				cli_print( "Pass phrase too long\r\n" );
				return;
			}
			strcpy( device_config.st_ssid, buffer1 );
			strcpy( device_config.st_wpa, buffer2 );
			set_wifi_ap_ssid( buffer1, buffer2, IMX_DEFAULT_AP_SECURITY );
			icb.wifi_up = false;
			if( device_config.AP_setup_mode == true ) { // We were in set up mode - turn off the blinking led
			    set_host_led( IMX_LED_RED, IMX_LED_OFF );           // Set RED Led to off
			    device_config.AP_setup_mode = false;
			}
			imatrix_save_config();
			cli_print( "SSID changing\r\n" );
		} else {
			cli_print( "Pass phrase Required\r\n" );
		}
	} else
		cli_print( "SSID Required\r\n" );

}
