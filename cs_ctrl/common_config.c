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
 *  Created on: October 25, 2017
 *      Author: greg.phillips
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"

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
static void print_csb_entry( imx_peripheral_type_t type, imx_control_sensor_block_t *csb, uint16_t entry );
/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern control_sensor_data_t *cd;
extern control_sensor_data_t *sd;
extern imx_control_sensor_block_t imx_controls_defaults[], imx_sensors_defaults[];
extern imx_functions_t imx_control_functions[], imx_sensor_functions[];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  reset the controls & sensor system
  *
  * Add all controls and sensors on our own platform.
  * @param:  None
  * @retval : None
  */
void cs_reset_defaults(void)
{
    imx_peripheral_type_t type;
    imx_control_sensor_block_t *csb, *config_source;
    uint16_t no_items, i;

    for( type = IMX_CONTROLS; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
        if( type == IMX_CONTROLS ) {
            csb = &device_config.ccb[ 0 ];
            config_source = &imx_controls_defaults[ 0 ];
        } else {
            csb = &device_config.scb[ 0 ];
            config_source = &imx_sensors_defaults[ 0 ];
        }
        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

//        imx_cli_print( "Setting up %s Data @: 0x%08lx - Adding %u entries\r\n", ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", (uint32_t ) config_source, no_items );
        for( i = 0; i < no_items; i++ ) {
            memcpy( &csb[ i ], &config_source[ i ], sizeof( imx_control_sensor_block_t ) );
//            imx_cli_print( "    Added" );
//            print_csb_entry( type, csb, i );
        }
    }
}
/**
 *  @brief  initialize the controls & sensors system
  * @param  None
  * @retval : None
  */
void cs_init(void)
{
    imx_peripheral_type_t type;
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;
    imx_functions_t *f;
    uint16_t no_items, i;
    wiced_time_t current_time;

    /*
     * Set last update time as now
     */
    wiced_time_get_time( &current_time );   // Current time in mS

    for( type = 0; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
        if( type == IMX_CONTROLS ) {
            csd = &cd[ 0 ];
            csb = &device_config.ccb[ 0 ];
            f = &imx_control_functions[ 0 ];
        } else {
            csd = &sd[ 0 ];
            csb = &device_config.scb[ 0 ];
            f = &imx_sensor_functions[ 0 ];
        }
        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

//        imx_cli_print( "Setting up %s Data @: 0x%08lx - Adding %u entries\r\n", ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", (uint32_t ) csd, no_items );
        for( i = 0; i < no_items; i++ ) {
            if( csb[ i ].set_default == true ) {
                csd[ i ].last_value.uint_32bit = csb[ i ].default_value.uint_32bit;
                csd[ i ].valid = true;
                csd[ i ].last_sample_time = current_time + ( i * 1000 );
            } else
                csd[ i ].valid = false;    // Not valid until set for the first time
            if( f[ i ].init != NULL ) {
                (f[ i ].init)( f[ i ].arg );    // Initialize control
                csd[ i ].last_sample_time = current_time + ( i * 1000 );
            }
        }
    }
}

/**
  * @brief  Load the defaults in the config for a generic device
  * @param  sensor no assigned to device
  * @retval : None
  */
void load_config_defaults_generic_ccb( uint16_t arg )
{
    memcpy( &device_config.ccb[ arg ], &imx_controls_defaults[ arg ], sizeof( imx_control_sensor_block_t ) );
}

/**
  * @brief print the details of the controls & sensors configuration
  * @param  None
  * @retval : None
  */
void print_common_config( imx_peripheral_type_t type, imx_control_sensor_block_t *csb )
{
    uint16_t i, no_items;


    no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;
    for( i = 0; i < no_items; i++ ) {
        print_csb_entry( type, csb, i );
    }

}

