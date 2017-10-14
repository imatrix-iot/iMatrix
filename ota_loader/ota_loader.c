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

/** @file ota_loader.c
 *
 * Download an image from the site and URI provided. Load to the SFLASH.
 *
 * Example
 * coap://192.168.123.121:5683/control/otaupdate
 *
 * Playload
 *
 *{ "site" : "ota.sierratelecom.net", "uri" : "/images/c7cb244bbafea573d815a75e5dd2476c.elf", "image_no" : 6, "cksum" : 1600602452}
 *
 *  Created on: October, 2015
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "wiced.h"
#include "spi_flash.h"

#include "../system.h"
#include "../defines.h"
#include "../hal.h"
#include "../cli/interface.h"
#include "../cli/print_dct.h"
#include "../coap/coap.h"
#include "../coap/coap_transmit.h"
#include "../CoAP_interface/coap_def.h"
#include "../device/dcb_def.h"
#include "../device/config.h"
#include "../device/version.h"
#include "../hal_support.h"
#include "../imatrix/imatrix.h"
#include "../json/mjson.h"
#include "../networking/utility.h"
#include "../sflash/sflash.h"
#include "../wifi/wifi.h"
#include "ota_checksum.h"
#include "ota_loader.h"
#include "ota_structure.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
//#define	VERIFY_FLASH
/*
 * Where to store things in the flash
 */

#define SFLASH_SECTOR_SIZE	0x001000	// 4K
#define MICRON_SECTOR_LENGTH 0x10000	// 64K

#define FLASH_IMAGE_SIZE	0x400000	// 4M

#define CHECKSUM_LENGTH		13

char *latest_image[] =
{
		"/latest/sflash",
		"/latest/master",
		"/latest/slave",
		"/latestbeta/sflash",
		"/latestbeta/master",
		"/latestbeta/slave"
};

#define ELF_SIGNATURE_LENGTH	8
/******************************************************
 *                   Enumerations
 ******************************************************/
enum ota_loader_state_t {
	OTA_LOADER_IDLE,
	OTA_LOADER_START_AFTER_COAP_FLUSH,
	OTA_LOADER_INIT,
	OTA_LOADER_ERASE_FLASH,
	OTA_LOADER_VERIFY_ERASE,
	OTA_DNS_LOOKUP,
	OTA_LOADER_OPEN_SOCKET,
	OTA_LOADER_ESTABLISH_CONNECTION,
	OTA_LOADER_SEND_REQUEST,
	OTA_LOADER_SEND_PARTIAL_REQUEST,
	OTA_LOADER_PARSE_HEADER,
	OTA_LOADER_PARSE_PARTIAL_HEADER,
	OTA_LOADER_RECEIVE_STREAM,
	OTA_LOADER_ALL_RECEIVED,
	OTA_LOADER_VERIFY_OTA,
	OTA_CALC_CRC,
	OTA_LOADER_DATA_TIMEOUT,
	OTA_LOADER_CLOSE_CONNECTION,
	OTA_LOADER_CLOSE_SOCKET,
	OTA_LOADER_DONE,
	OTA_LOADER_SFLASH_COMPLETE,
	OTA_PRE_LOADER_IDLE
};

enum load_latest_t {
	GET_LATEST_DNS,
	GET_LATEST_OPEN_SOCKET,
	GET_LATEST_ESTABLISH_CONNECTION,
	GET_LATEST_SEND_REQUEST,
	GET_LATEST_PARSE_HEADER,
	GET_LATEST_RECEIVE_STREAM,
	GET_LATEST_DATA_TIMEOUT,
	GET_LATEST_CLOSE_CONNECTION,
	GET_LATEST_CLOSE_SOCKET,
	GET_LATEST_DONE,
	GET_LATEST_IDLE
};
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
extern dcb_t dcb;							// Defined in device/dcb_def.h and initialized in device.c
extern IOT_Device_Config_t device_config;	// Defined in device/config.h and saved in DCT

extern sflash_handle_t sflash_handle;
extern app_header_t apps_lut[ FULL_IMAGE ];
struct OTA_CONFIGURATION ota_loader_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	initialize the OTA loader state machine
  * @param  None
  * @retval : None
  *
  */
void init_ota_loader(void)
{
	ota_loader_config.last_ota_loader_state =
	ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
	dcb.ota_loader_active = false;
	dcb.get_latest_active = false;
}
/**
 * Return 1 (true) if OTA is doing anything, otherwise return 0 (false).
 *
 * written by Eric Thelin 18 August 2016
 */
uint16_t ota_is_active()
{
	return ( ota_loader_config.ota_loader_state != OTA_LOADER_IDLE );
}

/**
 * Copy temporary variables on the stack into persistent variables and start an Over The Air(OTA) update.
 * If an OTA update is already in progress, do nothing.
 */
void setup_ota_loader( char *site, char *uri, uint16_t image_no, uint16_t load_file, uint32_t checksum32 )
{
	if( ota_is_active() ) {
		print_status( "OTA loader already active\r\n" );
		return;	// Only one can be active at any point in time
	}
	if( image_no >= NO_IMAGE_TYPES ) {
		print_status( "OTA loader Invalid Image type: %u\r\n", image_no );
		return;	// Only one can be active at any point in time
	}

	print_status( "OTA loader initiated from: %s%s to image %u with checksum: 0x%08lx\r\n", site, uri, image_no, checksum32 );
	strcpy( ota_loader_config.site, site );
	strcpy( ota_loader_config.uri, uri );
	ota_loader_config.image_no = image_no;
	ota_loader_config.load_file = load_file;// Currently always TRUE, but maybe used in the future.
	ota_loader_config.checksum32 = checksum32;

	ota_loader_config.ota_loader_state = OTA_LOADER_START_AFTER_COAP_FLUSH;
	dcb.ota_loader_active = true;
}
/**
  * @brief	ota loader state machine
  * @param  None
  * @retval : None
  */
