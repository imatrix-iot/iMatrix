/*
 * $Copyright 2013-2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file hal_arduino.c
 *
 *  Created on: Feb 12, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../generated/product.h"
#include "../../../system.h"
#include "../../../defines.h"
#include "../../../hal.h"
#include "../../../arduino.h"
#include "../../../cli/interface.h"
#include "../../../device/config.h"
#include "../../../device/dcb_def.h"
#include "../common/controls.h"
#include "../common/sensors.h"
#include "../../../hal/hal_event.h"
#include "../../../time/ck_time.h"
#include "arduino_spi.h"

#include "hal_arduino.h"
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

static wiced_time_t arduino_last_update;
static uint16_t arduino_cscb_list[ MAX_NO_CONTROLS ] CCMSRAM;
static uint16_t arduino_sscb_list[ MAX_NO_SENSORS ] CCMSRAM;
extern control_sensor_data_t cd[ MAX_NO_CONTROLS ] CCMSRAM;
extern control_sensor_data_t sd[ MAX_NO_SENSORS ] CCMSRAM;
extern IOT_Device_Config_t device_config;
extern dcb_t dcb;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief init_arduino
  * @param  None
  * @retval : None
  */
void init_arduino(void)
{
	uint16_t i;

	print_status( "Initializing Arduino config: %u Bytes\r\n", sizeof( arduino_config_t ) );
	memset( &device_config.acb, 0x00, sizeof( arduino_config_t ) );

	device_config.acb.config.arduino_code = IMATRIX_ARDUINO_CODE;
	for( i = 0; i < NO_ARDUINO_CONTROLS; i++ )
		arduino_cscb_list[ i ] =  IMATRIX_UNUSED;
	for( i = 0; i < NO_ARDUINO_SENSORS; i++ )
		arduino_sscb_list[ i ] =  IMATRIX_UNUSED;

	if( NO_ARDUINO_CONTROLS > MAX_ARDUINO_CONTROLS )
		device_config.acb.config.no_controls = MAX_ARDUINO_CONTROLS;
	else
		device_config.acb.config.no_controls = NO_ARDUINO_CONTROLS;

	if( NO_ARDUINO_SENSORS > MAX_ARDUINO_SENSORS )
		device_config.acb.config.no_sensors = MAX_ARDUINO_SENSORS;
	else
		device_config.acb.config.no_sensors = NO_ARDUINO_SENSORS;

}
/**
  * @brief	Intitalize arduino control
  * @param  dummay arg
  * @retval : None
  */

void init_arduino_control( uint16_t arg )
{
	UNUSED_PARAMETER(arg);
	/*
	 * Nothing to do, device is responsible for this
	 */
}
/**
  * @brief Configure the initial settings for the Arduino
  * @param  None
  * @retval : None
  */
void configure_arduino(void)
{
	uint16_t status;
	/*
	 * Check if we have an arduino defined
	 */
	device_config.acb.config.no_controls = NO_ARDUINO_CONTROLS;
	device_config.acb.config.no_sensors = NO_ARDUINO_SENSORS;

	if( ( device_config.acb.config.no_controls == 0 ) &&
		( device_config.acb.config.no_sensors == 0 ) ) {
		dcb.arduino_valid = false;
		return;	// Nothing to do
	}
	status = init_arduino_spi();
	if( status != IMATRIX_SUCCESS ) {
		/*
		 * Failed to connect and intitalize register space, flag as arduino as invalid
		 */
		dcb.arduino_valid = false;
		return;
	}
	wiced_time_get_time( &arduino_last_update );
	dcb.arduino_valid = true;
}
/**
  * @brief Process the Arduino device
  * @param  current_time
  * @retval : None
  */
uint8_t led = 0;