static void print_csb_entry( imx_peripheral_type_t type, imx_control_sensor_block_t *csb, uint16_t entry )
{
    imx_cli_print( " No. %2u, %32s, ID: 0x%08lx, ", entry, csb[  entry ].name, csb[ entry ].id );
    switch( csb[ entry ].data_type ) {
        case IMX_INT32 :
            imx_cli_print( "32bit INT  " );
            break;
        case IMX_FLOAT :
            imx_cli_print( "32bit Float" );
            break;
        case IMX_VARIABLE_LENGTH :
            imx_cli_print( "Variable   " );
            break;
        case IMX_UINT32 :
        default :
            imx_cli_print( "32bit UINT " );
            break;
    }

    imx_cli_print( ", %s-", csb[ entry ].enabled == true ? " Enabled" : "Disabled" );
    imx_cli_print( "%s, ", ( csb[ entry ].read_only == true ) ? " Read Only" : "Read/Write" );
    imx_cli_print( "Initialize: " );
    if( csb[ entry ].set_default == false )
        imx_cli_print( "None  " );
    else {
        switch( csb[ entry ].data_type ) {
            case IMX_UINT32 :
                imx_cli_print( "%6lu", csb[ entry ].default_value.uint_32bit );
                break;
            case IMX_INT32 :
                imx_cli_print( "%6ld", csb[ entry ].default_value.int_32bit );
                break;
            case IMX_FLOAT :
                imx_cli_print( "%6.2f", csb[ entry ].default_value.float_32bit );
                break;
            case IMX_VARIABLE_LENGTH :
                imx_cli_print( "Variable " );
                break;
        }
    }
    imx_cli_print( ", " );

    if( csb[ entry ].sample_rate == 0 )
        imx_cli_print( "          Event Driven" );
    else {
        if( csb[ entry ].sample_rate >= 1000 )
            imx_cli_print( "Sample Every: %03.1f Sec", ( (float) csb[ entry ].sample_rate ) / 1000.0 );
        else
            imx_cli_print( "Sample Every: %03u mSec", csb[ entry ].sample_rate );
    }
    imx_cli_print( ", Batch size: %2u", csb[ entry ].sample_batch_size );

    imx_cli_print( ", Monitoring Levels, Low enabled:" );
    if( csb[ entry ].use_warning_level_low == 0 )
        imx_cli_print( " None" );
    else {
        imx_cli_print( "%s%s%s",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
        imx_cli_print( ", High Level Settings: Watch_level: ");
        switch( csb[ entry ].data_type ) {
            case IMX_UINT32 :
                imx_cli_print( "%lu", csb[ entry ].warning_level_high[ 0 ].uint_32bit );
                break;
            case IMX_INT32 :
                imx_cli_print( "%ld", csb[ entry ].warning_level_high[ 0 ].int_32bit );
                break;
            case IMX_FLOAT :
                imx_cli_print( "%f", csb[ entry ].warning_level_high[ 0 ].float_32bit );
                break;
        }
    }
    imx_cli_print( ", High enabled:" );
    if( csb[ entry ].use_warning_level_high == 0 )
        imx_cli_print( " None" );
    else {
        imx_cli_print( "%s%s%s",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                ( ( csb[ entry ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
        imx_cli_print( " Low Level Settings: Watch: ");
        switch( csb[ entry ].data_type ) {
            case IMX_UINT32 :
                imx_cli_print( "%lu", csb[ entry ].warning_level_low[ 0 ].uint_32bit );
                break;
            case IMX_INT32 :
                imx_cli_print( "%ld", csb[ entry ].warning_level_low[ 0 ].int_32bit );
                break;
            case IMX_FLOAT :
                imx_cli_print( "%f", csb[ entry ].warning_level_low[ 0 ].float_32bit );
                break;
        }
        imx_cli_print( ", Advisory: ");
        switch( csb[ entry ].data_type ) {
            case IMX_UINT32 :
                imx_cli_print( "%lu", csb[ entry ].warning_level_low[ 1 ].uint_32bit );
                break;
            case IMX_INT32 :
                imx_cli_print( "%ld", csb[ entry ].warning_level_low[ 1 ].int_32bit );
                break;
            case IMX_FLOAT :
                imx_cli_print( "%f", csb[ entry ].warning_level_low[ 1 ].float_32bit );
                break;
        }
        imx_cli_print( ", Warning: ");
        switch( csb[ entry ].data_type ) {
            case IMX_UINT32 :
                imx_cli_print( "%lu", csb[ entry ].warning_level_low[ 2 ].uint_32bit );
                break;
            case IMX_INT32 :
                imx_cli_print( "%ld", csb[ entry ].warning_level_low[ 2 ].int_32bit );
                break;
            case IMX_FLOAT :
                imx_cli_print( "%f", csb[ entry ].warning_level_low[ 2 ].float_32bit );
                break;
        }
    }
    imx_cli_print( "\r\n" );
    /*      printf( "Loading defaults from Controls Defaults: %u, %s, id: %u, sample rate: %u, batch size: %u, percent change: %u, enabled: %u, send on percent: %u, data type: %u\r\n", i,
                    imx_controls_defaults[ i ].name,
                    (uint16_t) imx_controls_defaults[ i ].id,
                    (uint16_t) imx_controls_defaults[ i ].sample_rate,
                    (uint16_t) imx_controls_defaults[ i ].sample_batch_size,
                    (uint16_t) imx_controls_defaults[ i ].percent_change_to_send,
                    (uint16_t) imx_controls_defaults[ i ].enabled,
                    (uint16_t) imx_controls_defaults[ i ].send_on_percent_change,
                    (uint16_t) imx_controls_defaults[ i ].data_type );
    */

}
