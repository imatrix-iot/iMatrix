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

/** @file sflash.c
 *
 *  Created on: September 9, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include "wiced.h"
#include "imatrix.h"


#include "../ota_loader/ota_structure.h"
#include "spi_flash.h"
#include "wiced_apps_common.h"
#include "spi_flash.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"
//#include "../fixture/certificates.h"
#include "../common.h"

#include "sflash.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_SFLASH
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_SFLASH ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

#define IPV4_TO_4_BYTES( addr ) ( ( (addr.ip.v4) >> 24 ) & 0xFF ), \
                                ( ( (addr.ip.v4) >> 16 ) & 0xFF ), \
						        ( ( (addr.ip.v4) >> 8 ) & 0xFF ), \
						        ( (addr.ip.v4) & 0xFF )

/******************************************************
 *                    Constants
 ******************************************************/

#define SFLASH_TEST_SIZE	0x400000
#define OTA_PROGRAM_START   0x00000
#define FLASH_SIZE			4194304
#define CONFIG_START_OLD	0x3FFC00

#define SFLASH_ID_M25P32    ( (uint32_t) 0x202016 )  // Added by Sierra Telecom, Inc.

#define LAST_DEVICE_ADDRESS   (0x3FFFFF)
#define SECTOR_SIZE_64K (0x10000)
#define SFLASH_4K_SECTOR_SIZE ( 0x1000 )


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
static uint16_t all_areas( uint32_t device_address, uint32_t size );

/******************************************************
 *               Variable Definitions
 ******************************************************/
sflash_handle_t sflash_handle;
extern app_header_t apps_lut[ 8 ];// Look Up Table descriptions of the 8 entries that may be used as bootable images by the bootloader.
extern imx_imatrix_init_config_t imatrix_config;
extern IOT_Device_Config_t device_config;   // Defined in storage.h
/******************************************************
 *               Function Definitions
 ******************************************************/
uint32_t get_sflash_sector_size(void)
{
	if( sflash_handle.device_id == SFLASH_ID_M25P32 )
		return 0x10000;
	else
		return 0x1000;
}

/**
  * @brief  init_serial_flash
  * @param  None
  * @retval : None
  */
uint16_t init_serial_flash(void)
{
    int status;
    uint32_t size;
    /*
     * Initialize the interface
     */
    status = init_sflash( &sflash_handle, 0, SFLASH_WRITE_ALLOWED );
    if( status != WICED_SUCCESS ) {
        printf( "Failed to initialized Serial Flash: %d\r\n", status );
        return false;
    }
    sflash_get_size( &sflash_handle, &size );
    PRINTF( "Serial Flash successfully initialized, flash size is: %lu\r\n", size );
    if( size != device_config.sflash_size ) {
        printf( "Flash Size: %lu, Expected: %lu\r\n", size, device_config.sflash_size );
    	return false;
    } else
    	return true;
}

/**
  * @brief  read_serial_flash_status
  * @param  None
  * @retval : status
  */
int sflash_read_status_register( sflash_handle_t* const handle, unsigned char* dest_addr );

int read_serial_flash_status(void)
{
	return sflash_read_status_register( &sflash_handle, 0 );
}

void reboot_if( uint16_t failed, char* msg )
{
	if ( failed ) {
		printf( msg );
		wiced_rtos_delay_milliseconds( 10000 );
		wiced_framework_reboot();
	}
}

/**
 * Erase 64 KB worth of sectors returning WICED_SUCCESS or
 * WICED_ERROR if sflash_sector_erase returned an error code.
 *
 * written by Eric Thelin 1 June 2016
 */
