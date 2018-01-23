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
 * http://ota.imatrix.io/images/7fa2176d5ac741a8a0628e1379e35509.elf
 *
 *{ "site" : "ota.imatrix.io", "uri" : "/images/7fa2176d5ac741a8a0628e1379e35509.elf", "image_no" : 5, "cksum" : 458091861}
 *
 *  Created on: October, 2015
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "wiced.h"
#include "wiced_tls.h"
#include "spi_flash.h"
#include "base64.h"
#include "../storage.h"

#include "spi_flash_fast_erase.h"

#include "../cli/interface.h"
#include "../device/icb_def.h"
#include "../device/imx_leds.h"
#include "../device/version.h"
#include "../json/mjson.h"
#include "../networking/utility.h"
#include "../sflash/sflash.h"
#include "ota_loader.h"
#include "ota_structure.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef CCMSRAM_ENABLED
#pragma message "CCMSRAM Enabled"
#define CCMSRAM __attribute__((section(".ccmram")))
#else
#define CCMSRAM
#endif

/******************************************************
 *                    Constants
 ******************************************************/
//#define   VERIFY_FLASH
/*
 * Where to store things in the flash
 */

//#define USE_TLS
#define SFLASH_SECTOR_SIZE  0x001000    // 4K
#define MICRON_SECTOR_LENGTH 0x10000    // 64K

#define FLASH_IMAGE_SIZE    0x400000    // 4M

#define CHECKSUM_LENGTH		13

#define USE_SHA512 0
char *latest_image[] =
{
        "/latest/sflash",
        "/latest/master",
        "/latest/slave",
        "/latestbeta/sflash",
        "/latestbeta/master",
        "/latestbeta/slave"
};

#define ELF_SIGNATURE_LENGTH    8
/******************************************************
 *                   Enumerations
 ******************************************************/