void process_arduino( wiced_time_t current_time )
{
	uint16_t item, result, i;
	uint16_t reg_base, peripheral_base, peripheral, notify_event;
	uint32_t mask;
	data_32_t *data_ptr;


	if( is_later( current_time, arduino_last_update + 1000 ) == true ) {
		arduino_last_update = current_time;
		/*
		 * Simple test to change LED color using existing Cypress App
		 */
		if( dcb.arduino_valid == true ) {
			/*
			 * Read the status register and see if anything has changed
			 */
			if( read_smart_arduino( SPI_CSR_SLAVE, ARDUINO_REG_LENGTH, &device_config.acb.slave.status ) == IMATRIX_SUCCESS ) {
				if( device_config.acb.slave.status != 0 ) {
				    // print_status( "Arduino updates: 0x%08lx\r\n", device_config.acb.slave.status );
					/*
					 * find which entry needs to get updated
					 */
					mask = 0x03;

					for( i = 0; i < ( MAX_ARDUINO_CONTROLS + MAX_ARDUINO_SENSORS ); i++ ) {
					    wiced_rtos_delay_milliseconds( 100 );

						if( ( device_config.acb.slave.status & mask ) != 0 ) {
							if( i >= MAX_ARDUINO_SENSORS ) {        // Sensor data in low 16 bits
								/*
								 * This is a control -
								 */
								item = i - MAX_ARDUINO_SENSORS;
								reg_base = SPI_CONTROL_0;
								peripheral = CONTROLS;
								peripheral_base =  NO_INTEGRATED_CONTROLS;
								data_ptr = &device_config.acb.data_controls[ item ];
                                if( device_config.ccb[ peripheral_base + item ].sample_rate == 0 )   // An event has occurred
                                    notify_event = true;
                                else
                                    notify_event = false;
							} else {
                                /*
                                 * This is a sensor -
                                 */
                                item = i;
                                reg_base = SPI_SENSOR_0;
                                peripheral_base =  NO_INTEGRATED_SENSORS;
                                peripheral = SENSORS;
                                data_ptr = &device_config.acb.data_sensors[ item ];
                                if( device_config.scb[ peripheral_base + item ].sample_rate == 0 )   // An event has occurred
                                    notify_event = true;
                                else
                                    notify_event = false;
							}
                            result = read_smart_arduino( reg_base + item, ARDUINO_REG_LENGTH, data_ptr );
                            // print_status( "Reading %s item: %u, Value: uint32: %lu, int32: %ld, float: %f\r\n", peripheral == CONTROLS ? "Control" : "Sensor", item, data_ptr->uint_32bit, data_ptr->int_32bit, data_ptr->float_32bit );
                            if( result != WICED_SUCCESS )
                                print_status( "Error reading Arduino control: %u\r\n", item );
                            else {
                                if( notify_event ) {
                                    /*
                                     * We just read the value of the notification send this as an event, as controls/sensors with a sample rate of 0 are not uploaded
                                     */
                                    print_status( "Event Notification: Reading %s item: %u, Value: uint32: %lu, int32: %ld, float: %f\r\n", peripheral == CONTROLS ? "Control" : "Sensor", item, data_ptr->uint_32bit, data_ptr->int_32bit, data_ptr->float_32bit );
                                    hal_event( peripheral, peripheral_base + item, data_ptr );
                                }
                            }
						}
						mask = mask << 2;
					}
				}
			} else
			    print_status( "Failed to read Smart Arduino Status\r\n" );
			/*
			 * Check if we need to update any controls on the Arduino
			 */
			if( device_config.acb.master.status != 0 ) {
				/*
				 * find which entry needs to get updated
				 */
				mask = 0x03;
				for( i = 0; i < ( MAX_ARDUINO_CONTROLS + MAX_ARDUINO_SENSORS ); i++ ) {
					if( ( device_config.acb.master.status & mask ) != 0 ) {
						if( i >= MAX_ARDUINO_CONTROLS ) {
							/*
							 * This is a control -
							 */
							item = i - MAX_ARDUINO_CONTROLS;
							result = write_smart_arduino( SPI_CONTROL_0 + item, ARDUINO_REG_LENGTH, &device_config.acb.data_controls[ item ] );
							if( result != WICED_SUCCESS )
								print_status( "Error writing Arduino control: %u\r\n", item );
						} else {
							/*
							 * This is a sensor
							 */
							result = write_smart_arduino( SPI_CONTROL_0 + i, ARDUINO_REG_LENGTH, &device_config.acb.data_sensors[ i ] );
							if( result != WICED_SUCCESS )
								print_status( "Error writing Arduino sensor: %u\r\n", i );
						}
					}
					mask = mask << 2;
				}
				/*
				 * Clear value now all processed
				 */
				device_config.acb.master.status = 0;
			}
		} else {
			/*
			 * Try to re-setup device
			 */
			configure_arduino();
		}
	}
}
/**
  * @brief update the arduino control
  * @param  None
  * @retval : None
  */
uint16_t set_arduino(uint16_t arg, void *value )
{
	/*
	 * Update data structure - device will poll - add ability to generate Interrupt later
	 */
	memcpy( &device_config.acb.data_controls[ arg ].uint_32bit, value, SAMPLE_LENGTH );

	return( IMATRIX_SUCCESS );
}

uint16_t update_arduino_control(uint16_t arg, void *value )
{

	memcpy( value, &device_config.acb.data_controls[ arg ].uint_32bit, SAMPLE_LENGTH );
	return( IMATRIX_SUCCESS );
}
/**
  * @brief	Intitialize arduino sensor
  * @param  dummay arg
  * @retval : None
  */

void init_arduino_sensor( uint16_t arg )
{
	UNUSED_PARAMETER(arg);
	/*
	 * Nothing to do device is responsible for this
	 */

}
uint16_t sample_arduino_sensor(uint16_t arg, void *value )
{

	memcpy( value, &device_config.acb.data_sensors[ arg ].uint_32bit, SAMPLE_LENGTH );
	return( IMATRIX_SUCCESS );
}

void print_arduino( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	if( read_smart_arduino( SPI_CSR_SLAVE, ARDUINO_REG_LENGTH, &device_config.acb.slave.status ) == IMATRIX_SUCCESS ) {
		cli_print( "Arduino status: 0x%08x\r\n", device_config.acb.slave.status );
	} else
		cli_print( "Failed to get Arduino status\r\n", device_config.acb.slave.status );
}