static 	uint8_t local_buffer[ BUFFER_LENGTH ];

uint16_t ota_loader(void)
{
	wiced_result_t result;
	wiced_utc_time_t utc_time;
	uint32_t buffer_length, i, j, content_length;
	uint16_t retry_count;
	char *content_start;
	char *foo;
	/*
	 * Check the stack - we just put a big block on it.
	 */
	if( ota_loader_config.last_ota_loader_state != ota_loader_config.ota_loader_state ) {
		print_status( "Changing OTA loader state to: %u\r\n", ota_loader_config.ota_loader_state );
		ota_loader_config.last_ota_loader_state = ota_loader_config.ota_loader_state;
	}
	/*
	 * Run State machine
	 */
	switch( ota_loader_config.ota_loader_state ) {
		case OTA_LOADER_START_AFTER_COAP_FLUSH :// Starting point for the OTA process.
			if( coap_transmit_empty() == true )	{	// Wait until response sent back before proceeding
				ota_loader_config.data_retry_count = 0;
				ota_loader_config.ota_loader_state = OTA_LOADER_INIT;
			}
			break;
		case OTA_LOADER_INIT :// Restart point when restarting from the beginning.
			ota_loader_config.socket_assigned = false;
		    ota_loader_config.good_load = false;
			ota_loader_config.checksum = 0;
			start_ota_checksum();
			ota_loader_config.flash_sector_size = get_sflash_sector_size();// This is really a constant.
			ota_loader_config.accept_ranges = false;// Until a response from the web server says that the server will accept requests for part of a file.

			ota_loader_config.content_received = 0;

			if ( ota_loader_config.image_no < FULL_IMAGE ) {//This is one of the standard WICED IMAGES
				get_apps_lut_if_needed();
				print_lut( 0 );
				ota_loader_config.content_offset = ( uint32_t) apps_lut[ ota_loader_config.image_no ].sectors[ 0 ].start * SFLASH_SECTOR_SIZE;
				ota_loader_config.erase_length = ( uint32_t) apps_lut[ ota_loader_config.image_no ].sectors[ 0 ].count * SFLASH_SECTOR_SIZE;
				ota_loader_config.allowed_sflash_area = WRITE_SFLASH_APP_AREA( ota_loader_config.image_no );
			}
			else if ( ota_loader_config.image_no == FULL_IMAGE ) {
				ota_loader_config.content_offset = 0;
				ota_loader_config.erase_length = FLASH_IMAGE_SIZE;
				ota_loader_config.allowed_sflash_area = WRITE_SFLASH_ANY_AREA;
			}
			else {
				/*
				 * Unknown content - close connection
				 */
				ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
			}

			print_status( "About to erase FLash @:0x%08lX, for %lu Bytes, sector size: %lu\r\n", ota_loader_config.content_offset, ota_loader_config.erase_length, ota_loader_config.flash_sector_size );
			ota_loader_config.erase_count = ota_loader_config.content_offset;
			ota_loader_config.ota_loader_state = OTA_LOADER_ERASE_FLASH;
			break;
		case OTA_LOADER_ERASE_FLASH :
			if( ota_loader_config.erase_count < ( ota_loader_config.content_offset + ota_loader_config.erase_length ) ) {
	            if( sflash_read( &sflash_handle, ota_loader_config.erase_count, (void*) local_buffer, BUFFER_LENGTH ) == WICED_SUCCESS ) {
	                /*
	                 * Print the buffer out
	                 */
	                print_status( "." );
	                blink_red_led();
	                fflush(stdout);
	                if ( 0 != protected_sflash_sector_erase( &sflash_handle, ota_loader_config.erase_count, ota_loader_config.allowed_sflash_area ) ) {
	                    print_status( "Failed to erase serial flash prior to writing new image @: 0x%08lx. Update canceled.\r\n", ota_loader_config.erase_count );
	                    ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
	                    return false;
	                }
	                ota_loader_config.erase_count += ota_loader_config.flash_sector_size; // MICRON_SECTOR_LENGTH; // SFLASH_SECTOR_SIZE;
	            } else
	                print_status( "Failed to read SFLASH\r\n");
			} else {
				ota_loader_config.erase_count = ota_loader_config.content_offset;
				print_status( "\r\nVerifying Erased Flash\r\n" );
				ota_loader_config.ota_loader_state = OTA_LOADER_VERIFY_ERASE;
			}
			break;
		case OTA_LOADER_VERIFY_ERASE :
#ifdef VERIFY_FLASH
			if( ota_loader_config.erase_count < ( ota_loader_config.content_offset + ota_loader_config.erase_length ) ) {
				print_status( "." );
				blink_red_led();
				fflush(stdout);
				if ( 0 != sflash_read( &sflash_handle, ota_loader_config.erase_count, (void*) &local_buffer, BUFFER_LENGTH ) ) {
					for( j = 0; ota_loader_config.flash_sector_size / BUFFER_LENGTH; j++ )
						for( i = 0; i < BUFFER_LENGTH; i++ )
							if( local_buffer[ i ] != 0xff ) {
								print_status( "SFLASH not erased at location: 0x%08lx\r\n", ota_loader_config.erase_count + i );
								ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
								return false;
							}
				}
				ota_loader_config.erase_count += ota_loader_config.flash_sector_size; // SFLASH_SECTOR_SIZE;
			} else {
				print_status( "\r\nFlash Erased Verified\r\n" );
				ota_loader_config.ota_loader_state = OTA_DNS_LOOKUP;
			}
#else
			print_status( "Flash Erased Verification skipped\r\n" );
			ota_loader_config.ota_loader_state = OTA_DNS_LOOKUP;
#endif
			break;
		case OTA_DNS_LOOKUP :// Restart after a partial download here if web server supports partial downloads.
			/*
			 * During OTA Dim lights to 100% and turn ON Green LED
			 */
			print_status( "Attempting to load OTA image: %s, from: %s\r\n", ota_loader_config.uri, ota_loader_config.site );
			print_status( "Doing DNS lookup...\r\n" );
			retry_count = 0;
			update_led_green_status( true );
			update_led_red_status( false );
			if( get_site_ip( ota_loader_config.site, &ota_loader_config.address ) ) {
			    ota_loader_config.ota_loader_state = OTA_LOADER_OPEN_SOCKET;
			    return false;
			} else {
			    print_status( "Failed DNS... count: %u\r\n", retry_count );
			}
			print_status( "Failed to get IP address, aborting OTA loader\r\n");
			ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
			return true;
			break;
		case OTA_LOADER_OPEN_SOCKET :
			result = wiced_tcp_create_socket( &ota_loader_config.socket, WICED_STA_INTERFACE );
			if( result != WICED_TCPIP_SUCCESS ) {
				print_status( "Failed to create socket on STA Interface, aborting\r\n" );
				ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
				return true;
			}
			ota_loader_config.ota_loader_state = OTA_LOADER_ESTABLISH_CONNECTION;
			break;
		case OTA_LOADER_ESTABLISH_CONNECTION :
			retry_count = 0;
			while( retry_count < MAX_CONNECTION_RETRY_COUNT ) {
				result = wiced_tcp_connect( &ota_loader_config.socket, &ota_loader_config.address, 80, 1000 );
				if( result == WICED_TCPIP_SUCCESS ) {
					print_status( "Successfully Connected to: %s on Port: 80\r\n", ota_loader_config.site );
					/*
					 * Create the stream
					 */
					result = wiced_tcp_stream_init( &ota_loader_config.tcp_stream, &ota_loader_config.socket );
					if( result == WICED_TCPIP_SUCCESS ) {
						ota_loader_config.socket_assigned = true;
						if ( ota_loader_config.content_received == 0 ) {
							ota_loader_config.ota_loader_state = OTA_LOADER_SEND_REQUEST;
						}
						else {
							ota_loader_config.ota_loader_state = OTA_LOADER_SEND_PARTIAL_REQUEST;
						}
						return false;
					} else {
						print_status( "Failed to connect to stream\r\n" );
					}
				}
			}
			print_status( "Failed to Connect to: %s on Port: 80, aborting OTA loader\r\n", ota_loader_config.site );
			ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
			break;
		case OTA_LOADER_SEND_REQUEST :		// Create query request and send
			memset( local_buffer, 0x00, BUFFER_LENGTH );
			strcat( (char *) local_buffer, "GET " );
			strcat( (char *) local_buffer, ota_loader_config.uri );
			strcat( (char *) local_buffer, " HTTP/1.1\nHost: " );
			strcat( (char *) local_buffer, ota_loader_config.site );
			strcat( (char *) local_buffer, "\r\n\r\n" );
		    print_status( "Sending query: %s\r\n", local_buffer );
		    result = wiced_tcp_stream_write( &ota_loader_config.tcp_stream, local_buffer , (uint32_t) strlen( (char *) local_buffer ) );
		    if ( result == WICED_TCPIP_SUCCESS ) {
		    	result =  wiced_tcp_stream_flush( &ota_loader_config.tcp_stream );
		    	if( result == WICED_TCPIP_SUCCESS ) {
		    		wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
		    		ota_loader_config.ota_loader_state = OTA_LOADER_PARSE_HEADER;
		    		return false;
		    	}
		    }
		    print_status( "FAILED to send request.\r\n" );
		    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
		    break;
		case OTA_LOADER_SEND_PARTIAL_REQUEST :		// Create query request and send
			memset( local_buffer, 0x00, BUFFER_LENGTH );
			strcat( (char *) local_buffer, "GET " );
			strcat( (char *) local_buffer, ota_loader_config.uri );
			strcat( (char *) local_buffer, " HTTP/1.1\nHost: " );
			strcat( (char *) local_buffer, ota_loader_config.site );
			sprintf( (char *) &local_buffer[ strlen( (char *) local_buffer ) ], "\nRange: bytes=%lu-%lu", ota_loader_config.content_received, ota_loader_config.total_content_length );
			strcat( (char *) local_buffer, "\r\n\r\n" );
		    print_status( "Sending partial request query: %s\r\n", local_buffer );
		    result = wiced_tcp_stream_write( &ota_loader_config.tcp_stream, local_buffer , (uint32_t) strlen( (char *) local_buffer ) );
		    if ( result == WICED_TCPIP_SUCCESS ) {
		    	result =  wiced_tcp_stream_flush( &ota_loader_config.tcp_stream );
		    	if( result == WICED_TCPIP_SUCCESS ) {
		    		wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
		    		ota_loader_config.ota_loader_state = OTA_LOADER_PARSE_PARTIAL_HEADER;
		    		return false;
		    	}
		    }
		    print_status( "FAILED to send request.\r\n" );
		    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
		    break;
		case OTA_LOADER_PARSE_HEADER :
			result = wiced_tcp_stream_read_with_count( &ota_loader_config.tcp_stream, local_buffer, BUFFER_LENGTH, 0, &buffer_length );
			if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
				print_status( "\r\nReceived: %lu Bytes\r\n", buffer_length );


				/*
				 * Look for 200 OK, Content length and two CR/LF - make sure we are getting what we asked for
				 */
				if( strstr( (char *) local_buffer, HTTP_RESPONSE_GOOD ) && strstr( (char *) local_buffer, CONTENT_LENGTH ) && strstr( (char *) local_buffer, CRLFCRLF ) ) {
					/*
					 * Header has all elements we need. Pull out the content length and save off the rest
					 */
					if( strstr( (char *) local_buffer, ACCEPT_RANGES ) )
						ota_loader_config.accept_ranges = true;
					else
						ota_loader_config.accept_ranges = false;
					ota_loader_config.content_length = strtol( strstr( (char *) local_buffer, CONTENT_LENGTH) + strlen( CONTENT_LENGTH ), &foo, 10 );

					/*
					 * Find the end of the header
					 */
					content_start = strstr( (char *) local_buffer, CRLFCRLF ) + strlen( CRLFCRLF );

					/*
					for( i = 0; i < ((uint8_t*)content_start - local_buffer); i++ )
						if( isprint( (uint16_t ) local_buffer[ i ] ) || ( local_buffer[ i ] == '\r' ) || ( local_buffer[ i ] == '\n' ) )
							print_status( "%c", local_buffer[ i ] );
						else
							print_status( "." );
					for( i = ((uint8_t*)content_start - local_buffer); i < buffer_length; i++ ) {
						print_status( "%x", local_buffer[ i ] );
						if ( (i % 20) == 19 ) {
							print_status( "\r\n");
						}
					}
					*/

					/*
					 * Calculate the size of the real content
					 */
					ota_loader_config.total_content_length = ota_loader_config.content_length;	// This is the full length of the body expected
					print_status( "\r\nTotal expected content Length: %lu\r\n", ota_loader_config.total_content_length );

					if( ota_loader_config.total_content_length > ota_loader_config.erase_length ) {
						print_status( "Content length(%lu) does not match expected erase_length(%lu).. Aborting\r\n",
											ota_loader_config.content_length, ota_loader_config.erase_length );
						ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
						return false;
					}
					/*
					 * Save the remaining part of this buffer as the first part of the image
					 */
					content_length = (uint32_t ) ( ( uint32_t ) &local_buffer[ 0 ] + buffer_length ) - ( uint32_t ) content_start;
					if( content_length > 0 ) {
						print_status( "Saving %lu Bytes to SFLASH\r\n", content_length );
						/*
						 * Write bytes to flash
						 */
						j = (uint32_t ) content_start - ( uint32_t ) &local_buffer[ 0 ];


						for( i = 0; i < content_length; i++ ) {
							ota_loader_config.checksum += local_buffer[ j++ ];
						}

						if( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length,
								ota_loader_config.allowed_sflash_area ) ) {
							print_status( "Write to serial flash failed!\r\n" );
							device_config.ota_fail_sflash_write += 1;
							save_config();
							ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
							return false;
						}
						calc_ota_checksum_partial( (uint8_t*)content_start, content_length );

						ota_loader_config.content_received = content_length;
						ota_loader_config.content_offset += content_length;
					}
					wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
					ota_loader_config.packet_count = 1;
					ota_loader_config.ota_loader_state = OTA_LOADER_RECEIVE_STREAM;
					return false;
				} else {
					print_status( "Garbage back - no header found: %80s\r\n", local_buffer );
					ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
					return false;
				}
			}else {
				wiced_time_get_utc_time( &utc_time );
				if( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) {
					ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
				}
			}
			break;
		case OTA_LOADER_PARSE_PARTIAL_HEADER :
			result = wiced_tcp_stream_read_with_count( &ota_loader_config.tcp_stream, local_buffer, BUFFER_LENGTH, 0, &buffer_length );
			if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
				print_status( "\r\nReceived: %lu Bytes\r\n", buffer_length );
				for( i = 0; i < buffer_length; i++ )
					if( isprint( (uint16_t ) local_buffer[ i ] ) || ( local_buffer[ i ] == '\r' ) || ( local_buffer[ i ] == '\n' ) )
						print_status( "%c", local_buffer[ i ] );
					else
						print_status( "." );
				 // Look for 206 OK, Content length and two CR/LF - make sure we are getting what we asked for
				if( strstr( (char *) local_buffer, HTTP_RESPONSE_PARTIAL ) && strstr( (char *) local_buffer, CONTENT_LENGTH ) && strstr( (char *) local_buffer, CRLFCRLF ) ) {
					// Header has all elements we need. Pull out the content length and save off the rest
					ota_loader_config.content_length = strtol( strstr( (char *) local_buffer, CONTENT_LENGTH) + strlen( CONTENT_LENGTH ), &foo, 10 );
					print_status( "Content Length: %lu\r\n", ota_loader_config.content_length );
					/*
					 * Find the end of the header
					 */
					content_start = strstr( (char *) local_buffer, CRLFCRLF ) + strlen( CRLFCRLF );
					/*
					 * Save the remaining part of this buffer as the first part of the image
					 */
					content_length = (uint32_t ) ( ( uint32_t ) &local_buffer[ 0 ] + buffer_length ) - ( uint32_t ) content_start;
					if( content_length > 0 ) {
						print_status( "Saving %lu Bytes to SFLASH\r\n", content_length );
						/*
						 * Write bytes to flash
						 */
						j = (uint32_t ) content_start - ( uint32_t ) &local_buffer[ 0 ];


						for( i = 0; i < content_length; i++ ) {
							ota_loader_config.checksum += local_buffer[ j++ ];
						}

						if( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length,
								ota_loader_config.allowed_sflash_area ) ) {
							print_status( "Write to serial flash failed!\r\n" );
							device_config.ota_fail_sflash_write += 1;
							save_config();
							ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
							return false;
						}
						calc_ota_checksum_partial( (uint8_t*)content_start, content_length );

						ota_loader_config.content_received += content_length;
						ota_loader_config.content_offset += content_length;
					}
					wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
					/*
					 * We may have got all the data needed in the block so check if we are done or if we have more to receive
					 */
					if( ota_loader_config.content_received == ota_loader_config.total_content_length ) {
						ota_loader_config.ota_loader_state = OTA_LOADER_ALL_RECEIVED;
						return false;
					}
					ota_loader_config.packet_count = 1;
					ota_loader_config.ota_loader_state = OTA_LOADER_RECEIVE_STREAM;
					return false;
				}
			} else {
				wiced_time_get_utc_time( &utc_time );
				if( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) {
					ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
				}
			}
			break;
		case OTA_LOADER_RECEIVE_STREAM :
			blink_red_led();
			result = wiced_tcp_stream_read_with_count( &ota_loader_config.tcp_stream, local_buffer, BUFFER_LENGTH, 1000, &buffer_length );
			if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
				ota_loader_config.packet_count += 1;
				for( i = 0; i < buffer_length; i++ ) {
					//print_status( "%c", ( uint8_t ) local_buffer[ i ] );
					ota_loader_config.checksum += local_buffer [ i ];
				}
				/*
				 * Check to make sure we don't get memory overwrite
				 */
				if( ota_loader_config.content_received + buffer_length > ota_loader_config.total_content_length  ) {
					/*
					 * Abort as we would over run the allocated space
					 */
					print_status( "Data length exceeded\r\n" );
					ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
					return false;
				}
				/*
				 * Write bytes to flash
				 */
				if ( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) local_buffer, buffer_length,
						ota_loader_config.allowed_sflash_area ) ) {
					print_status( "Write to serial flash failed.\r\n");
					device_config.ota_fail_sflash_write += 1;
					save_config();
					ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
					return false;
				}
				print_status( "(%u)Packet %u, @: 0x%08lX ", ota_loader_config.data_retry_count, ota_loader_config.packet_count, ota_loader_config.content_offset );
                fflush(stdout);
				calc_ota_checksum_partial( (uint8_t*)local_buffer, buffer_length );

				ota_loader_config.content_received += buffer_length;
				ota_loader_config.content_offset += buffer_length;
				print_status( "%lu Bytes Now %lu/%lu\r", buffer_length, ota_loader_config.content_received, ota_loader_config.total_content_length );
				/*
				 * Check if we have all the data yet
				 */