enum ota_loader_state_t {
    OTA_LOADER_IDLE,
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

static const char ota_root_ca[] = "-----BEGIN CERTIFICATE-----\n"
"MIIGCDCCA/CgAwIBAgIQKy5u6tl1NmwUim7bo3yMBzANBgkqhkiG9w0BAQwFADCB\n"
"hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n"
"A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n"
"BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwMjEy\n"
"MDAwMDAwWhcNMjkwMjExMjM1OTU5WjCBkDELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n"
"EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n"
"Q09NT0RPIENBIExpbWl0ZWQxNjA0BgNVBAMTLUNPTU9ETyBSU0EgRG9tYWluIFZh\n"
"bGlkYXRpb24gU2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAI7CAhnhoFmk6zg1jSz9AdDTScBkxwtiBUUWOqigwAwCfx3M28Sh\n"
"bXcDow+G+eMGnD4LgYqbSRutA776S9uMIO3Vzl5ljj4Nr0zCsLdFXlIvNN5IJGS0\n"
"Qa4Al/e+Z96e0HqnU4A7fK31llVvl0cKfIWLIpeNs4TgllfQcBhglo/uLQeTnaG6\n"
"ytHNe+nEKpooIZFNb5JPJaXyejXdJtxGpdCsWTWM/06RQ1A/WZMebFEh7lgUq/51\n"
"UHg+TLAchhP6a5i84DuUHoVS3AOTJBhuyydRReZw3iVDpA3hSqXttn7IzW3uLh0n\n"
"c13cRTCAquOyQQuvvUSH2rnlG51/ruWFgqUCAwEAAaOCAWUwggFhMB8GA1UdIwQY\n"
"MBaAFLuvfgI9+qbxPISOre44mOzZMjLUMB0GA1UdDgQWBBSQr2o6lFoL2JDqElZz\n"
"30O0Oija5zAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNV\n"
"HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRVHSAAMAgG\n"
"BmeBDAECATBMBgNVHR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9kb2NhLmNv\n"
"bS9DT01PRE9SU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDBxBggrBgEFBQcB\n"
"AQRlMGMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29tL0NPTU9E\n"
"T1JTQUFkZFRydXN0Q0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5jb21v\n"
"ZG9jYS5jb20wDQYJKoZIhvcNAQEMBQADggIBAE4rdk+SHGI2ibp3wScF9BzWRJ2p\n"
"mj6q1WZmAT7qSeaiNbz69t2Vjpk1mA42GHWx3d1Qcnyu3HeIzg/3kCDKo2cuH1Z/\n"
"e+FE6kKVxF0NAVBGFfKBiVlsit2M8RKhjTpCipj4SzR7JzsItG8kO3KdY3RYPBps\n"
"P0/HEZrIqPW1N+8QRcZs2eBelSaz662jue5/DJpmNXMyYE7l3YphLG5SEXdoltMY\n"
"dVEVABt0iN3hxzgEQyjpFv3ZBdRdRydg1vs4O2xyopT4Qhrf7W8GjEXCBgCq5Ojc\n"
"2bXhc3js9iPc0d1sjhqPpepUfJa3w/5Vjo1JXvxku88+vZbrac2/4EjxYoIQ5QxG\n"
"V/Iz2tDIY+3GH5QFlkoakdH368+PUq4NCNk+qKBR6cGHdNXJ93SrLlP7u3r7l+L4\n"
"HyaPs9Kg4DdbKDsx5Q5XLVq4rXmsXiBmGqW5prU5wfWYQ//u+aen/e7KJD2AFsQX\n"
"j4rBYKEMrltDR5FL1ZoXX/nUh8HCjLfn4g8wGTeGrODcQgPmlKidrv0PJFGUzpII\n"
"0fxQ8ANAe4hZ7Q7drNJ3gjTcBpUC2JD5Leo31Rpg0Gcg19hCC0Wvgmje3WYkN5Ap\n"
"lBlGGSW4gNfL1IYoakRwJiNiqZ+Gb7+6kHDSVneFeO/qJakXzlByjAA6quPbYzSf\n"
"+AZxAeKCINT+b72x\n"
"-----END CERTIFICATE-----";


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
extern IOT_Device_Config_t device_config;	// Defined in device/config.h and saved in DCT

extern sflash_handle_t sflash_handle;
app_header_t apps_lut[ FULL_IMAGE ] = {0};
struct OTA_CONFIGURATION ota_loader_config CCMSRAM;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  initialize the OTA loader state machine
  * @param  None
  * @retval : None
  *
  */
void init_ota_loader(void)
{
    ota_loader_config.last_ota_loader_state =
    ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
	icb.ota_loader_active = false;
	icb.get_latest_active = false;
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

#ifdef SKIP_THIS
/**
 * Execute an Over The Air(OTA) update of the firmware and return 1 (true) if successful, 0 otherwise.
 *
 * written by Eric Thelin 13 April 2017
 */
void ota_firmware_update( void )
{
#ifndef USE_HARDCODED_OTA_URL
    imx_printf( "Test of ota_firmware_update() requires USE_HARDCODED_OTA_URL be defined.\r\n");
#else
    char host[40] = {0};
    char* url = USE_HARDCODED_OTA_URL;
    char* path = "";

    // Eliminate http:// or https://
    path = strstr( url, "//" );
    if ( path != NULL ) {
        url = path + 2;
    }

    path = strchr( url, '/' );

    memcpy( host, url, path - url ); // Copy characters before '/' into host. Everything else is /0.

    imx_printf( "OTA update using: %s|%s|%s\r\n", url, host, path );
    init_ota_loader();
    setup_ota_loader( host, path, 80, APP0, WICED_TRUE );
    while( ota_is_active() )
    {
        ota_loader();
    }
#endif
}
#endif

/**
 * This function is the equivalent of the non-standard memmem() function.
 */
static void* return_item( uint8_t* data, uint32_t data_length, uint8_t* item, uint32_t item_length )
{
    uint32_t i;

    if ( ( data == NULL ) || ( data_length < item_length ) || ( item == NULL ) || ( item_length == 0 ) ) {
        return NULL;
    }

    for ( i = 0; i <= data_length - item_length; i++ ) {
        if ( 0 == memcmp( data, item, item_length) ) {
            return data;
        }
        data += 1;
    }
    return NULL;
}

/**
 * Copy temporary variables on the stack into persistent variables and start an Over The Air(OTA) update.
 * If an OTA update is already in progress, do nothing.
 */
void setup_ota_loader( char *site, char *uri, uint16_t port, uint16_t image_no, uint16_t load_file )
{
    if( ota_is_active() ) {
        imx_printf( "OTA loader already active\r\n" );
        return; // Only one can be active at any point in time
    }
    if( image_no >= NO_IMAGE_TYPES ) {
        imx_printf( "OTA loader Invalid Image type: %u\r\n", image_no );
        return; // Only one can be active at any point in time
    }

    imx_printf( "OTA loader initiated from: %s%s to image %u\r\n", site, uri, image_no );
    strcpy( ota_loader_config.site, site );
    if ( uri[ 0 ] == '/' ) {
        strcpy( ota_loader_config.uri, uri );
    }
    else {
        ota_loader_config.uri[ 0 ] = '/';
        strcpy( ota_loader_config.uri + 1, uri );
    }
    ota_loader_config.port = port;
    ota_loader_config.image_no = image_no;
    ota_loader_config.load_file = load_file;// Currently always TRUE, but maybe used in the future.

    ota_loader_config.ota_loader_state = OTA_LOADER_INIT;
//    sha4_starts( &ota_loader_config.sha512context, USE_SHA512 );// Guarantees that SHA512 is initialized,
    // but needs to be called again after each full file request is called, because of the possibility of re-trys.
}

#ifdef SKIP_THIS

wiced_bool_t verify_sha512( sha4_context* context, uint8_t hash[] )
{
    int print_byte = 0;
    unsigned char hash_output[ HASH_SIZE ];

    memset( hash_output, 0, HASH_SIZE );
    sha4_finish( context, hash_output );

    imx_printf( "SHA512                                  = ");
    for ( print_byte = 0; print_byte < 64; print_byte++ ) imx_printf( "%x,", hash_output[ print_byte ] );
    imx_printf( "\r\n" );

    if ( 0 == memcmp( hash_output, hash, HASH_SIZE )) {
        imx_printf( "SHA512, matches!\r\n");
        return WICED_TRUE;
    }
    else {
        imx_printf( "Calculated SHA512 is inconsistent with original : ");
        for ( print_byte = 0; print_byte < HASH_SIZE; print_byte++ ) imx_printf( "%x,", hash[ print_byte ] );
        imx_printf( "\r\n" );
    }
    return WICED_FALSE;
}
#endif

/**
  * @brief  ota loader state machine
  * @param  None
  * @retval : None
  */
static  uint8_t local_buffer[ BUFFER_LENGTH ] CCMSRAM;

void ota_loader(void)
{
    wiced_result_t result;
    wiced_utc_time_t utc_time;
    uint32_t i, content_length;
#ifdef VERIFY_FLASH
    uint32_t j;
#endif
    uint16_t retry_count;
    char *content_start;
    wiced_packet_t*    temp_packet = NULL;
    uint8_t* packet_data = NULL;
    uint16_t data_length, available_data_length;

    /*
     * Check the stack - we just put a big block on it.
     */
    if( ota_loader_config.last_ota_loader_state != ota_loader_config.ota_loader_state ) {
        imx_printf( "Changing OTA loader state to: %u\r\n", ota_loader_config.ota_loader_state );
        ota_loader_config.last_ota_loader_state = ota_loader_config.ota_loader_state;
    }
    /*
     * Run State machine
     */
    if( ota_loader_config.ota_loader_state % 20 == 19 ) imx_printf("\r\n");
    switch( ota_loader_config.ota_loader_state ) {
        case OTA_LOADER_INIT :// Restart point when restarting from the beginning.
            ota_loader_config.socket_assigned = false;
            ota_loader_config.good_load = false;
            ota_loader_config.flash_sector_size = get_sflash_sector_size();// This is really a constant.
            ota_loader_config.accept_ranges = false;// Until a response from the web server says that the server will accept requests for part of a file.

            ota_loader_config.content_received = 0;

/*
            image_location_t dct_app_location = {0};', DCT_APP_LOCATION_OF( ota_loader_config.image_no ), sizeof(image_location_t) ) != WICED_SUCCESS ) {
                imx_printf( "Failed to get the location from the DCT where the Over The Air Update will be saved.\r\n");
                ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                return;
            }
*/
            if ( ota_loader_config.image_no < FULL_IMAGE ) {//This is one of the standard WICED IMAGES
                get_apps_lut_if_needed();
                print_lut( 0 );
                ota_loader_config.content_offset = ( uint32_t) apps_lut[ ota_loader_config.image_no ].sectors[ 0 ].start * SFLASH_SECTOR_SIZE;
                ota_loader_config.erase_length = ( uint32_t) apps_lut[ ota_loader_config.image_no ].sectors[ 0 ].count * SFLASH_SECTOR_SIZE;
                ota_loader_config.allowed_sflash_area = WRITE_SFLASH_APP_AREA ( ota_loader_config.image_no );

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

            imx_printf( "About to erase FLash @:0x%08lX, for %lu Bytes\r\n", ota_loader_config.content_offset, ota_loader_config.erase_length );
            ota_loader_config.erase_count = ota_loader_config.content_offset;
            ota_loader_config.ota_loader_state = OTA_LOADER_ERASE_FLASH;
            break;
        case OTA_LOADER_ERASE_FLASH :
            if( sflash_erase_area( &sflash_handle, ota_loader_config.content_offset,
                    ota_loader_config.erase_length, ota_loader_config.erase_length ) != 0 )
            {
                imx_printf( "Sflash erase failed.\r\n" );
            }
            ota_loader_config.ota_loader_state = OTA_LOADER_VERIFY_ERASE;
            break;
        case OTA_LOADER_VERIFY_ERASE :
#ifdef VERIFY_FLASH
            if( ota_loader_config.erase_count < ( ota_loader_config.content_offset + ota_loader_config.erase_length ) ) {
                imx_printf( "." );
//              blink_red_led();
                fflush(stdout);
                if ( 0 != sflash_read( &sflash_handle, ota_loader_config.erase_count, (void*) &local_buffer, BUFFER_LENGTH ) ) {
                    for( j = 0; ota_loader_config.flash_sector_size / BUFFER_LENGTH; j++ )
                        for( i = 0; i < BUFFER_LENGTH; i++ )
                            if( local_buffer[ i ] != 0xff ) {
                                imx_printf( "SFLASH not erased at location: 0x%08lx\r\n", ota_loader_config.erase_count + i );
                                ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                                return;
                            }
                }
                ota_loader_config.erase_count += ota_loader_config.flash_sector_size; // SFLASH_SECTOR_SIZE;
            } else {
                imx_printf( "\r\nFlash Erased Verified\r\n" );
                ota_loader_config.ota_loader_state = OTA_DNS_LOOKUP;
            }
#else
            imx_printf( "Flash Erased Verification skipped\r\n" );
            ota_loader_config.ota_loader_state = OTA_DNS_LOOKUP;
#endif
            break;
        case OTA_DNS_LOOKUP :// Restart after a partial download here if web server supports partial downloads.
            /*
             * During OTA Dim lights up to 100% and turn ON Green LED
             */
            imx_printf( "Attempting to load OTA image: %s, from: %s\r\n", ota_loader_config.uri, ota_loader_config.site );
            imx_printf( "Doing DNS lookup...\r\n" );
            imx_set_led( IMX_LED_GREEN, IMX_LED_ON, 0 );
            imx_set_led( IMX_LED_RED, IMX_LED_OFF, 0 );
            if( get_site_ip( ota_loader_config.site, &ota_loader_config.address ) ) {
                ota_loader_config.ota_loader_state = OTA_LOADER_OPEN_SOCKET;
                return;
            } else {
                imx_printf( "Failed DNS...\r\n");
            }
            imx_printf( "Failed to get IP address, aborting OTA loader\r\n");
            ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
            return;
            break;
        case OTA_LOADER_OPEN_SOCKET :
#ifdef USE_TLS
            if ( ota_loader_config.port != 80 ) {
                wiced_tls_deinit_root_ca_certificates();
                if (wiced_tls_init_root_ca_certificates( ota_root_ca, strlen( ota_root_ca )) != WICED_SUCCESS)
                {
                    ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                }

                // initialize a TLS context, not using a TLS identity because this is a client implementation only
    //            if (wiced_tls_init_context(&ota_loader_config.context, 0, ota_loader_config.site) != WICED_SUCCESS)
                if (wiced_tls_init_context(&ota_loader_config.context, NULL, NULL) != WICED_SUCCESS)
                {
                    wiced_tls_deinit_root_ca_certificates();
                    ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                }
            }
#endif
            result = wiced_tcp_create_socket( &ota_loader_config.socket, WICED_STA_INTERFACE );
            if( result != WICED_TCPIP_SUCCESS ) {
                imx_printf( "Failed to create socket on STA Interface, aborting\r\n" );
                ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
#ifdef USE_TLS
                if ( ota_loader_config.port != 80 ) {
                    wiced_tls_deinit_root_ca_certificates();
                    wiced_tls_deinit_context(&ota_loader_config.context);
                }
#endif
                return;
            }
#ifdef USE_TLS
            if ( ota_loader_config.port != 80 ) {
                if (wiced_tcp_enable_tls(&ota_loader_config.socket, &ota_loader_config.context) != WICED_SUCCESS)
                {
                    imx_printf( "Failed to enable TLS, aborting\r\n" );
                    ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                    wiced_tls_deinit_root_ca_certificates();
                    wiced_tls_deinit_context(&ota_loader_config.context);
                    wiced_tcp_delete_socket(&ota_loader_config.socket);
                }
            }
#endif
            ota_loader_config.ota_loader_state = OTA_LOADER_ESTABLISH_CONNECTION;
            break;
        case OTA_LOADER_ESTABLISH_CONNECTION :
            retry_count = 0;

            while( retry_count < IMX_MAX_CONNECTION_RETRY_COUNT ) {
                result = wiced_tcp_connect( &ota_loader_config.socket, &ota_loader_config.address, ota_loader_config.port, 10000 );
                if( result == WICED_TCPIP_SUCCESS ) {
                    imx_printf( "Successfully Connected to: %s on Port: %u\r\n", ota_loader_config.site, ota_loader_config.port );
                    if( result == WICED_TCPIP_SUCCESS ) {
                        ota_loader_config.socket_assigned = true;
                        if ( ota_loader_config.content_received == 0 ) {
                            ota_loader_config.ota_loader_state = OTA_LOADER_SEND_REQUEST;
                        }
                        else {
                            ota_loader_config.ota_loader_state = OTA_LOADER_SEND_PARTIAL_REQUEST;
                        }
                        return;
                    } else {
                        imx_printf( "Failed TCP connect\r\n" );
                    }
                }
            }
            imx_printf( "Failed to Connect to: %s on Port: %u, aborting OTA loader\r\n", ota_loader_config.site, ota_loader_config.port );
            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
            break;
        case OTA_LOADER_SEND_REQUEST :      // Create query request and send
            memset( local_buffer, 0x00, BUFFER_LENGTH );
            strcat( (char *) local_buffer, "GET " );
            strcat( (char *) local_buffer, ota_loader_config.uri );
            strcat( (char *) local_buffer, " HTTP/1.1\nHost: " );
            strcat( (char *) local_buffer, ota_loader_config.site );
            strcat( (char *) local_buffer, "\r\n\r\n" );
            imx_printf( "Sending query: %s\r\n", local_buffer );
            result = wiced_tcp_send_buffer( &ota_loader_config.socket, local_buffer , (uint32_t) strlen( (char *) local_buffer ) );
            if ( result == WICED_TCPIP_SUCCESS ) {
                if( result == WICED_TCPIP_SUCCESS ) {
                    wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
                    ota_loader_config.ota_loader_state = OTA_LOADER_PARSE_HEADER;
                    return;
                }
            }
            imx_printf( "FAILED to send request.\r\n" );
            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
//            sha4_starts( &ota_loader_config.sha512context, USE_SHA512 );// Initialize the SHA512 computation.
            break;
        case OTA_LOADER_SEND_PARTIAL_REQUEST :      // Create query request and send
            memset( local_buffer, 0x00, BUFFER_LENGTH );
            strcat( (char *) local_buffer, "GET " );
            strcat( (char *) local_buffer, ota_loader_config.uri );
            strcat( (char *) local_buffer, " HTTP/1.1\nHost: " );
            strcat( (char *) local_buffer, ota_loader_config.site );
            sprintf( (char *) &local_buffer[ strlen( (char *) local_buffer ) ], "\nRange: bytes=%lu-%lu", ota_loader_config.content_received, ota_loader_config.total_content_length );
            strcat( (char *) local_buffer, "\r\n\r\n" );
            imx_printf( "Sending partial request query: %s\r\n", local_buffer );
            result = wiced_tcp_send_buffer( &ota_loader_config.socket, local_buffer , (uint32_t) strlen( (char *) local_buffer ) );
            if ( result == WICED_TCPIP_SUCCESS ) {
                if( result == WICED_TCPIP_SUCCESS ) {
                    wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
                    ota_loader_config.ota_loader_state = OTA_LOADER_PARSE_PARTIAL_HEADER;
                    return;
                }
            }
            imx_printf( "FAILED to send request.\r\n" );
            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
            break;
        case OTA_LOADER_PARSE_HEADER :
            result = wiced_tcp_receive( &ota_loader_config.socket, &temp_packet, 5000 );

            if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header

                wiced_packet_get_data(temp_packet, 0, &packet_data, &data_length, &available_data_length);
                imx_printf( "\r\nReceived: %u Bytes\r\n", data_length );

                uint8_t end_value;
                uint8_t* end_header;
                if ( ( end_header = return_item( packet_data, data_length, (uint8_t*)"\r\n\r\n", 4 ) ) ) {
                    end_value = *end_header;
                    *end_header = 0;
                    imx_printf( "%s\r\n", packet_data );
                    *end_header = end_value;
                }
                /*
                 * Look for 200 OK, Content length and two CR/LF - make sure we are getting what we asked for
                 */

                if( return_item( packet_data, data_length, (uint8_t*)HTTP_RESPONSE_GOOD, strlen( HTTP_RESPONSE_GOOD ) ) &&
                        return_item( packet_data, data_length, (uint8_t*)CONTENT_LENGTH, strlen( CONTENT_LENGTH ) ) &&
                        return_item( packet_data, data_length, (uint8_t*)CRLFCRLF, strlen( CRLFCRLF ) ) ) {
                    /*
                     * Header has all elements we need. Pull out the content length and save off the rest
                     */
                    if( return_item( packet_data, data_length, (uint8_t*)ACCEPT_RANGES, strlen( ACCEPT_RANGES ) ) )
                        ota_loader_config.accept_ranges = true;
                    else
                        ota_loader_config.accept_ranges = false;

                    ota_loader_config.total_content_length = strtol( return_item( packet_data, data_length, (uint8_t*)CONTENT_LENGTH, strlen( CONTENT_LENGTH ) ) + strlen( CONTENT_LENGTH ), NULL, 10 );
#ifdef SKIP_THIS
                    /*
                     * Get the SHA512 value if it was passed in.
                     */
                    char* sha = return_item( packet_data, data_length, (uint8_t*)SHA512_FIELD_NAME, strlen( SHA512_FIELD_NAME ) );
                    if ( sha == NULL ) {
                        ota_loader_config.using_sha512 = 0;
                    }
                    else {
                        ota_loader_config.using_sha512 = 1;
                        sha += strlen( SHA512_FIELD_NAME );
                        uint32_t sha_offset = (uint8_t*)sha - packet_data;
                        char* eol = return_item( (uint8_t*)sha, data_length - sha_offset, (uint8_t*)"\r", 1);
                        char* sha_i = sha;

                        imx_printf( "Received SHA512: " );
                        while ( sha_i < eol ) {
                            imx_printf( "%c", *sha_i );
                            sha_i++;
                        }
                        imx_printf("\r\n");

                        int success = base64_decode( (unsigned char*)sha, eol - sha, ota_loader_config.hash, HASH_SIZE + 1, BASE64_STANDARD );

                        if ( success != HASH_SIZE ) {
                            imx_printf("Base64 conversion failed with code: %d\r\n", success);
                        }
                    }
#endif
                    /*
                     * Find the end of the header
                     */
                    content_start = return_item( packet_data, data_length, (uint8_t*)CRLFCRLF, strlen( CRLFCRLF ) ) + strlen( CRLFCRLF );

                    for( i = 0; i < ((uint8_t*)content_start - packet_data); i++ )
                        if( isprint( (uint16_t ) packet_data[ i ] ) || ( packet_data[ i ] == '\r' ) || ( packet_data[ i ] == '\n' ) )
                            imx_printf( "%c", packet_data[ i ] );
                        else
                            imx_printf( "." );
                    for( i = ((uint8_t*)content_start - packet_data); i < data_length; i++ ) {
                        imx_printf( "%x", packet_data[ i ] );
                        if ( (i % 20) == 19 ) {
                            imx_printf( "\r\n");
                        }
                    }
                    /*
                     * Calculate the size of the real content
                     */
                    imx_printf( "\r\nTotal expected content Length: %lu\r\n", ota_loader_config.total_content_length );

                    if( ota_loader_config.total_content_length > ota_loader_config.erase_length ) {
                        imx_printf( "Content length(%lu) does not match expected erase_length(%lu).. Aborting\r\n",
                                ota_loader_config.total_content_length, ota_loader_config.erase_length );
                        ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_SOCKET;
                        wiced_packet_delete( temp_packet );
                        return;
                    }
                    /*
                     * Save the remaining part of this buffer as the first part of the image
                     */
                    content_length = (uint32_t ) ( ( uint32_t ) &packet_data[ 0 ] + data_length ) - ( uint32_t ) content_start;
                    if( content_length > 0 ) {
                        imx_printf( "Saving %lu Bytes to SFLASH\r\n", content_length );
                        /*
                         * Write bytes to flash
                         */

                        if( 0 != sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length ) ) {
//                        if( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length,
//                                ota_loader_config.allowed_sflash_area ) ) {
                            imx_printf( "Write to serial flash failed!\r\n" );
                            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                            wiced_packet_delete( temp_packet );
                            return;
                        }

//                        sha4_update( &ota_loader_config.sha512context, (const unsigned char*) content_start, content_length );

                        ota_loader_config.content_received = content_length;
                        ota_loader_config.content_offset += content_length;
                    }
                    wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );
                    ota_loader_config.packet_count = 1;
                    ota_loader_config.ota_loader_state = OTA_LOADER_RECEIVE_STREAM;
                    wiced_packet_delete( temp_packet );
                    return;
                } else {
                    imx_printf( "Garbage back - no header found: %80s\r\n", packet_data );
                    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                    wiced_packet_delete( temp_packet );
                    return;
                }
                wiced_packet_delete( temp_packet );
                temp_packet = NULL;
                packet_data = NULL;
                data_length = 0;
            }else {
                wiced_time_get_utc_time( &utc_time );
                if( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) {
                    ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
                }
            }
            break;

        case OTA_LOADER_PARSE_PARTIAL_HEADER :
            result = wiced_tcp_receive( &ota_loader_config.socket, &temp_packet, 5000 );

            if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header

                wiced_packet_get_data(temp_packet, 0, &packet_data, &data_length, &available_data_length);

                imx_printf( "\r\nReceived: %d Bytes\r\n", data_length );

                uint8_t end_value;
                uint8_t* end_header;
                if ( ( end_header = return_item( packet_data, data_length, (uint8_t*)"\r\n\r\n", 4 ) ) ) {
                    end_value = *end_header;
                    *end_header = 0;
                    imx_printf( "%s\r\n", packet_data );
                    *end_header = end_value;
                }

//                for( i = 0; i < buffer_length; i++ )
//                    if( isprint( (uint16_t ) local_buffer[ i ] ) || ( local_buffer[ i ] == '\r' ) || ( local_buffer[ i ] == '\n' ) )
//                        imx_printf( "%c", local_buffer[ i ] );
//                    else
//                        imx_printf( "." );

                 // Look for 206 OK, Content length and two CR/LF - make sure we are getting what we asked for
                if( return_item( packet_data, data_length, (uint8_t*)HTTP_RESPONSE_PARTIAL, strlen( HTTP_RESPONSE_PARTIAL ) ) &&
                    return_item( packet_data, data_length, (uint8_t*)CRLFCRLF, strlen( CRLFCRLF ) ) )
                {
                    // Find the end of the header

                    content_start = return_item( packet_data, data_length,  (uint8_t*)CRLFCRLF, strlen( CRLFCRLF ) ) + strlen( CRLFCRLF );

                    // Save the remaining part of this buffer as the first part of the image

                    content_length = (uint32_t ) ( ( uint32_t ) &packet_data[ 0 ] + data_length ) - ( uint32_t ) content_start;
                    if( content_length > 0 ) {
                        imx_printf( "Saving %lu Bytes to SFLASH\r\n", content_length );

                        // Write bytes to flash

                        if( 0 != sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length ) ) {
//                        if( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) content_start, content_length,
//                                ota_loader_config.allowed_sflash_area ) ) {
                            imx_printf( "Write to serial flash failed!\r\n" );
//                          device_config.ota_fail_sflash_write += 1;
//                          save_config();
                            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                            wiced_packet_delete( temp_packet );
                            return;
                        }

                        ota_loader_config.content_received += content_length;
                        ota_loader_config.content_offset += content_length;
                    }
                    wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );

                    // We may have got all the data needed in the block so check if we are done or if we have more to receive

                    if( ota_loader_config.content_received == ota_loader_config.total_content_length ) {
                        ota_loader_config.ota_loader_state = OTA_LOADER_ALL_RECEIVED;
                        wiced_packet_delete( temp_packet );
                        return;
                    }
                    ota_loader_config.packet_count += 1;
                    ota_loader_config.ota_loader_state = OTA_LOADER_RECEIVE_STREAM;
                    wiced_packet_delete( temp_packet );
                    return;
                }
            } else {
                wiced_time_get_utc_time( &utc_time );
                if( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) {
                    ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
                }
            }
            break;

        case OTA_LOADER_RECEIVE_STREAM :
            result = wiced_tcp_receive( &ota_loader_config.socket, &temp_packet, 5000 );
//wiced_rtos_delay_milliseconds(500);
            if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
                wiced_packet_get_data(temp_packet, 0, &packet_data, &data_length, &available_data_length);

                if ( data_length > BUFFER_LENGTH ) {
                    imx_printf( "The local buffer was too small to store the contents of a received packet.\r\n" );
                    while ( 1 );
                }
                memmove( local_buffer, packet_data, data_length);
                wiced_packet_delete( temp_packet );
                temp_packet = NULL;
                packet_data = NULL;

                ota_loader_config.packet_count += 1;
                /*
                 * Check to make sure we don't get memory overwrite
                 */
                if( ota_loader_config.content_received + data_length > ota_loader_config.total_content_length  ) {
                    /*
                     * Abort as we would over run the allocated space
                     */
                    imx_printf( "Data length exceeded: %lu + %u = %lu\r\n", ota_loader_config.content_received , data_length, ota_loader_config.content_received + data_length);
                    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                    return;
                }
                /*
                 * Write bytes to flash
                 */
                wiced_time_t process_start = 0, process_end = 0, sflash_end = 0, crc_end = 0;

                wiced_time_get_time( &process_start );

                if ( 0 != sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) local_buffer, data_length ) ) {
//                if ( 0 != protected_sflash_write( &sflash_handle, ota_loader_config.content_offset, (void*) local_buffer, data_length,
//                        ota_loader_config.allowed_sflash_area ) ) {
                    imx_printf( "Write to serial flash failed.\r\n");
                    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                    wiced_packet_delete( temp_packet );
                    return;
                }
                wiced_time_get_time( &sflash_end );

                imx_printf( "(%u)Packet %u, @: 0x%08lX ", ota_loader_config.data_retry_count, ota_loader_config.packet_count, ota_loader_config.content_offset );

                wiced_time_get_time( &crc_end );

