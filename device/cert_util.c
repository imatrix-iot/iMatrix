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
/** @file cert_util.c
 *
 *
 *  Created on: December, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "wiced.h"
#include "base64.h"

#include "../storage.h"
#include "../device/icb_def.h"
#include "../cli/cli.h"
#include "cli/cli_dump.h"
#include "../cli/interface.h"

#include "cert_util.h"
#include "cert_defs.h"
    /*
     * Certificates used for 802.1X authentication
     */
    uint8_t ca_root[ MAX_CERT_LENGTH ];
    uint8_t wifi_cert[ MAX_CERT_LENGTH ];
    uint8_t wifi_key[ MAX_CERT_LENGTH ];
    uint8_t dtls_cert[ MAX_CERT_LENGTH ];

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_CERTIFICATE_LENGTH	1800
#define CERT_RECORD_LENGTH		48
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
#include "certificates.h"

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	Verify that the string passed in is the right format
  * @param  certificate
  * @retval : True if success False if fail
  */
uint16_t verify_certificate( uint8_t *certificate, uint16_t type )
{
	// imx_printf( "Verifying certificate length: %u, Certificate:\r\n%s", strlen( (char *) certificate ), (char * ) certificate );
	if( ( strlen( (char *) certificate ) < MAX_CERTIFICATE_LENGTH ) ) {
		// imx_printf( "Looking for ->%s<-, in ->%s<-/r/n", CERT_BEGIN, ( char *) certificate );
		if( type == IS_CERTIFICATE ) {
			if( strstr( (char *) certificate, CERT_BEGIN ) != 0x00 ) {	// Found
				if( strstr( (char *) certificate, CERT_END ) != 0x00 ) {	// Found
					return true;
				} else
					imx_printf( "Could not fined END of Certificate\r\n" );
			} else
				imx_printf( "Could not fined BEGIN of Certificate\r\n" );
		} else if( type == IS_KEY ) {
			if( strstr( (char *) certificate, KEY_BEGIN ) != 0x00 ) {	// Found
				if( strstr( (char *) certificate, KEY_END ) != 0x00 ) {	// Found
					return true;
				} else
					imx_printf( "Could not fined END of Certificate\r\n" );
			} else
				imx_printf( "Could not fined BEGIN of Certificate\r\n" );
		} else
			imx_printf( "Unknown type\r\n" );
	} else
		imx_printf( "Certificate is too long\r\n" );
	return false;
}
/**
  * @brief Convert a base64 block to binary
  * @param  Certificate and length
  * @retval : Length of binary block
  */

uint32_t convert_certificate_to_bin( uint8_t *buffer, uint32_t buffer_length, uint8_t *certificate, uint32_t certificate_length, uint16_t type )
{
	char *ptr;
	int result;
	uint16_t done;
	uint32_t buffer_index, cert_index, process_length;
	/*
	 * Convert the certificate to a binary block, the base 64 strings are stored in blocks with a terminating CR LF,
	 */
	buffer_index = 0;
	cert_index = 0;
	result = 0;
	done = false;
	do {
		/*
		 * Check to see if we are at the end
		 */
		if( type == IS_CERTIFICATE ) {
			if( strncmp( (char *) &certificate[ cert_index], CERT_END, strlen( CERT_END ) ) == 0x00 ) {	// Found
				done = true;
			}
		} else if( type == IS_KEY ) {
			if( strncmp( (char *) &certificate[ cert_index], KEY_END, strlen( KEY_END ) ) == 0x00 ) {	// Found
				done = true;
			}
		} else {
			imx_printf( "Unknown type\r\n" );
			done = true;
		}
		/*
		 * Process if we have more data
		 */
		if( !done ) {
			ptr = strstr( (char *) &certificate[ cert_index ], CRLF );
			if( ptr == NULL ) {
				imx_printf( "End of line not found\r\n" );
				done = true;
			} else {
				process_length = (uint32_t) ptr - (uint32_t) &certificate[ cert_index ];
				// imx_printf( "Processing: %lu characters\r\n", process_length );
				result = base64_decode( ( unsigned char *) &certificate[ cert_index], ( int32_t ) process_length,
										( unsigned char *) &buffer[ buffer_index ], ( (uint32_t) buffer_length ) - buffer_index , BASE64_STANDARD );
				if( result == -1 ) {	// Something failed
					imx_printf( "base64 decode failed\r\n" );
					return 0;
				} else {
					buffer_index += result;
				}
				cert_index += process_length + 2; // Move past this data and the CR/LF
			}
		}
	} while( !done );

	return buffer_index;
}
/**
  * @brief Convert a block of binary to a certificate
  * @param  binary block, length and return Certificate and length
  * @retval : Length of certificate
  */

