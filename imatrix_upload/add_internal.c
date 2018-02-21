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

/** @file add_internal.c
 *
 *  Created on: May 15, 2017
 *      Author: greg.phillips
 *
 *      Add data from an internal control / sensor
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../imatrix_upload/registration.h"
#include "../networking/utility.h"

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
extern IOT_Device_Config_t device_config;   // Defined in device\config.h
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  add registration event to the payload
  * @param  pointer to payload, utc time
  * @retval : None
  */
void add_registration( upload_data_t **upload_data, uint16_t *remaining_data_length, wiced_utc_time_ms_t upload_utc_ms_time )
{

    uint32_t foo32bit;
    bits_t header_bits;

    imx_printf( "Registering product: %s - %s ", device_config.device_name, device_config.device_serial_number );
    /*
     * Set the data type to UINT32 and load the entry
     *
     * Entry is just time in sec + value of 1
     */
    /*
     * Registration
     */
    (*upload_data)->header.id = htonl( IMX_INTERNAL_SENSOR_THING_EVENT );
    header_bits.bits.data_type = IMX_UINT32;  // Floating point data for GPS
    (*upload_data)->header.sample_rate = htonl( 0 );
    header_bits.bits.block_type = IMX_BLOCK_EVENT_SENSOR;

    header_bits.bits.no_samples = 2;
    header_bits.bits.warning = 0;
    header_bits.bits.sensor_error = 0;
    header_bits.bits.version = IMATRIX_VERSION_1;
    header_bits.bits.reserved = 0;
    (*upload_data)->header.bits.bit_data = htonl( header_bits.bit_data );

    /*
     * Load Defaults
     */
    (*upload_data)->header.last_utc_ms_sample_time = 0;
    (*upload_data)->data[ 0 ].uint_32bit = 0;

    if( icb.time_set_with_NTP == true ) {
        (*upload_data)->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );
        (*upload_data)->data[ 0 ].uint_32bit = htonl( (uint32_t) ( upload_utc_ms_time / 1000L ) );  // Time in UTC Sec
    }
    (*upload_data)->data[ 1 ].uint_32bit = htonl( IMX_EVENT_REGISTRATION );
    /*
    * Update the pointer and amount number of bytes left in buffer
    */
    foo32bit = sizeof( header_t ) + ( IMX_SAMPLE_LENGTH * 2 );
    (*upload_data) = ( upload_data_t *) ( ( uint32_t) ( *upload_data ) + foo32bit );
    *remaining_data_length -= foo32bit;
    /*
    * Update the pointer and amount number of bytes left in buffer
    */
    foo32bit = sizeof( header_t ) + ( IMX_SAMPLE_LENGTH );
    (*upload_data) = ( upload_data_t *) ( ( uint32_t) ( *upload_data ) + foo32bit );
    *remaining_data_length -= foo32bit;
    imx_printf( "Added %lu Bytes, %u Bytes remaining in packet\r\n", foo32bit * 3, *remaining_data_length );

    ack_registration();     // This needs to be set when the response is received
    return;
}

/**
  * @brief  add local location to the payload
  * @param  pointer to payload, utc time
  * @retval : None
  */
void add_indoor_location( upload_data_t **upload_data, uint16_t *remaining_data_length, wiced_utc_time_ms_t upload_utc_ms_time )
{
    return;
}

/**
  * @brief  add GPS coordinates to the payload
  * @param  pointer to payload
  * @retval : None
  */