wiced_result_t erase_64KB_sector( uint32_t sector_start_address, uint16_t allowed_area )
{
	uint16_t i;
	int result = 0;

	if( sflash_handle.device_id == SFLASH_ID_M25P32 ) {// This device already has 64 KB sectors.
	    result = protected_sflash_sector_erase( &sflash_handle, sector_start_address, allowed_area );
		if ( result != 0 ) goto fail;
	}
	else {// This device has 4 KB sectors.
		for ( i = 0; i < 16; i++ ) {
			result = protected_sflash_sector_erase( &sflash_handle, sector_start_address, allowed_area );
			if ( result != 0 ) goto fail;

			sector_start_address += 4096;
		}
	}
    return WICED_SUCCESS;

fail:
    printf("erase_64KB_sector returned Error Code: %d. Aborting Erase!\r\n", result );
    return WICED_ERROR;
}

/**
 * Load the Look Up Table(LUT) from the first part of the serial flash.
 *
 * written by Eric Thelin 17 August 2016
 */
void get_apps_lut_if_needed(void)
{
/*
    uint8_t valid_lut[ sizeof( apps_lut ) ] = {
            0x01, 0x00, 0x10, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x00, 0xF0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xE0, 0x01, 0x20, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0xE0, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xE0, 0x02,
            0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
            0xC0, 0x03, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
*/
    static uint16_t apps_lut_requires_initialization = true;

    if ( apps_lut_requires_initialization ) {

        sflash_read( &sflash_handle, 0, apps_lut, sizeof( apps_lut ) );
        apps_lut_requires_initialization = false;

        //  Verify LUT against valid imatrix_config.lut

        if( memcmp( &apps_lut, imatrix_config.lut, sizeof( apps_lut ) ) != 0 ) {
            imx_printf( "LUT invalid, loading with default values\r\n" );
            memcpy( &apps_lut, imatrix_config.lut, sizeof( apps_lut ) );
            sflash_sector_erase( &sflash_handle, 0 );
            sflash_write( &sflash_handle, 0, (void*) &apps_lut, sizeof( apps_lut ) );
        }
    }
}

/**
 * Return a value containing bit flags for every writable serial flash area
 * starting at "device_address" through the end of size bytes.
 * The bit flags are specified by the WRITE_SFLASH_APP_AREA, WRITE_SFLASH_LUT_AREA and WRITE_SFLASH_CONFIG_AREA macros and
 * if the entire block from "device_address" through "size" lies outside any areas specified by those macros
 * then WRITE_SFLASH_UNPARTITIONED_SPACE is returned.
 *
 * written by Eric Thelin 17 August 2016
 */
static uint16_t all_areas( uint32_t device_address, uint32_t size )
{
	uint16_t i;
	uint16_t writing_to_areas = 0;
	uint32_t app_addr, next_app_addr;

	if ( device_address + size - 1 > LAST_DEVICE_ADDRESS ) {
		PRINTF("Address past end of flash in all areas function, start: 0x%lx, end: 0x%lx\r\n", device_address, device_address + size - 1 );
		return 0;// Not allowed.
	}

	// Identify to which app areas the content would be written.

	get_apps_lut_if_needed();

	if ( device_address < SECTOR_SIZE_64K ) {
		writing_to_areas |= WRITE_SFLASH_LUT_AREA;// first 64K sector
	}

	for ( i = 0; i < FULL_IMAGE; i++ ) {
		if ( apps_lut[ i ].count > 1 ) {
			PRINTF( "Error: fragmented file in LUT.\r\n" );
		}
		else if ( apps_lut[ i ].count == 1 ) {
			app_addr = apps_lut[ i ].sectors[0].start * SFLASH_4K_SECTOR_SIZE;
			next_app_addr = app_addr + ( apps_lut[ i ].sectors[0].count * SFLASH_4K_SECTOR_SIZE );

			// If writing to this app, set the bit flag.

			if ( ( device_address < next_app_addr ) && ( device_address + size > app_addr ) ) {
				writing_to_areas |= WRITE_SFLASH_APP_AREA( i );
			}
		}
		// else ignore any apps entries that have no data saved (count == 0).
	}
	if ( ( device_address + size ) > ( LAST_DEVICE_ADDRESS - 2 * SECTOR_SIZE_64K ) ) {
		writing_to_areas |= WRITE_SFLASH_CONFIG_AREA;
	}
    if ( writing_to_areas == 0 ) {//Entire block is being written to an unpartitioned area on the flash.
    	writing_to_areas = WRITE_SFLASH_UNPARTITIONED_SPACE;
    }
	return writing_to_areas;
}
/**
 * Write "size" bytes from the data block to serial flash starting at "device_address".
 * If attempting to write to any partition other than "allowed_areas"
 * return an error (-1) without writing anything to serial flash.
 * Otherwise return the value returned by sflash_write ( 0 for success, -1 for failure ).
 * If the entire block being written is in unpartitioned space,
 * unless "allow_areas" includes the WRITE_SFLASH_UNPARTITIONED_SPACE bit
 * return an error (-1).
 *
 * written by Eric Thelin 17 August 2016
 */