uint32_t convert_bin_to_certificate( uint8_t *buffer, uint32_t buffer_length, uint8_t *certificate, uint32_t certificate_length, uint16_t type )
{
	uint32_t buffer_index, cert_index, process_count, encode_count;
	uint16_t done;

	done = false;
	buffer_index = 0;

	if( certificate_length < MAX_CERTIFICATE_LENGTH )
		return 0;	// Do not process
	/*
	 * Write out what type of cert this is
	 */
	if( type == IS_CERTIFICATE )
		cert_index = sprintf( (char *) certificate, "%s\r\n", CERT_BEGIN );
	else if( type == IS_KEY )
		cert_index = sprintf( (char *) certificate, "%s\r\n", KEY_BEGIN );
	else
		return 0;	// Do not process
	do {
		if( buffer_length - buffer_index < CERT_RECORD_LENGTH  )
			process_count = buffer_length - buffer_index;
		else
			process_count = CERT_RECORD_LENGTH;
		if( process_count > 0 ) {
			encode_count = base64_encode( (unsigned char*) &buffer[ buffer_index ], process_count, (unsigned char *) &certificate[ cert_index ], certificate_length - cert_index, BASE64_STANDARD );
			if( encode_count == -1 ) {
				cli_print( "Encoding failed\r\n" );
				return 0;	// Fail for some reason
			}
			cert_index += encode_count;
			cert_index += sprintf( (char *) &certificate[ cert_index ], "\r\n" );
			buffer_index += process_count;
		} else
			done = true;
	} while( !done );

	/*
	 * Write out END of cert, first check if there is room
	 */

	if( type == IS_CERTIFICATE )
		cert_index += sprintf( (char *) &certificate[ cert_index ], "%s\r\n%c", CERT_END, 0x00 );
	else if( type == IS_KEY )
		cert_index += sprintf( (char *) &certificate[ cert_index ], "%s\r\n%c", KEY_END, 0x00 );

	return cert_index;
}

void cert_test(void)
{
    uint8_t buffer[ MAX_CERT_LENGTH ], certificate[ MAX_CERT_LENGTH ];
    uint16_t cert_length, done;
    uint32_t buffer_index, cert_index, process_count;
    int encode_count;

    update_cert_pointers();

	cli_print( "Testing with CA Root certificate - Verifying Certificate..." );
	if( verify_certificate( icb.root_certificate, IS_CERTIFICATE ) ) {
		cli_print( "Verified\r\n" );
		cli_print( "Encoding to binary..." );
    	/*
    	 * Convert Certificate to Binary -
    	 */
		cert_length = convert_certificate_to_bin( buffer, MAX_CERT_LENGTH, (uint8_t*) ( icb.root_certificate + BEGIN_CERT_LENGTH ),
				(uint32_t) ( strlen( (char *) icb.root_certificate) - BEGIN_END_CERT_LENGTH ), IS_CERTIFICATE );
		if( cert_length == 0 ) {
			imx_printf( "Certificate is 0 length\r\n" );
			return;
		}
		cli_print( "Certificate is: %u long\r\n", cert_length );
		hex_dump( buffer, 0, (uint32_t) cert_length );

		cli_print( "Converting back to Text.." );

		buffer_index = 0;
		cert_index = sprintf( (char *) certificate, "%s\r\n", CERT_BEGIN );

		done = false;
		do {
			if( ( cert_length - buffer_index ) < CERT_RECORD_LENGTH  )
				process_count = cert_length - buffer_index;
			else
				process_count = CERT_RECORD_LENGTH;
			if( process_count > 0 ) {
				/*
				 imx_printf( "Encoding block %lu bytes, @ 0x%08lx, to: 0x%08lx, with %lu space available\r\n",
						process_count, (uint32_t) &buffer[ buffer_index ], (uint32_t) &certificate[ cert_index ], MAX_CERT_LENGTH - cert_index );
				 */
				encode_count = base64_encode( (unsigned char*) &buffer[ buffer_index ], process_count, (unsigned char *) &certificate[ cert_index ], MAX_CERT_LENGTH - cert_index, BASE64_STANDARD );
				if( encode_count == -1 ) {
					cli_print( "Encoding failed\r\n" );
					return;	// Fail for some reason
				}
				cert_index += encode_count;
				cert_index += sprintf( (char *) &certificate[ cert_index ], "\r\n" );	// Put a CR/LF between records
				buffer_index += process_count;
			} else
				done = true;
		} while( !done );

		/*
		 * Write out END of cert, first check if there is room
		 */

		cert_index += sprintf( (char *) &certificate[ cert_index ], "%s\r\n%c", CERT_END, 0x00 );

		cli_print( "Converted: %u bytes long\r\n%s", cert_index, certificate );

	} else
		cli_print( "Certificate failed to verify\r\n" );
}

