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

#include "../common.h"
#include "../storage.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../cli/interface.h"
#include "../cs_ctrl/imx_cs_interface.h"
#include "../wifi/wifi.h"

#include "at_cmds.h"
/******************************************************
 *                      Macros
 ******************************************************/
/*
 *  Set up standard variables based on the type of data we are using
 */
#define SET_CSB_VARS( type )    \
                if( type == IMX_CONTROLS ) {        \
                    csb = &device_config.ccb[ 0 ];  \
                    csd = cd[ 0 ];                 \
                } else {                            \
                    csb = &device_config.scb[ 0 ];  \
                    csd = sd[ 0 ];                 \
                }
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
static bool print_register( imx_peripheral_type_t type, uint16_t entry );
static void at_print( char *response );
/******************************************************
 *               Variable Definitions
 ******************************************************/
const char *imx_data_types[ IMX_NO_DATA_TYPES ] =
{
        "32 Bit Unsigned",
        "32 Bit signed",
        "32 Bit Float",
        "Variable Length",
};
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
extern control_sensor_data_t *sd[];
extern control_sensor_data_t *cd[];
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
	uint16_t at_register, result, i, reg_width;
	imx_peripheral_type_t type;
	imx_data_32_t value;
	char *token;

	reg_width = 0;
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
	    type = IMX_CONTROLS;
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
	        cli_print( "&IT -  Print the UTC time in seconds since 1970 - 0 if NTP cannot get time\r\n" );
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
                icb.print_debugs = false;   // When enforced reset
                imatrix_save_config();
            } else if( token[ 1 ] == '1' ) {
                device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD;
                icb.print_debugs = false;   // When enforced reset
                imatrix_save_config();
            } else if( token[ 1 ] == '2' ) {
                device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD_STATUS;
                icb.print_debugs = false;   // When enforced reset
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
        } else if( strncmp( token, "&IT", 3 ) == 0x00 ) {
            /*
             * Process &IT - print UTC time
             */
            uint32_t utc_time;
            if( icb.time_set_with_NTP == true )
                wiced_time_get_utc_time( (wiced_utc_time_t *) &utc_time );
            else
                utc_time = 0;
            cli_print( "%lu\r\n", utc_time );
		} else {
		    /*
		     * Unknown command
		     */
		    cli_print( "Unknown Command: %s\r\n", token );
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
	            if( isdigit( (int) token[ 4 ] ) != 0x00 ) {
	                at_register = at_register * 10 + ( token[ 4 ] - 0x30 );
	                reg_width = 1;
	            } else
	                reg_width = 0;
	        } else {
	            cli_print( "No Register Supplied\r\n" );
	            icb.AT_command_errors += 1;
	            at_print( AT_RESPONSE_ERROR );
	            return;
	        }
	        if( token[ 4 + reg_width ] == '=' ) {
	            /*
	             * Save value
	             */
	            if( imx_parse_value( type, at_register, &token[ 5 + reg_width ], &value ) == false ) {
                    icb.AT_command_errors += 1;
                    at_print( AT_RESPONSE_ERROR );
                    return;
	            }
	            if( imx_set_control_sensor( type, at_register, &value ) != IMX_SUCCESS ) {
	                icb.AT_command_errors += 1;
	                at_print( AT_RESPONSE_ERROR );
	                return;
	            }
	        } else if( token[ 4 + reg_width ] == '?' ) {
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
/**
  * @brief  print a register managed by AT commands
  * @param  None
  * @retval : None
  */

static bool print_register( imx_peripheral_type_t type, uint16_t entry )
{

    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;

    SET_CSB_VARS( type );

    if( type == IMX_CONTROLS ) {
        if( entry >= device_config.no_controls )
            return false;
        csd = cd[ 0 ];
        csb = &device_config.ccb[ 0 ];
    } else {
        if( entry >= device_config.no_sensors )
            return false;
        csd = sd[ 0 ];
        csb = &device_config.scb[ 0 ];
    }
    switch( csb[ entry].data_type ) {
        case IMX_INT32 :
            cli_print( "%d", csd[ entry].last_value.int_32bit );
            break;
        case IMX_FLOAT :
            cli_print( "%f", csd[ entry].last_value.float_32bit );
            break;
        case IMX_UINT32 :
        default :
            cli_print( "%u", csd[ entry].last_value.uint_32bit );
            break;
    }

    cli_print( "\r\n" );
    return true;
}

static void at_print( char *response )
{
    if( device_config.AT_verbose != IMX_AT_VERBOSE_NONE )
        cli_print( response );
}