int protected_sflash_write(const sflash_handle_t* const handle, unsigned long device_address, /*@observer@*/ const void* const data_addr, unsigned int size, uint16_t allowed_areas  )
{
	uint16_t areas_to_write = 0;

	// Reject any attempt to write beyond the last address in flash memory.

	if ( device_address + size - 1 > LAST_DEVICE_ADDRESS ) {
		PRINTF( "Writing past end of flash, start: 0x%lx, end: 0x%lx\r\n", device_address, device_address + size - 1 );
		return -1;
	}

	// If attempting to write to an area other than an allowed area return an error.

    areas_to_write = all_areas( device_address, size );
	if ( ( areas_to_write == 0 ) || ( ( ( ~ allowed_areas ) & areas_to_write ) != 0 ) ) {
		PRINTF( "Writing outside of permitted area( 0x%x ), start: 0x%lx, end: 0x%lx, resultant area: 0x%x\r\n", allowed_areas, device_address, device_address + size - 1, areas_to_write );
		return -1;
	}

	return sflash_write( handle, device_address, data_addr, size );
}

/**
 * Erase the sector starting at "device_address".
 * If attempting to write outside the "allowed_area" return an error (-1) without erasing anything.
 * Otherwise return the value returned by sflash_sector_erase ( 0 for success, -1 for failure ).
 * Either 4KB or 64KB sector sizes will work.
 * If any "device_address" is not on a sector boundary return error (-1) without erasing anything.
 * If the "allowed_area" is WRITE_SFLASH_ANY_AREA then erasing any sector in flash is allowed.
 *
 * written by Eric Thelin 17 August 2016
 */
int protected_sflash_sector_erase( const sflash_handle_t* const handle, unsigned long device_address, uint16_t allowed_area )
{
 	// Reject any attempt to erase beyond the last address in flash memory or erase from anywhere other than a 4KB boundary.

	if ( ( device_address > LAST_DEVICE_ADDRESS ) ) {
		PRINTF( "Sector Erase starts past end of flash: 0x%lx\r\n", device_address );
		return -1;
	}
	if ( ( ( device_address % get_sflash_sector_size() ) != 0) ) {
		PRINTF( "Sector erase (%lu) off of sector boundary\r\n", device_address );
		return -1;
	}

	// If attempting to write to an area other than an allowed area return an error.

	if ( ( allowed_area != WRITE_SFLASH_ANY_AREA) &&
			( allowed_area != all_areas( device_address, get_sflash_sector_size() ) ) ) {
        PRINTF( "Error, sector erase allowed area: %x should be: %x\r\n", all_areas( device_address, get_sflash_sector_size() ), allowed_area );
		return -1;
	}

	return sflash_sector_erase( handle, device_address );
}