/*
				if ( ( ota_loader_config.packet_count >= 150 ) && ( ota_loader_config.data_retry_count == 0 ) ) {// Force timeout.
					if ( ota_loader_config.tcp_stream.rx_packet != NULL ) {
						wiced_packet_delete( ota_loader_config.tcp_stream.rx_packet );
						ota_loader_config.tcp_stream.rx_packet = NULL;
					}
					ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
					return false;
				}
*/
				if( ota_loader_config.content_received == ota_loader_config.total_content_length ) {
					ota_loader_config.ota_loader_state = OTA_LOADER_ALL_RECEIVED;
					return false;
				}
				wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );	// Set last time we got a packet
			}else {
				wiced_time_get_utc_time( &utc_time );
				if( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) {
					ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
				}
			}
			break;
		case OTA_LOADER_ALL_RECEIVED :
			print_status( "All Content received, simple checksum: 0x%08lx\r\n", ota_loader_config.checksum );
			print_status( "Received CRC checksum: 0x%08lx - ",  get_ota_checksum() );
			if ( ota_loader_config.checksum32 != IGNORE_CHECKSUM32 ) {
				print_status( "must match 0x%08lx\r\n", (long int)ota_loader_config.checksum32 );
			} else
				print_status( " - Checksum Ignore Request\r\n" );
			// Set content_offset back to the start of the downloaded file in flash.
			ota_loader_config.content_offset -= ota_loader_config.content_received;
			ota_loader_config.ota_loader_state = OTA_LOADER_VERIFY_OTA;
			return false;
			break;
		case OTA_LOADER_VERIFY_OTA :
			if ( ota_loader_config.checksum32 != IGNORE_CHECKSUM32 ) {

				// Calculate checksum for flash and check it.
				destroy_ota_checksum();
				start_ota_checksum();
				ota_loader_config.crc_content_offset = ota_loader_config.content_offset;// - ota_loader_config.content_received;
				ota_loader_config.crc_content_end = ota_loader_config.content_offset + ota_loader_config.content_received;
				print_status( "Calculating SFLASH Checksum\r\n" );
				ota_loader_config.ota_loader_state = OTA_CALC_CRC;
			} else {
				ota_loader_config.good_load = true;
				ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
			}
			break;
		case OTA_CALC_CRC :
			if( ota_loader_config.crc_content_offset < ota_loader_config.crc_content_end ) {
				blink_red_led();
				uint32_t bytes_remaining = ota_loader_config.crc_content_end -  ota_loader_config.crc_content_offset;
				uint16_t length = BUFFER_LENGTH;
				if ( bytes_remaining < BUFFER_LENGTH ) {
					length = bytes_remaining;
				}
				if( 0 != sflash_read( &sflash_handle, ota_loader_config.crc_content_offset, local_buffer, length ) ) {
				    print_status( "Error Reading back SFLASH for CRC\r\n" );
                    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                    return false;
				}
				for( i = 0; i < length; i++ )
				    if( local_buffer[ i ] != 0 )
				        print_status( "Non Zero Data detected\r\n" );
				calc_ota_checksum_partial( (uint8_t*)local_buffer, length );
				ota_loader_config.crc_content_offset += length;
			} else if ( ota_loader_config.checksum32 != get_ota_checksum() ) {
				print_status( "Checksum for saved file(0x%08lx) is invalid even though download was successful.\r\n", get_ota_checksum() );
				print_status( "Writing file to flash failed! Checksum should have been: 0x%08lx\r\n", ota_loader_config.checksum32 );
				device_config.ota_fail_sflash_crc += 1;
				save_config();
				ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
				return false;
			} else {
				print_status( "Checksum good\r\n" );
				ota_loader_config.good_load = true;
				ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
			}
			break;
		case OTA_LOADER_DATA_TIMEOUT :
			ota_loader_config.data_retry_count++;
			if( ota_loader_config.data_retry_count > MAX_DATA_RETRY_COUNT ) {
				print_status( "Max Retry Count exceeded. Giving up\r\n" );
				device_config.ota_fail_communications_link += 1;
				save_config();
				ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
				return false;
			}
		    wiced_tcp_disconnect( &ota_loader_config.socket );
		    wiced_tcp_delete_socket( &ota_loader_config.socket );
		    print_status( "Timeout Retry Count %u\r\n", ota_loader_config.data_retry_count );
		    if ( ota_loader_config.accept_ranges == true ) {
			    ota_loader_config.ota_loader_state = OTA_DNS_LOOKUP;
		    }
		    else {
			    ota_loader_config.ota_loader_state = OTA_LOADER_INIT;
		    }

			break;
		case OTA_LOADER_CLOSE_CONNECTION :
			ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
			break;
		case OTA_LOADER_CLOSE_SOCKET :
		    wiced_tcp_disconnect( &ota_loader_config.socket );
		    wiced_tcp_delete_socket( &ota_loader_config.socket );
		    ota_loader_config.socket_assigned = false;
		    if( ota_loader_config.good_load ) {
		    	print_status("Save is good.\r\n");
		    	ota_loader_config.ota_loader_state = OTA_LOADER_DONE;
		    }
		    else {
		    	print_status("Save failed.\r\n");
		    	ota_loader_config.ota_loader_state = OTA_PRE_LOADER_IDLE;
		    }
			break;
		case OTA_LOADER_DONE :
			if( ota_loader_config.load_file == true ) {
				if( ( ota_loader_config.image_no == APP0 ) || ( ota_loader_config.image_no == APP1 ) ) {
					reboot_to_image( ota_loader_config.image_no );
					/*
					 * Control should never return here - it only could if image fails signature test
					 *
					 */
					ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
					return( true );
				}
				else if( ( ota_loader_config.image_no == APP2 ) || ( ota_loader_config.image_no == WIFI_DATA ) ) {
				} else if( ota_loader_config.image_no == FULL_IMAGE ) {
					if( device_config.do_SFLASH_load == true ) {	// Got it successfully no need to do it next boot
						device_config.do_SFLASH_load = false;
						save_config();
						/*
						 * Go to idle mode with GREEN LED on
						 */
						update_led_red_status( false );
						update_led_green_status( true );
						print_status( "OTA loader completed loading SFLASH, in IDLE mode\r\n" );
						ota_loader_config.checksum32 = IGNORE_CHECKSUM32;
						ota_loader_config.ota_loader_state = OTA_LOADER_SFLASH_COMPLETE;
						return( false );
					}
				}
			}
			ota_loader_config.checksum32 = IGNORE_CHECKSUM32;
			ota_loader_config.ota_loader_state = OTA_PRE_LOADER_IDLE;
            break;
		case OTA_PRE_LOADER_IDLE :
			update_led_red_status( false );
			print_status( "OTA loader completed, returning to IDLE\r\n" );
			ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
			if ( ota_loader_config.image_no == FULL_IMAGE ) {// After returning to normal mode, upload details to imatrix.
				print_status( "SFLASH Loaded\r\n" );
				imatrix_production_upload( 1 );	// Loaded SFLASH
			}
			break;
		case OTA_LOADER_SFLASH_COMPLETE :
			update_led_green_status( true );	// Make sure it stays ON
			ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
			break;
		case OTA_LOADER_IDLE :
			dcb.ota_loader_active = false;
			return true;
			break;
		default :
			break;
	}
	return false;
}
/**
  * @brief	Terminate the OTA process - cleanly
  * @param  None
  * @retval : None
  */
