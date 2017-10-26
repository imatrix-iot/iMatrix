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

/** @file at_cmds.c
 *
 *  Created on: April 7, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cs_ctrl/simulated.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../cli/interface.h"
#include "../wifi/wifi.h"

#include "at_cmds.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define AT_RESPONSE_OK      "OK\r\n"
#define AT_RESPONSE_ERROR   "ERROR\r\n"
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
void at_print( char *response );
/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief process the AT command
  * @param  arg - unused
  * @retval : None
  */
void cli_at( uint16_t arg )
{
	UNUSED_PARAMETER(arg);
	bool process_ct = false;
	uint16_t at_register, result, base, count, i;
	peripheral_type_t type;
	char *token;

	/*
	 * Statistics collection
	 */
	icb.AT_commands_processed += 1;
	/*
	 * Get the command
	 */
	token = strtok( NULL, " " );

	while( token != NULL ) {
	    /*
	     * Upper case string to make it easier to pass
	     */
	    type = 0;
	    i = 0;
	    while( token[ i ] != 0x00 ) {
	        token[ i ] = (char) toupper( (int) token[ i ] );
	        i++;
	    }
	    if( strcmp( token, "?" ) == 0x00 ) {
	        cli_print( "AT Commands Supported by CLI\r\n" );
	        cli_print( "E - 0 Turn Off Echo | 1 - Turn On Echo\r\n" );
	        cli_print( "V - 0 No Responses | 1 - Standard Responses | 2 - Include Status messages\r\n" );
	        cli_print( "&ICn - ? - Get value for Control register n | = <xx> - Set Control Register n to value xxx. Value must match data type\r\n" );
	        cli_print( "&IP - Set Provisioning Mode\r\n" );
	        cli_print( "&ISn - ? - Get value for Sensor register n\r\n" );
	        cli_print( "System Defined with the following Controls & Sensors\r\n" );
	        count = device_config.no_at_controls;
	        base = device_config.at_control_start;
	        for( at_register = 0; at_register < count; at_register++ )
	            cli_print( "C%u - %s\r\n", at_register, device_config.ccb[ base + at_register  ].name );
            count = device_config.no_at_sensors;
	        base = device_config.at_sensor_start;
            for( at_register = 0; at_register < count; at_register++ )
                cli_print( "S%u - %s\r\n", at_register, device_config.scb[ base + at_register ].name );
	    } else if( token[ 0 ] == 'E') {
	        /*
	         * Process E
	         */
	        if( token[ 1 ] == '0' ) {
	            device_config.AT_echo = false;
	            imatrix_save_config();
	        } else if( token[ 1 ] == '1' ) {
                device_config.AT_echo = true;
                imatrix_save_config();
            } else {
                at_print( AT_RESPONSE_ERROR );
                return;
            }
	    } else if( token[ 0 ] == 'V') {
            /*
             * Process V
             */
            if( token[ 1 ] == '0' ) {
                device_config.AT_verbose = IMX_AT_VERBOSE_NONE;
                imatrix_save_config();
            } else if( token[ 1 ] == '1' ) {
                device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD;
                imatrix_save_config();
            } else if( token[ 1 ] == '2' ) {
                device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD_STATUS;
                imatrix_save_config();
            } else {
                at_print( AT_RESPONSE_ERROR );
                return;
            }
	    } else if( strcmp( token, "&IP" ) == 0x00 ) {
	        /*
	         * Enter Provisioning mode
	         */
	        cli_wifi_setup( 0 );
	    } else if( strncmp( token, "&IC", 3 ) == 0x00 ) {
	        /*
	         * Process &IC
	         */
	        process_ct = true;
	        type = IMX_CONTROLS;
		} else if( strncmp( token, "&IS", 3 ) == 0x00 ) {
	        /*
	         * Process &IS
	         */
		    process_ct = true;
		    type = IMX_SENSORS;
		} else {
		    /*
		     * Unknown command
		     */
		    icb.AT_command_errors += 1;
		    at_print( AT_RESPONSE_ERROR );
			return;
		}
	    if( process_ct == true ) {
	        if( isdigit( (int) token[ 3 ] ) != 0x00 ) {
	            /*
	             * get the register
	             */
	            at_register = token[ 3 ] - 0x30;
	        } else {
	            imx_printf( "No Register Supplied\r\n" );
	            icb.AT_command_errors += 1;
	            at_print( AT_RESPONSE_ERROR );
	            return;
	        }
	        if( token[ 4 ] == '=' ) {
	            /*
	             * Save value
	             */
	            result = set_register( type, at_register, &token[ 5 ] );
	        } else if( token[ 4 ] == '?' ) {
	            /*
	             * Display value
	             */
	            result = print_register( type, at_register );
	        } else {
	            icb.AT_command_errors += 1;
	            at_print( AT_RESPONSE_ERROR );
	            return;
	        }
	        if( result == false ) {
	            icb.AT_command_errors += 1;
	            at_print( AT_RESPONSE_ERROR );
	            return;
	        }

	    }
	    /*
	     * Get next command
	     */
	    token = strtok( NULL, " " );
	}
	/*
	 * Fell thru must be good at this point
	 */
	at_print( AT_RESPONSE_OK );
}

void at_print( char *response )
{
    if( device_config.AT_verbose != IMX_AT_VERBOSE_NONE )
        cli_print( response );
}