//                sha4_update( &ota_loader_config.sha512context, (const unsigned char*) local_buffer, data_length );

                wiced_time_get_time( &process_end );

                ota_loader_config.content_received += data_length;
                ota_loader_config.content_offset += data_length;
                imx_printf( "%u Bytes Now %lu/%lu ms: %lu/%lu/%lu/%lu\r\n", data_length, ota_loader_config.content_received, ota_loader_config.total_content_length,
                        process_end - process_start, sflash_end - process_start, crc_end - sflash_end, process_end - crc_end );
                /*
                 * Check if we have all the data yet
                 */
/* For testing timeout/restart functionality by forcing a timeout in the middle of a download.
                if ( ( ota_loader_config.packet_count >= 150 ) && ( ota_loader_config.data_retry_count == 0 ) ) {// Force timeout.
                    if ( ota_loader_config.tcp_stream.rx_packet != NULL ) {
                        wiced_packet_delete( ota_loader_config.tcp_stream.rx_packet );
                        ota_loader_config.tcp_stream.rx_packet = NULL;
                    }
                    ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
                    return;
                }
*/
                if( ota_loader_config.content_received == ota_loader_config.total_content_length ) {
                    ota_loader_config.ota_loader_state = OTA_LOADER_ALL_RECEIVED;
                    return;
                }
                wiced_time_get_utc_time( &ota_loader_config.last_recv_packet_utc_time );    // Set last time we got a packet
                data_length = 0;
            }else {
                imx_printf("wiced_tcp_receive() failed with error code: %u\r\n", (unsigned int)result);
                wiced_time_get_utc_time( &utc_time );
                if( ( result == 7014 ) || ( utc_time > ota_loader_config.last_recv_packet_utc_time + TIMEOUT_WAIT_FOR_DATA ) ) {
                    ota_loader_config.ota_loader_state = OTA_LOADER_DATA_TIMEOUT;
                }
            }
            break;
        case OTA_LOADER_ALL_RECEIVED :