/**
  * @brief  set the DCT up with default certs
  * @param  None
  * @retval : None
  */
// The following function does not work - memory locations in the DCT are not static. The DCT keeps switching back and forth between two blocks of memory.
//void set_8021X_default_certs(void)
//{
	/*
	 * We use the DCT security structure to store these default values
	 *
	 * The security structure in the DCT should stay with same structure but we need to use it a bit differently. It has provision
	 * for only two real elements and a third small key
	 * typedef struct
	 * {
     *  	char    private_key[ PRIVATE_KEY_SIZE ]; <- 2K
     * 		char    certificate[ CERTIFICATE_SIZE ]; <- 4K
     * 		uint8_t cooee_key  [ COOEE_KEY_SIZE ];
	 * } platform_dct_security_t;
	 *
	 * We use the private key to store the 802.1X WiFi Key and the certificate to store the 802.1X certificate. We use the same certificate for DTLS
	 * The root CA is embedded in the "second half" of the certificate record, but maybe changed at a later point
	 *
	 * Save the data for the certs to DCT then assign the pointers used in the code to point to these points.
	 */
//    wiced_dct_write( (void*) WIFI_USER_PRIVATE_KEY_STRING, DCT_SECURITY_SECTION, OFFSETOF(platform_dct_security_t, private_key),
//    		strlen( (char*) WIFI_USER_PRIVATE_KEY_STRING) );
//    wiced_dct_write( (void*) WIFI_USER_CERTIFICATE_STRING, DCT_SECURITY_SECTION, OFFSETOF(platform_dct_security_t, certificate),
//    		strlen( (char*) WIFI_USER_CERTIFICATE_STRING) );
//    wiced_dct_write( (void*) WIFI_ROOT_CERTIFICATE_STRING, DCT_SECURITY_SECTION, (uint32_t) ( OFFSETOF(platform_dct_security_t, certificate) + (uint32_t ) ROOT_CA_OFFSET) ,
//    		strlen( (char*) WIFI_ROOT_CERTIFICATE_STRING) );

//    update_cert_pointers();
//}


/**
  * @brief  update the pointers to certificates in the ifc block
  * @param  None
  * @retval : None
  */
void update_cert_pointers()
{

	icb.wifi_8021X_key = (uint8_t *) WIFI_USER_PRIVATE_KEY_STRING;
	icb.wifi_8021X_certificate = (uint8_t *) WIFI_USER_CERTIFICATE_STRING;
	icb.root_certificate = (uint8_t *) WIFI_ROOT_CERTIFICATE_STRING;
	icb.dtls_certificate = icb.wifi_8021X_certificate;	// Same for now

	return;

// The following code does not work - memory locations in the DCT are not static. The DCT keeps switching back and forth between two blocks of memory.
//    uint8_t* curr_dct  = (uint8_t*) wiced_dct_get_current_address( DCT_SECURITY_SECTION );

//    icb.wifi_8021X_key = (uint8_t *) ( (uint32_t) curr_dct + (uint32_t) OFFSETOF(platform_dct_security_t, private_key) );
//    icb.wifi_8021X_certificate = (uint8_t *) ( (uint32_t) curr_dct + (uint32_t) OFFSETOF(platform_dct_security_t, certificate) );
//    icb.root_certificate = (uint8_t *) ( (uint32_t) curr_dct + (uint32_t) (OFFSETOF(platform_dct_security_t, certificate) ) + (uint32_t) ROOT_CA_OFFSET );
//    icb.dtls_certificate = icb.wifi_8021X_certificate;	// Same for now
}

/**
  * @brief print_credentials - print certs and keys - only included in debugging modes
  * @param  None
  * @retval : None
  */
void print_credentials(void)
{
	update_cert_pointers();
	cli_print( "Root CA certificate @ 0x%08lx\r\n%s\r\n", (uint32_t) icb.root_certificate, icb.root_certificate );
	cli_print( "802.1X WiFi certificate: @ 0x%08lx\r\n%s\r\n", (uint32_t) icb.wifi_8021X_certificate, icb.wifi_8021X_certificate );
	cli_print( "802.1X WiFi key: @ 0x%08lx\r\n%s\r\n", (uint32_t) icb.wifi_8021X_key, icb.wifi_8021X_key );
	//cli_print( "DTLS certificate: @ 0x%08lx\r\n%s\r\n", (uint32_t) icb.dtls_certificate, icb.dtls_certificate );
}