void ota_loader_deinit(void)
{
	if( ( ota_is_active() ) && ( ota_loader_config.socket_assigned == true ) ) {
	    wiced_tcp_disconnect( &ota_loader_config.socket );
	    wiced_tcp_delete_socket( &ota_loader_config.socket );
	    ota_loader_config.socket_assigned = false;
		update_led_red_status( false );
		print_status( "OTA loader terminated, returning to IDLE\r\n" );
	}
	ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
}
/**
  * @brief	print the LUT
  * @param  None
  * @retval : None
  */

void print_lut(uint16_t arg)
{
    UNUSED_PARAMETER(arg);

	uint16_t i, j;

	get_apps_lut_if_needed();
	print_status( "Current LUT\r\n" );
	for( i = 0; i < 8; i++ ) {//NO_IMAGE_TYPES; i++ ) {
		cli_print( "Entry: %u, Sector Count: %u, Secure: %s\r\n", i, apps_lut[ i ].count,
				apps_lut[ i ].secure ? "True" : "False" );
		if( apps_lut[ i ].count != 0xFF )	// Real Data
			for( j = 0; j < apps_lut[ i ].count; j++  )
				cli_print( "Start Sector: %X Address: 0x%08lX, Length in sectors: %u - %lu Bytes ", apps_lut[ i ].sectors[ j ].start,
				    ( uint32_t ) ( apps_lut[ i ].sectors[ j ].start ) * SFLASH_SECTOR_SIZE,
					apps_lut[ i ].sectors[ j ].count,
					( uint32_t ) ( apps_lut[ i ].sectors[ j ].count ) * SFLASH_SECTOR_SIZE );
		cli_print("\r\n" );
	}

}