#ifdef SKIP_THIS
            if ( ota_loader_config.using_sha512 != 0 ) {
                if ( WICED_TRUE != verify_sha512( &ota_loader_config.sha512context, ota_loader_config.hash ) )
                {
                    imx_printf("Aborting update.\r\n");
                    ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                    return;
                }
            }

            else {
                imx_printf("No SHA512 was sent.\r\n");// Store hash from downloaded file to verify sflash write.
                sha4_finish( &ota_loader_config.sha512context, ota_loader_config.hash );
                int print_byte = 0;
                imx_printf( "SHA512                                  = ");
                for ( print_byte = 0; print_byte < 64; print_byte++ ) imx_printf( "%x,", ota_loader_config.hash[ print_byte ] );
                imx_printf( "\r\n" );
            }
#endif
            // Set content_offset back to the start of the downloaded file in flash.
            ota_loader_config.content_offset -= ota_loader_config.content_received;
            ota_loader_config.ota_loader_state = OTA_LOADER_VERIFY_OTA;
            return;
            break;
        case OTA_LOADER_VERIFY_OTA :
//            sha4_starts( &ota_loader_config.sha512context, USE_SHA512 );
            ota_loader_config.crc_content_offset = ota_loader_config.content_offset;// - ota_loader_config.content_received;
            ota_loader_config.crc_content_end = ota_loader_config.content_offset + ota_loader_config.content_received;