void add_gps( upload_data_t **upload_data, uint16_t *remaining_data_length, wiced_utc_time_ms_t upload_utc_ms_time )
{
    uint32_t foo32bit;
    bits_t header_bits;

    imx_printf( "Adding GPS Location - Latitude: %f, Longitude: %f, Altitude: %f ", icb.latitude, icb.longitude, icb.elevation );
    /*
     * Set the data type to float and load the 3 entries
     */
    /*
     * Latitude
     */
    (*upload_data)->header.id = htonl( IMX_INTERNAL_SENSOR_GPS_LATITUDE );
    header_bits.bits.data_type = IMX_FLOAT;  // Floating point data for GPS
    (*upload_data)->header.sample_rate = htons( 0 );
    header_bits.bits.block_type = IMX_BLOCK_GPS_COORDINATES;

    header_bits.bits.no_samples = 1;
    header_bits.bits.warning = 0;
    header_bits.bits.sensor_error = 0;
    header_bits.bits.version = IMATRIX_VERSION_1;
    header_bits.bits.reserved = 0;
    (*upload_data)->header.bits.bit_data = htonl( header_bits.bit_data );

    (*upload_data)->header.last_utc_ms_sample_time = 0;
    if( icb.time_set_with_NTP == true )
        (*upload_data)->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );
    memcpy( &foo32bit, &icb.latitude, IMX_SAMPLE_LENGTH );
    (*upload_data)->data[ 0 ].uint_32bit = htonl( foo32bit );
    /*
    * Update the pointer and amount number of bytes left in buffer
    */
    foo32bit = sizeof( header_t ) + ( IMX_SAMPLE_LENGTH );
    (*upload_data) = ( upload_data_t *) ( ( uint32_t) ( *upload_data ) + foo32bit );
    *remaining_data_length -= foo32bit;
    /*
     * Longitude
     */
    (*upload_data)->header.id = htonl( IMX_INTERNAL_SENSOR_GPS_LONGITUDE );
    header_bits.bits.data_type = IMX_FLOAT;  // Floating point data for GPS
    (*upload_data)->header.sample_rate = htons( 0 );
    header_bits.bits.block_type = IMX_BLOCK_GPS_COORDINATES;

    header_bits.bits.no_samples = 1;
    header_bits.bits.warning = 0;
    header_bits.bits.sensor_error = 0;
    header_bits.bits.version = IMATRIX_VERSION_1;
    header_bits.bits.reserved = 0;
    (*upload_data)->header.bits.bit_data = htonl( header_bits.bit_data );

    (*upload_data)->header.last_utc_ms_sample_time = 0;
    if( icb.time_set_with_NTP == true )
        (*upload_data)->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );
    memcpy( &foo32bit, &icb.longitude, IMX_SAMPLE_LENGTH );
    (*upload_data)->data[ 0 ].uint_32bit = htonl( foo32bit );
    /*
    * Update the pointer and amount number of bytes left in buffer
    */
    foo32bit = sizeof( header_t ) + ( IMX_SAMPLE_LENGTH );
    (*upload_data) = ( upload_data_t *) ( ( uint32_t) ( *upload_data ) + foo32bit );
    *remaining_data_length -= foo32bit;
    /*
     * Elevation
     */
    (*upload_data)->header.id = htonl( IMX_INTERNAL_SENSOR_GPS_ELEVATION );
    header_bits.bits.data_type = IMX_FLOAT;  // Floating point data for GPS
    (*upload_data)->header.sample_rate = htons( 0 );
    header_bits.bits.block_type = IMX_BLOCK_GPS_COORDINATES;

    header_bits.bits.no_samples = 1;
    header_bits.bits.warning = 0;
    header_bits.bits.sensor_error = 0;
    header_bits.bits.version = IMATRIX_VERSION_1;
    header_bits.bits.reserved = 0;
    (*upload_data)->header.bits.bit_data = htonl( header_bits.bit_data );

    (*upload_data)->header.last_utc_ms_sample_time = 0;
    if( icb.time_set_with_NTP == true )
        (*upload_data)->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );
    memcpy( &foo32bit, &icb.elevation, IMX_SAMPLE_LENGTH );
    (*upload_data)->data[ 0 ].uint_32bit = htonl( foo32bit );
    /*
    * Update the pointer and amount number of bytes left in buffer
    */
    foo32bit = sizeof( header_t ) + ( IMX_SAMPLE_LENGTH );
    (*upload_data) = ( upload_data_t *) ( ( uint32_t) ( *upload_data ) + foo32bit );
    *remaining_data_length -= foo32bit;
    imx_printf( "Added %lu Bytes, %u Bytes remaining in packet\r\n", foo32bit * 3, *remaining_data_length );

    return;

}