void reboot_to_image( uint16_t image_no )

{
	wiced_result_t result;

	if( image_no == FACTORY_RESET )
		result = protected_wiced_framework_set_boot( DCT_FR_APP_INDEX, 1 /* PLATFORM_DEFAULT_LOAD */ );
	else if( image_no == LEGACY_OTA )
		result = protected_wiced_framework_set_boot( DCT_OTA_APP_INDEX, 1 /* PLATFORM_DEFAULT_LOAD */ );
	else if( image_no == APP0 ) {
		result = protected_wiced_framework_set_boot( DCT_APP0_INDEX, 1 /* PLATFORM_DEFAULT_LOAD */ );
		print_status( "Rebooting to APP0: %u\r\n", DCT_APP0_INDEX);
	}
	else if( image_no == APP1 ) {
		result = protected_wiced_framework_set_boot( DCT_APP1_INDEX, 1 /* PLATFORM_DEFAULT_LOAD */ );
		print_status( "Rebooting to APP1: %u\r\n", DCT_APP1_INDEX);
	}
	else {
		print_status( "Invalid image number to boot from\r\n" );
		return;
	}
	if( result == WICED_SUCCESS ) {
		print_status( "Framework Set successfully\r\n" );
		print_dct_header();
        print_status( "Rebooting to APP...\r\n" );
	    // wiced_deinit();
        wiced_rtos_delay_milliseconds( 2000 );
        wiced_framework_reboot();
	} else
		print_status( "Failed to Set Boot - " );
    print_status( "Reboot failed!\r\n" );


}