//            imx_printf( "Calculating SFLASH SHA512 hash.\r\n" );
            /*
             * Blink LED to indicate action
             */
            imx_set_led( IMX_LED_RED, IMX_LED_OTHER, IMX_LED_BLINK_1 | IMX_LED_BLINK_1_8 );
            ota_loader_config.good_load = true;
            ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
//            ota_loader_config.ota_loader_state = OTA_CALC_CRC;
            break;
#ifdef SKIP_THIS
        case OTA_CALC_CRC :
            if( ota_loader_config.crc_content_offset < ota_loader_config.crc_content_end ) {
//              blink_red_led();
                uint32_t bytes_remaining = ota_loader_config.crc_content_end -  ota_loader_config.crc_content_offset;
                uint16_t length = BUFFER_LENGTH;
                if ( bytes_remaining < BUFFER_LENGTH ) {
                    length = bytes_remaining;
                }
                sflash_read( &sflash_handle, ota_loader_config.crc_content_offset, local_buffer, length );
//                sha4_update( &ota_loader_config.sha512context, local_buffer, length );
                ota_loader_config.crc_content_offset += length;
            } else if ( WICED_TRUE != verify_sha512( &ota_loader_config.sha512context, ota_loader_config.hash ) ) {
                imx_printf( "SHA512 hash for saved file is different from the hash computed from the download.\r\n" );
                imx_printf( "Writing file to flash failed! Aborting update.\r\n" );
                ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                return;
            } else {
                imx_printf( "Sflash write is good\r\n" );
                ota_loader_config.good_load = true;
                ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
            }
            break;
#endif
        case OTA_LOADER_DATA_TIMEOUT :
            ota_loader_config.data_retry_count++;
            if( ota_loader_config.data_retry_count > MAX_DATA_RETRY_COUNT ) {
                imx_printf( "Max Retry Count exceeded. Giving up\r\n" );
                ota_loader_config.ota_loader_state = OTA_LOADER_CLOSE_CONNECTION;
                return;
            }
            wiced_tcp_disconnect( &ota_loader_config.socket );
            wiced_tcp_delete_socket( &ota_loader_config.socket );
            imx_printf( "Timeout Retry Count %u\r\n", ota_loader_config.data_retry_count );
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
                imx_printf("Save is good.\r\n");
                ota_loader_config.ota_loader_state = OTA_LOADER_DONE;
            }
            else {
                imx_printf("Save failed.\r\n");
                ota_loader_config.ota_loader_state = OTA_PRE_LOADER_IDLE;
            }
#ifdef USE_TLS
            if ( ota_loader_config.port != 80 ) {
                wiced_tls_deinit_context(&ota_loader_config.context);
                wiced_tls_deinit_root_ca_certificates();
            }
#endif

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
                    return;
                }
