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
static void print_cs_block_entry( peripheral_type_t type, imx_control_sensor_block_t *cs_block, uint16_t entry );
/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern control_sensor_data_t cd[ MAX_NO_CONTROLS ], sd[ MAX_NO_SENSORS ];
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
    peripheral_type_t type;
    imx_control_sensor_block_t *cs_block, *config_source;
    uint16_t no_items, i;

    for( type = 0; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
        if( type == IMX_CONTROLS ) {
            cs_block = &device_config.ccb[ 0 ];
            config_source = &imx_controls_defaults[ 0 ];
        } else {
            cs_block = &device_config.scb[ 0 ];
            config_source = &imx_sensors_defaults[ 0 ];
        }
        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

        cli_print( "Setting up %s Data @: 0x%08lx - Adding %u entries\r\n", ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", (uint32_t ) config_source, no_items );
        for( i = 0; i < no_items; i++ ) {
            memcpy( &cs_block[ i ], &config_source[ i ], sizeof( imx_control_sensor_block_t ) );
            imx_printf( "  Added" );
            print_cs_block_entry( type, cs_block, i );
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
    peripheral_type_t type;
    control_sensor_data_t *data;
    imx_control_sensor_block_t *cs_block;
    imx_functions_t *f;
    uint16_t no_items, i;
    wiced_time_t current_time;

    /*
     * This data is in CCMSRAM - Initialize to zeros
     */
    memset( &cd, 0x00, sizeof( cd ) );
    memset( &sd, 0x00, sizeof( sd ) );
    /*
     * Set last update time as now
     */
    wiced_time_get_time( &current_time );   // Current time in mS

    for( type = 0; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
        if( type == IMX_CONTROLS ) {
            data = &cd[ 0 ];
            cs_block = &device_config.ccb[ 0 ];
            f = &imx_control_functions[ 0 ];
        } else {
            data = &sd[ 0 ];
            cs_block = &device_config.scb[ 0 ];
            f = &imx_sensor_functions[ 0 ];
        }
        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

        cli_print( "Setting up %s Data @: 0x%08lx - Adding %u entries\r\n", ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", (uint32_t ) data, no_items );
        for( i = 0; i < no_items; i++ ) {
            if( cs_block[ i ].set_default == true ) {
                data[ i ].last_value.uint_32bit = cs_block[ i ].default_value.uint_32bit;
                cs_block[ i ].valid = true;
            }
            if( f[ i ].init != NULL )
                (f[ i ].init)( f[ i ].arg );    // Initialize control
            data[ i ].update_now = true;
            data[ i ].last_sample_time = current_time + ( i * 1000 );
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
void print_common_config( peripheral_type_t type, imx_control_sensor_block_t *cs_block )
{
    uint16_t i, no_items;


    no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;
    for( i = 0; i < no_items; i++ ) {
        print_cs_block_entry( type, cs_block, i );
    }

}

static void print_cs_block_entry( peripheral_type_t type, imx_control_sensor_block_t *cs_block, uint16_t entry )
{
    cli_print( " No. %u, %32s, ID: 0x%08lx, %s, ", entry, cs_block[  entry ].name, cs_block[ entry ].id, cs_block[ entry ].enabled == true ? " Enabled" : "Disabled" );
    cli_print( "%s, ", ( cs_block[ entry ].read_only == true ) ? " Read Only" : "Read/Write" );
    if( cs_block[ entry ].sample_rate == 0 )
        imx_printf( "          Event Driven" );
    else {
        if( cs_block[ entry ].sample_rate >= 1000 )
            imx_printf( "Sample Every: %4.1f Sec", ( (float) cs_block[ entry ].sample_rate ) / 1000.0 );
        else
            imx_printf( "Sample Every: %5u mSec", cs_block[ entry ].sample_rate );
    }
    imx_printf( ", Batch size: %2u", cs_block[ entry ].sample_batch_size );

    cli_print( ", Monitoring Levels low enabled:" );
    if( cs_block[ entry ].use_warning_level_low == 0 )
        cli_print( " None" );
    else {
        cli_print( "%s%s%s",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
        cli_print( ", High Level Settings: Watch_level: ");
        switch( cs_block[ entry ].data_type ) {
            case IMX_UINT32 :
                cli_print( "%lu", cs_block[ entry ].warning_level_high[ 0 ].uint_32bit );
                break;
            case IMX_INT32 :
                cli_print( "%ld", cs_block[ entry ].warning_level_high[ 0 ].int_32bit );
                break;
            case IMX_FLOAT :
                cli_print( "%f", cs_block[ entry ].warning_level_high[ 0 ].float_32bit );
                break;
        }
    }
    cli_print( ", Monitoring Levels high enabled:" );
    if( cs_block[ entry ].use_warning_level_high == 0 )
        cli_print( " None" );
    else {
        cli_print( "%s%s%s",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                ( ( cs_block[ entry ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
        cli_print( " Low Level Settings: Watch: ");
        switch( cs_block[ entry ].data_type ) {
            case IMX_UINT32 :
                cli_print( "%lu", cs_block[ entry ].warning_level_low[ 0 ].uint_32bit );
                break;
            case IMX_INT32 :
                cli_print( "%ld", cs_block[ entry ].warning_level_low[ 0 ].int_32bit );
                break;
            case IMX_FLOAT :
                cli_print( "%f", cs_block[ entry ].warning_level_low[ 0 ].float_32bit );
                break;
        }
        cli_print( ", Advisory: ");
        switch( cs_block[ entry ].data_type ) {
            case IMX_UINT32 :
                cli_print( "%lu", cs_block[ entry ].warning_level_low[ 1 ].uint_32bit );
                break;
            case IMX_INT32 :
                cli_print( "%ld", cs_block[ entry ].warning_level_low[ 1 ].int_32bit );
                break;
            case IMX_FLOAT :
                cli_print( "%f", cs_block[ entry ].warning_level_low[ 1 ].float_32bit );
                break;
        }
        cli_print( ", Warning: ");
        switch( cs_block[ entry ].data_type ) {
            case IMX_UINT32 :
                cli_print( "%lu", cs_block[ entry ].warning_level_low[ 2 ].uint_32bit );
                break;
            case IMX_INT32 :
                cli_print( "%ld", cs_block[ entry ].warning_level_low[ 2 ].int_32bit );
                break;
            case IMX_FLOAT :
                cli_print( "%f", cs_block[ entry ].warning_level_low[ 2 ].float_32bit );
                break;
        }
    }
    cli_print( "\r\n" );
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