void test_sflash( wiced_system_monitor_t* watchdog, uint32_t watchdog_delay )
{
	uint8_t test_buffer[ 1024 ], read_buffer[ 1024 ];
	int result;
	uint32_t i, j, k, l, sector_size;

	imx_printf( "Commencing Serial Flash Write test\r\n" );
	for( i = 0; i < 256; i++ ) {
		test_buffer[ i ] = (uint8_t) ( i + 0 ) & 0xff;
		test_buffer[ i + 256 ] = (uint8_t) ( i + 1 ) & 0xff;
		test_buffer[ i + 512 ] = (uint8_t) ( i + 2 ) & 0xff;
		test_buffer[ i + 768 ] = (uint8_t) ( i + 3 ) & 0xff;
	}
	sector_size = get_sflash_sector_size();
	wiced_update_system_monitor( watchdog, watchdog_delay );
	imx_printf( "Erasing Flash: ");
	for( i = 0; i < 0x400000; i += sector_size ) {
		wiced_update_system_monitor( watchdog, watchdog_delay );
		sflash_sector_erase( &sflash_handle,  i );
		imx_printf( "." );
	}
	imx_printf( "\n" );
	/*
	 * Program with data
	 */
	imx_printf( "Programming Block: " );
	for( i = 0; i < 0x400000; i += 0x400 ) {
		wiced_update_system_monitor( watchdog, watchdog_delay );
		imx_printf( "." );
		result = sflash_write( &sflash_handle, i, (void*) &test_buffer, 0x400 );
		if( result != 0 ) {
			imx_printf( "Error Writing Flash - Aborting: %d\r\n", result );
			return;
		}
	}
	imx_printf( "\n" );
	/*
	 * Read it back to verify write
	 */
	imx_printf( "Commencing Serial Flash Verification test\r\n" );
	imx_printf( "Verifying Block: " );
	for( i = 0; i < 0x400000; i += 0x400 ) {
		wiced_update_system_monitor( watchdog, watchdog_delay );
		imx_printf( "." );
		result = sflash_read( &sflash_handle, i, (void*) &read_buffer, 0x400 );
	    if( result != 0 ) {
	    	imx_printf( "Error Reading Flash - Aborting: %d\r\n", result );
	    	return;
	    }
	    for( j = 0; j < 0x400; j++ ) {
	    	if( test_buffer[ j ] != read_buffer[ j ] ) {
	    		imx_printf( "Bad data found in block: 0x%0lx - offset: 0x%l08x\r\n", i, j  );
	    		/*
	    		 *
	    		 * print out the two for comparison - 10 lines
	    		 */
				for( k = 0; k < ( j + 1023 ); k += 32 ) {
					/*
					 * Print 32 Bytes per line
					 */
					imx_printf( "Written   - 0x%08lX  ", i + k );
					for( l = 0; ( k  < 1024 ) && ( l < 32 ); l++ ) {
						imx_printf( "%02X ", (uint16_t) test_buffer[ k + l ] );
						if( ( ( l + 1 ) % 8 == 0 ) && ( l != 0 ) )	// Make it easier to read
							imx_printf( " " );
					}
					for( l = 0; ( k  < 1023 ) && ( l < 32 ); l++ ) {
						if( isprint( (uint16_t) test_buffer[ k + l ] ) && ( (uint16_t) test_buffer[ k + l ] != 0x0a ) && ((uint16_t) test_buffer[ k + l ] != 0x0d ) )
							imx_printf( "%c", test_buffer[ k + l ] );
						else
							imx_printf( "." );
					}
					imx_printf( "\n" );
					imx_printf( "Read Back - 0x%08lX  ", i + k );
					for( l = 0; ( k  < 1024 ) && ( l < 32 ); l++ ) {
						imx_printf( "%02X ", (uint16_t) read_buffer[ k + l ] );
						if( ( ( l + 1 ) % 8 == 0 ) && ( l != 0 ) )	// Make it easier to read
							imx_printf( " " );
					}
					for( l = 0; ( k  < 1024 ) && ( l < 32 ); l++ ) {
						if( isprint( (uint16_t) read_buffer[ k + l ] ) && ( (uint16_t) read_buffer[ k + l ] != 0x0a ) && ((uint16_t) read_buffer[ k + l ] != 0x0d ) )
							imx_printf( "%c", read_buffer[ k + l ] );
						else
							imx_printf( "." );
					}
					imx_printf( "\n" );
				}
			    return;
	    	}
	    }
	}
	imx_printf( "\nSerial Flash Verification PASSED\n" );
}
