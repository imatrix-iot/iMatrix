/*
 * $Copyright 2013-2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file arduino_spi.c
 *
 *  Created on: Feb 18, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"
#include "platform.h"

#include "../../../defines.h"
#include "../../../system.h"
#include "../../../hal.h"
#include "../../../hal/hal_spi.h"
#include "../../../device/config.h"
#include "../../../device/dcb_def.h"
#include "../../../cli/interface.h"
#include "../../../arduino.h"
#include "../generated/product.h"

#include "arduino_spi.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define SOH     0x02
#define EOT     0x04

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
void write_nop_arduino( uint16_t count );
/******************************************************
 *               Variable Definitions
 ******************************************************/
wiced_spi_device_t arduino_spi =
{
    .port        = ARDUINO_SPI_PORT,
    .chip_select = ARDUINO_SPI_CS,
    .speed       = ARDUINO_SPI_INTERFACE_SPEED,
    .mode        = ARDUINO_SPI_MODE,
    .bits        = 8
};
extern dcb_t dcb;
extern IOT_Device_Config_t device_config;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	Initialize the SPI for the Arduino and copy down the configuration
  * @param  None
  * @retval : None
  */
uint16_t init_arduino_spi(void)
{
	uint16_t result;
	device_configuration_t config;

	init_spi( &arduino_spi );
	return 0;
	/*
	 * Send out some NOPs to ensure device is synced
	 */
	write_nop_arduino( 1 );
	/*
	 * Read the configuration register to see if the device matches the ID we expect
	 */
	result = read_smart_arduino( SPI_CONFIG_REG, ARDUINO_REG_LENGTH, &config );

	if( ( result != IMATRIX_SUCCESS ) ||
		( device_config.acb.config.arduino_code != config.arduino_code ) ||
		( device_config.acb.config.no_controls != config.no_controls ) ||
		( device_config.acb.config.no_sensors != config.no_sensors ) ) {
		 print_status( "Invalid Arduino ID: 0x%04x, Expected: 0x%04x, No Controls: %u, No Sensors: %u\r\n", config.arduino_code, device_config.acb.config.arduino_code, config.no_controls, config.no_sensors );
		return IMATRIX_BAD_ARDUINO;
	}
	return IMATRIX_SUCCESS;
}
/**
  * @brief	Send NOP to Arduino to ensure it is synced and ready to receive a command
  * @param  None
  * @retval : None
  */
void write_nop_arduino( uint16_t count )
{
    uint8_t tx_data[ SPI_COMMAND_LENGTH ];
    uint16_t i;
	wiced_result_t wiced_result;
    wiced_spi_message_segment_t segments[1];

//	print_status( "About Send NOPs to Arduino SPI Interface\r\n" );

	wiced_result = init_spi( &arduino_spi );
	if ( wiced_result != IMATRIX_SUCCESS ) {
//		 print_status( "Arduino SPI Init Failed: %d\r\n", wiced_result );
		return;
	}

	tx_data[ 0 ] = SPI_NOP;
	tx_data[ 1 ] = 0;
	tx_data[ 2 ] = 0;
	segments[0].tx_buffer = &tx_data;
	segments[0].length = 3;
	segments[0].rx_buffer = NULL;

	for( i = 0; i < count; i++ ) {

	    /* Transfer one segments */
	    wiced_result = wiced_spi_transfer( &arduino_spi, segments, 1 );
	    if ( wiced_result ) {
//	        print_status( "SPI Transfer Failed: %d\r\n", wiced_result );
	        deinit_spi();
	        dcb.spi_errors += 1;
	        return;
	    }
	    wiced_rtos_delay_milliseconds( 50 );
	}
    deinit_spi();

}
/**
  * @brief	Read a register(s) from the Arduino
  * @param  start register, number of bytes to read, point to location to store data
  * @retval : status
  */