void cli_get_latest( uint16_t arg )
{
    setup_get_latest_version( OTA_IMAGE_MASTER, device_config.ota_public_url );
}

void setup_get_latest_version(uint16_t image_type, char *site )
{
	dcb.get_latest_active = true;
	ota_loader_config.retry_count = 0;
	ota_loader_config.good_get_latest = false;
	ota_loader_config.ota_getlatest_state = GET_LATEST_DNS;
	ota_loader_config.image_type = image_type;
	memmove( ota_loader_config.site, site, SITE_LENGTH );

}
uint16_t get_latest_version(void)
{
	uint32_t buffer_length;
	wiced_result_t result;
	char local_buffer[ BUFFER_LENGTH ], version[ VERSION_LENGTH ], checksum[ CHECKSUM_LENGTH ], uri[ URI_LENGTH ], my_version[ VERSION_LENGTH ];
	char *content_start, *content_end;
    struct json_attr_t json_attrs[] = {
            {"image_url",  t_string, .addr.string = uri, .len = URI_LENGTH },
            {"version", t_string, .addr.string = version, .len = VERSION_LENGTH },
			{"checksum", t_string, .addr.string = checksum, .len = CHECKSUM_LENGTH },// initially tried using t_uinteger, but that is only a 31 bit integer.
            {NULL}
    };

	switch( ota_loader_config.ota_getlatest_state ) {
		case GET_LATEST_DNS :
            print_status( "DNS Lookup for site: %s\r\n", ota_loader_config.site );
            if( get_site_ip( ota_loader_config.site, &ota_loader_config.address ) == true ) {
                print_status("IP address %lu.%lu.%lu.%lu\r\n",
                        ( ota_loader_config.address.ip.v4 >> 24) & 0xFF,
                        ( ota_loader_config.address.ip.v4 >> 16) & 0xFF,
                        ( ota_loader_config.address.ip.v4 >> 8 ) & 0xFF,
                        ( ota_loader_config.address.ip.v4 & 0xFF) );
                    ota_loader_config.ota_getlatest_state = GET_LATEST_OPEN_SOCKET;
                    break;
            } else {
                print_status( "Failed DNS... \r\n" );
                print_status( "Failed to get IP address, aborting getting latest revision of: %u\r\n", ota_loader_config.image_type);
                ota_loader_config.ota_getlatest_state = GET_LATEST_IDLE;
                dcb.get_latest_active = false;
                return true;    // We are done
            }
			break;
		case GET_LATEST_OPEN_SOCKET :
			result = wiced_tcp_create_socket( &ota_loader_config.socket, WICED_STA_INTERFACE );
			if( result != WICED_TCPIP_SUCCESS ) {
				print_status( "Failed to create socket on STA Interface, aborting\r\n" );
				ota_loader_config.ota_getlatest_state = GET_LATEST_IDLE;
				dcb.get_latest_active = false;
				return true;	// We are done
			}
			ota_loader_config.ota_getlatest_state = GET_LATEST_ESTABLISH_CONNECTION;
			break;
		case GET_LATEST_ESTABLISH_CONNECTION :
			ota_loader_config.retry_count = 0;
			while( ota_loader_config.retry_count < MAX_CONNECTION_RETRY_COUNT ) {
				result = wiced_tcp_connect( &ota_loader_config.socket, &ota_loader_config.address, 80, 1000 );
				if( result == WICED_TCPIP_SUCCESS ) {
					print_status( "Successfully Connected to: %s on Port: 80\r\n", ota_loader_config.site );
					/*
					 * Create the stream
					 */
					result = wiced_tcp_stream_init( &ota_loader_config.tcp_stream, &ota_loader_config.socket );
					if( result == WICED_TCPIP_SUCCESS ) {
						ota_loader_config.ota_getlatest_state = GET_LATEST_SEND_REQUEST;
						break;
					} else {
						print_status( "Failed to connect to stream\r\n" );
					}
				}
			}
			if( ota_loader_config.retry_count >= MAX_CONNECTION_RETRY_COUNT ) {
				print_status( "Failed to Connect to: %s on Port: 80, aborting OTA loader\r\n", ota_loader_config.site );
				ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_SOCKET;
			}
			break;
		case GET_LATEST_SEND_REQUEST :		// Create query request and send
			memset( local_buffer, 0x00, BUFFER_LENGTH );
			strcat( local_buffer, "GET " );
			if( ota_loader_config.image_type < OTA_IMAGE_NO_IMAGES ) {
				strcat( local_buffer, latest_image[ ota_loader_config.image_type ] );
			} else {
				ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_SOCKET;
				break;
			}
			strcat( local_buffer, " HTTP/1.1\nHost: " );
			strcat( local_buffer, ota_loader_config.site );
			strcat( local_buffer, "\r\n\r\n" );
		    print_status( "Sending query: %s\r\n", local_buffer );
		    result = wiced_tcp_stream_write( &ota_loader_config.tcp_stream, local_buffer , (uint32_t) strlen( local_buffer ) );
		    if ( result == WICED_TCPIP_SUCCESS ) {
		    	result =  wiced_tcp_stream_flush( &ota_loader_config.tcp_stream );
		    	if( result == WICED_TCPIP_SUCCESS ) {
		    		wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
		    		ota_loader_config.retry_count = 0;
		    		ota_loader_config.ota_getlatest_state = GET_LATEST_PARSE_HEADER;
		    		break;
		    	}
		    }
		    if( result != WICED_TCPIP_SUCCESS ) {
		    	print_status( "FAILED to send request.\r\n" );
		    	ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_SOCKET;
		    }
		    break;
		case GET_LATEST_PARSE_HEADER :
			result = wiced_tcp_stream_read_with_count( &ota_loader_config.tcp_stream, local_buffer, BUFFER_LENGTH, 10000, &buffer_length );
			if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
				print_status( "\r\nReceived: %lu Bytes\r\n", buffer_length );

				/*
				 *
			    uint32_t i;
				for( i = 0; i < buffer_length; i++ )
					if( isprint( (uint16_t ) local_buffer[ i ] ) || ( local_buffer[ i ] == '\r' ) || ( local_buffer[ i ] == '\n' ) )
						print_status( "%c", local_buffer[ i ] );
					else
						print_status( "." );

				*/

				/*
				 * Look for 200 OK, Content length and two CR/LF - make sure we are getting what we asked for
				 */
				if( strstr( local_buffer, HTTP_RESPONSE_GOOD ) && strstr( local_buffer, CRLFCRLF ) ) {
					/*
					 * Header has all elements we need. Pull out the content length and save off the rest
					 */
					if( strstr( local_buffer, ACCEPT_RANGES ) )
						ota_loader_config.accept_ranges = true;
					else
						ota_loader_config.accept_ranges = false;
					/*
					 * Find the end of the header
					 */
					content_start = strstr( local_buffer, "{" );
					content_end = strstr( local_buffer, "}" ) + 1;
					if( content_start == (char *) 0 || content_end == (char *) 1 ) {
						print_status( "JSON not found in body\r\n" );
						ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_CONNECTION;
					} else {
						*content_end = 0x00;	// Null terminate the string

					print_status( "JSON: %s\r\n", content_start );
				    /*
				     * Process the passed URI Query
				     */
				    result = json_read_object( content_start, json_attrs, NULL );

				    if( result ) {
				        print_status( "JSON parsing failed, Result: %u\r\n", result );
				        return( false );
				    }

				    if(strcmp( uri, "" ) == 0 ) {
				        print_status( "Missing uri value in JSON from request.\r\n");
				        return( false );
				    }
				    memmove( ota_loader_config.uri, uri, URI_LENGTH );

				    if(strcmp( version, "" ) == 0 ) {
				        print_status( "Missing Version value in JSON from request.\r\n");
				        return( false );
				    }
				    memmove( ota_loader_config.version, version, VERSION_LENGTH );

				    if(strcmp( checksum, "" ) == 0 ) {
				        print_status( "Missing checksum value in JSON from request.\r\n");
				        return( false );
				    }
				    ota_loader_config.checksum32 = atol( checksum );

				    /*
				     * Got here with maybe GOOD data, return TRUE :)
				     *
				     */
				    ota_loader_config.good_get_latest = true;
					}
				}
			}
			ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_CONNECTION;
			break;
		case GET_LATEST_CLOSE_CONNECTION :
			ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_SOCKET;
			break;
		case GET_LATEST_CLOSE_SOCKET :
	    	dcb.get_latest_active = false;
		    wiced_tcp_disconnect( &ota_loader_config.socket );
		    wiced_tcp_delete_socket( &ota_loader_config.socket );
		    if( ota_loader_config.good_get_latest ) {
		    	if( ota_loader_config.image_type == OTA_IMAGE_MASTER ) {	// Only check version for MASTER flash
			    	sprintf( my_version, VERSION_FORMAT, MajorVersion, MinorVersion, AUTO_BUILD );
			    	if( strcmp( my_version, ota_loader_config.version ) >= 0 ) {
			    		print_status( "We have the latest Version: %s, No need to upgrade to: %s\r\n", my_version, ota_loader_config.version );
			    		return( true );
			    	}
		    	}
		    	print_status("Got the uri and checksum.\r\n");
		    	if( ( ota_loader_config.image_type == OTA_IMAGE_SFLASH ) ||
		    		( ota_loader_config.image_type == OTA_IMAGE_BETA_SFLASH ) )
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, FULL_IMAGE, true, (uint32_t)ota_loader_config.checksum32 );
		    	else if( ( ota_loader_config.image_type == OTA_IMAGE_MASTER ) ||
		    			 ( ota_loader_config.image_type == OTA_IMAGE_BETA_MASTER ) )
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, APP0, true, (uint32_t)ota_loader_config.checksum32 );
		    	else
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, APP2, true, (uint32_t)ota_loader_config.checksum32 );
		    }
		    else {
		    	print_status("Get Latest failed.\r\n");
		    }
	    	dcb.get_latest_active = false;
    		return( true );
			break;
		case GET_LATEST_IDLE :
			return true;
			break;
	}
	return( false );
}

/**
  * @brief	Verify that the target is a vaild ELF image
  * @param  image_no, boot mode
  * @retval : result
  */

wiced_result_t protected_wiced_framework_set_boot( uint16_t image_no, uint16_t load_mode )
{
	uint8_t buffer[ ELF_SIGNATURE_LENGTH ];
	uint8_t valid_elf[ ELF_SIGNATURE_LENGTH ] = { 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 00 };
	wiced_result_t result;

	get_apps_lut_if_needed();
    result = sflash_read( &sflash_handle, apps_lut[ image_no ].sectors[ 0 ].start * SFLASH_SECTOR_SIZE , (void*) buffer, ELF_SIGNATURE_LENGTH );
    if( result != WICED_SUCCESS )
    	return result;
    /*
     * Verify this image
     *
     * Start of ELF image should be: 7F 45 4C 46 01 01 01 00
     */
    if( memcmp( buffer, valid_elf, ELF_SIGNATURE_LENGTH ) == 0 ) {
    	print_status( "Elf signature Verified\r\n" );
    	return( wiced_framework_set_boot( image_no, load_mode ) );
    } else {
    	print_status( "Selected Image: %u does not have a valid ELF signature\r\n", image_no );
    	return WICED_ERROR;
    }

}