/* This code was for downloading updates other than the main system image. -- Not used for birdi.
                else if( ( ota_loader_config.image_no == APP2 ) || ( ota_loader_config.image_no == WIFI_DATA ) ) {
                } else if( ota_loader_config.image_no == FULL_IMAGE ) {
                    if( device_config.do_SFLASH_load == true ) {    // Got it successfully no need to do it next boot
                        device_config.do_SFLASH_load = false;
                        save_config();


						// Go to idle mode with GREEN LED on

			            imx_set_led( IMX_LED_GREEN, IMX_LED_OFF, 0 );
			            imx_set_led( IMX_LED_RED, IMX_LED_ON, 0 );                        imx_printf( "OTA loader completed loading SFLASH, in IDLE mode\r\n" );
                        ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
                        return;
                    }
                }
*/
            }
            ota_loader_config.ota_loader_state = OTA_PRE_LOADER_IDLE;
            break;
        case OTA_PRE_LOADER_IDLE :
//          update_led_red_status( false );
            imx_printf( "OTA loader completed, returning to IDLE\r\n" );
            imx_set_led( IMX_LED_GREEN, IMX_LED_OFF, 0 );
            imx_set_led( IMX_LED_RED, IMX_LED_OFF, 0 );
            ota_loader_config.ota_loader_state = OTA_LOADER_IDLE;
            break;
        case OTA_LOADER_IDLE :
//          update_led_green_status( true );    // Make sure it stays ON
            break;
        default :
            break;
    }
    return;
}

void print_lut(uint16_t arg)
{
    UNUSED_PARAMETER(arg);

    uint16_t i, j;

    get_apps_lut_if_needed();
    imx_printf( "Current LUT\r\n" );
    for( i = 0; i < 8; i++ ) {//NO_IMAGE_TYPES; i++ ) {
        cli_print( "Entry: %u, Sector Count: %u, Secure: %s\r\n", i, apps_lut[ i ].count,
                apps_lut[ i ].secure ? "True" : "False" );
        if( apps_lut[ i ].count != 0xFF )   // Real Data
            for( j = 0; j < apps_lut[ i ].count; j++  )
                cli_print( "Start Sector: %X Address: 0x%08lX, Length in sectors: %u - %lu Bytes ", apps_lut[ i ].sectors[ j ].start,
                    ( uint32_t ) ( apps_lut[ i ].sectors[ j ].start ) * SFLASH_SECTOR_SIZE,
                    apps_lut[ i ].sectors[ j ].count,
                    ( uint32_t ) ( apps_lut[ i ].sectors[ j ].count ) * SFLASH_SECTOR_SIZE );
        cli_print("\r\n" );
    }

}

void cli_get_latest( uint16_t arg )
{
    setup_get_latest_version( OTA_IMAGE_MASTER, device_config.ota_public_url );
}

void setup_get_latest_version(uint16_t image_type, char *site )
{
	icb.get_latest_active = true;
	ota_loader_config.retry_count = 0;
	ota_loader_config.good_get_latest = false;
	ota_loader_config.ota_getlatest_state = GET_LATEST_DNS;
	ota_loader_config.image_type = image_type;
	memmove( ota_loader_config.site, site, IMX_IMATRIX_SITE_LENGTH );

}