uint16_t read_smart_arduino( uint32_t start_register, uint16_t count, void *ptr )
{
    uint8_t tx_data[ SPI_COMMAND_LENGTH ];
    uint8_t rx_data[ MAX_SPI_RESPONSE ];
	wiced_result_t wiced_result;
    wiced_spi_message_segment_t segments[1];

//	print_status( "About to initialize Arduino SPI Interface\r\n" );

	wiced_result = init_spi( &arduino_spi );
	if ( wiced_result ) {
		print_status( "Arduino SPI Init Failed: %d\r\n", wiced_result );
		return IMATRIX_SPI_ERROR;
	}

//	print_status( "Sending read regs command\r\n" );

	tx_data[ SPI_SOH ] = SOH;
	tx_data[ SPI_COMMAND ] = SPI_READ_REGS;
	tx_data[ SPI_REGISTER ] = start_register;
	tx_data[ SPI_LENGTH ] = count;
	tx_data[ SPI_EOT ] = EOT;
	segments[0].tx_buffer = &tx_data;
	segments[0].length = SPI_COMMAND_LENGTH;
	segments[0].rx_buffer = NULL;

    /* Transfer one segments */
    wiced_result = wiced_spi_transfer( &arduino_spi, segments, 1 );
    if ( wiced_result ) {
//        print_status( "SPI Transfer Failed: %d\r\n", wiced_result );
        deinit_spi();
        dcb.spi_errors += 1;
        return IMATRIX_SPI_ERROR;
    }
    /*
     * Give the device some time to prepare
     */
    wiced_rtos_delay_milliseconds( 100 );
	segments[0].tx_buffer = NULL;
	segments[0].length = count + 2;
	segments[0].rx_buffer = rx_data;

    /* Transfer one segments */
    wiced_result = wiced_spi_transfer( &arduino_spi, segments, 1 );
    if ( wiced_result ) {
//        print_status( "SPI Transfer Failed: %d\r\n", wiced_result );
        deinit_spi();
        dcb.spi_errors += 1;
        return IMATRIX_SPI_ERROR;
    }
	wiced_result = deinit_spi();
	/*
	 * Verify data
	 */
	if( ( rx_data[ SPI_SOH ] == SOH ) && ( rx_data[ count + 1 ] == EOT ) ) {
	    /*
	     * Valid Data - copy to callers buffer
	     */
	    memcpy( ptr, &rx_data[ 1 ], count );
	} else
	    wiced_result = WICED_ERROR;

	if( wiced_result != WICED_SUCCESS )
		return wiced_result;
	return IMATRIX_SUCCESS;
}
/**
  * @brief	Write a register(s) to the Arduino
  * @param  start register, number of bytes to write, point to location to store data
  * @retval : status
  */
uint16_t write_smart_arduino( uint32_t start_register, uint16_t count, void *ptr )
{
    uint8_t tx_data[ MAX_SPI_RESPONSE ];
	wiced_result_t wiced_result;
    wiced_spi_message_segment_t segments[1];

//	print_status( "About to initialize Arduino SPI Interface\r\n" );

    if( count > ( MAX_SPI_RESPONSE ) )
        return IMATRIX_SPI_ERROR;

	wiced_result = init_spi( &arduino_spi );
	if ( wiced_result ) {
//		print_status( "Arduino SPI Init Failed: %d\r\n", wiced_result );
		return IMATRIX_SPI_ERROR;
	}

//	print_status( "Sending read regs command\r\n" );

    tx_data[ SPI_SOH ] = SOH;
    tx_data[ SPI_COMMAND ] = SPI_WRITE_REGS;
    tx_data[ SPI_REGISTER ] = start_register;
    tx_data[ SPI_LENGTH ] = count;
    tx_data[ SPI_EOT ] = EOT;
    segments[0].tx_buffer = &tx_data;
    segments[0].length = SPI_COMMAND_LENGTH;
    segments[0].rx_buffer = NULL;

    /* Transfer one segments */
    wiced_result = wiced_spi_transfer( &arduino_spi, segments, 1 );
    if ( wiced_result ) {
//        print_status( "SPI Transfer Failed: %d\r\n", wiced_result );
        deinit_spi();
        dcb.spi_errors += 1;
        return IMATRIX_SPI_ERROR;
    }
    /*
     * Give the device some time to prepare
     */
    wiced_rtos_delay_milliseconds( 100 );
    tx_data[ SPI_SOH ] = SOH;
    memcpy( &tx_data[ SPI_COMMAND ], ptr, count );
    tx_data[ count + 1 ] = EOT;

	segments[0].tx_buffer = tx_data;
	segments[0].length = count;
	segments[0].rx_buffer = NULL;

    /* Transfer one segments */
    wiced_result = wiced_spi_transfer( &arduino_spi, segments, 1 );
    if ( wiced_result ) {
//        print_status( "SPI Transfer Failed: %d\r\n", wiced_result );
        deinit_spi();
        dcb.spi_errors += 1;
        return IMATRIX_SPI_ERROR;
    }
	wiced_result = deinit_spi();

	if( wiced_result != WICED_SUCCESS )
		return wiced_result;
	return IMATRIX_SUCCESS;
}