uint16_t get_latest_version(void)
{
	uint32_t buffer_length;
	wiced_result_t result;
	char local_buffer[ BUFFER_LENGTH ], version[ VERSION_LENGTH ], checksum[ CHECKSUM_LENGTH ], uri[ IMX_IMATRIX_URI_LENGTH ], my_version[ VERSION_LENGTH ];
	char *content_start, *content_end;
    struct json_attr_t json_attrs[] = {
            {"image_url",  t_string, .addr.string = uri, .len = IMX_IMATRIX_URI_LENGTH },
            {"version", t_string, .addr.string = version, .len = VERSION_LENGTH },
			{"checksum", t_string, .addr.string = checksum, .len = CHECKSUM_LENGTH },// initially tried using t_uinteger, but that is only a 31 bit integer.
            {NULL}
    };

	switch( ota_loader_config.ota_getlatest_state ) {
		case GET_LATEST_DNS :
            imx_printf( "DNS Lookup for site: %s\r\n", ota_loader_config.site );
            if( get_site_ip( ota_loader_config.site, &ota_loader_config.address ) == true ) {
                imx_printf("IP address %lu.%lu.%lu.%lu\r\n",
                        ( ota_loader_config.address.ip.v4 >> 24) & 0xFF,
                        ( ota_loader_config.address.ip.v4 >> 16) & 0xFF,
                        ( ota_loader_config.address.ip.v4 >> 8 ) & 0xFF,
                        ( ota_loader_config.address.ip.v4 & 0xFF) );
                    ota_loader_config.ota_getlatest_state = GET_LATEST_OPEN_SOCKET;
                    break;
            } else {
                imx_printf( "Failed DNS... \r\n" );
                imx_printf( "Failed to get IP address, aborting getting latest revision of: %u\r\n", ota_loader_config.image_type);
                ota_loader_config.ota_getlatest_state = GET_LATEST_IDLE;
                icb.get_latest_active = false;
                return true;    // We are done
            }
			break;
		case GET_LATEST_OPEN_SOCKET :
			result = wiced_tcp_create_socket( &ota_loader_config.socket, WICED_STA_INTERFACE );
			if( result != WICED_TCPIP_SUCCESS ) {
				imx_printf( "Failed to create socket on STA Interface, aborting\r\n" );
				ota_loader_config.ota_getlatest_state = GET_LATEST_IDLE;
				icb.get_latest_active = false;
				return true;	// We are done
			}
			ota_loader_config.ota_getlatest_state = GET_LATEST_ESTABLISH_CONNECTION;
			break;
		case GET_LATEST_ESTABLISH_CONNECTION :
			ota_loader_config.retry_count = 0;
			while( ota_loader_config.retry_count < IMX_MAX_CONNECTION_RETRY_COUNT ) {
				result = wiced_tcp_connect( &ota_loader_config.socket, &ota_loader_config.address, 80, 1000 );
				if( result == WICED_TCPIP_SUCCESS ) {
					imx_printf( "Successfully Connected to: %s on Port: 80\r\n", ota_loader_config.site );
					/*
					 * Create the stream
					 */
					result = wiced_tcp_stream_init( &ota_loader_config.tcp_stream, &ota_loader_config.socket );
					if( result == WICED_TCPIP_SUCCESS ) {
						ota_loader_config.ota_getlatest_state = GET_LATEST_SEND_REQUEST;
						break;
					} else {
						imx_printf( "Failed to connect to stream\r\n" );
					}
				}
			}
			if( ota_loader_config.retry_count >= IMX_MAX_CONNECTION_RETRY_COUNT ) {
				imx_printf( "Failed to Connect to: %s on Port: 80, aborting OTA loader\r\n", ota_loader_config.site );
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
		    imx_printf( "Sending query: %s\r\n", local_buffer );
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
		    	imx_printf( "FAILED to send request.\r\n" );
		    	ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_SOCKET;
		    }
		    break;
		case GET_LATEST_PARSE_HEADER :
			result = wiced_tcp_stream_read_with_count( &ota_loader_config.tcp_stream, local_buffer, BUFFER_LENGTH, 10000, &buffer_length );
			if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - This will be the header
				imx_printf( "\r\nReceived: %lu Bytes\r\n", buffer_length );

				/*
				 *
			    uint32_t i;
				for( i = 0; i < buffer_length; i++ )
					if( isprint( (uint16_t ) local_buffer[ i ] ) || ( local_buffer[ i ] == '\r' ) || ( local_buffer[ i ] == '\n' ) )
						imx_printf( "%c", local_buffer[ i ] );
					else
						imx_printf( "." );

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
						imx_printf( "JSON not found in body\r\n" );
						ota_loader_config.ota_getlatest_state = GET_LATEST_CLOSE_CONNECTION;
					} else {
						*content_end = 0x00;	// Null terminate the string

					imx_printf( "JSON: %s\r\n", content_start );
				    /*
				     * Process the passed URI Query
				     */
				    result = json_read_object( content_start, json_attrs, NULL );

				    if( result ) {
				        imx_printf( "JSON parsing failed, Result: %u\r\n", result );
				        return( false );
				    }

				    if(strcmp( uri, "" ) == 0 ) {
				        imx_printf( "Missing uri value in JSON from request.\r\n");
				        return( false );
				    }
				    memmove( ota_loader_config.uri, uri, IMX_IMATRIX_URI_LENGTH );

				    if(strcmp( version, "" ) == 0 ) {
				        imx_printf( "Missing Version value in JSON from request.\r\n");
				        return( false );
				    }
				    memmove( ota_loader_config.version, version, VERSION_LENGTH );

				    if(strcmp( checksum, "" ) == 0 ) {
				        imx_printf( "Missing checksum value in JSON from request.\r\n");
				        return( false );
				    }
//				    ota_loader_config.checksum32 = atol( checksum );

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
	    	icb.get_latest_active = false;
		    wiced_tcp_disconnect( &ota_loader_config.socket );
		    wiced_tcp_delete_socket( &ota_loader_config.socket );
		    if( ota_loader_config.good_get_latest ) {
		    	if( ota_loader_config.image_type == OTA_IMAGE_MASTER ) {	// Only check version for MASTER flash
			    	sprintf( my_version, VERSION_FORMAT, MajorVersion, MinorVersion, AUTO_BUILD );
			    	if( strcmp( my_version, ota_loader_config.version ) >= 0 ) {
			    		imx_printf( "We have the latest Version: %s, No need to upgrade to: %s\r\n", my_version, ota_loader_config.version );
			    		return( true );
			    	}
		    	}
		    	imx_printf("Got the uri and checksum.\r\n");
		    	if( ( ota_loader_config.image_type == OTA_IMAGE_SFLASH ) ||
		    		( ota_loader_config.image_type == OTA_IMAGE_BETA_SFLASH ) )
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, 80, FULL_IMAGE, true );
		    	else if( ( ota_loader_config.image_type == OTA_IMAGE_MASTER ) ||
		    			 ( ota_loader_config.image_type == OTA_IMAGE_BETA_MASTER ) )
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, 80, APP0, true );
		    	else
		    		setup_ota_loader( ota_loader_config.site, ota_loader_config.uri, 80, APP2, true );
		    }
		    else {
		    	imx_printf("Get Latest failed.\r\n");
		    }
	    	icb.get_latest_active = false;
    		return( true );
			break;
		case GET_LATEST_IDLE :
			return true;
			break;
	}
	return( false );
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
        printf( "Rebooting to APP0: %u\r\n", DCT_APP0_INDEX);
    }
    else if( image_no == APP1 ) {
        result = protected_wiced_framework_set_boot( DCT_APP1_INDEX, 1 /* PLATFORM_DEFAULT_LOAD */ );
        printf( "Rebooting to APP1: %u\r\n", DCT_APP1_INDEX);
    }
    else {
        printf( "Invalid image number to boot from\r\n" );
        return;
    }
    if( result == WICED_SUCCESS ) {
        printf( "Framework Set successfully\r\n" );
//        print_dct_header();
        printf( "Rebooting to APP...\r\n" );
        wiced_rtos_delay_milliseconds( 2000 );
        wiced_framework_reboot();
    } else
        printf( "Failed to Set Boot - " );
    printf( "Reboot failed!\r\n" );

}
/**
  * @brief  Verify that the target is a vaild ELF image
  * @param  image_no, boot mode
  * @retval : result
  */

wiced_result_t protected_wiced_framework_set_boot( uint16_t image_no, uint16_t load_mode )
{
    uint8_t buffer[ ELF_SIGNATURE_LENGTH ];
    uint8_t valid_elf[ ELF_SIGNATURE_LENGTH ] = { 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 00 };
    wiced_result_t result;

//    wiced_framework_set_boot( image_no, load_mode );
//    if(1) return WICED_SUCCESS;

    image_location_t dct_app_location = {0};

    result = wiced_dct_read_with_copy( &dct_app_location, DCT_INTERNAL_SECTION, DCT_APP_LOCATION_OF( image_no ), sizeof(image_location_t) );
    if ( result != WICED_SUCCESS ) {
        printf( "Failed to get the location from the DCT for a system image to boot into.\r\n");
        return result;
    }

    result = sflash_read( &sflash_handle, dct_app_location.detail.external_fixed.location, (void*) buffer, ELF_SIGNATURE_LENGTH );
    if( result != WICED_SUCCESS )
        return result;

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
        printf( "Elf signature Verified\r\n" );
        return( wiced_framework_set_boot( image_no, load_mode ) );
    } else {
        printf( "Selected Image: %u does not have a valid ELF signature\r\n", image_no );
        return WICED_ERROR;
    }

}
