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
/*
 * ota_structure.h
 *
 *  Created on: May, 2016
 *      Author: greg.phillips
 */

#ifndef OTA_STRUCTURE_H_
#define OTA_STRUCTURE_H_

/** @file
 *
 *	Defines for ota structures
 *
 */
#include "wiced_apps_common.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define WRITE_SFLASH_APP_AREA(app_image_type)   ( 1 << app_image_type )
#define WRITE_SFLASH_LUT_AREA                   ( 1 << LUT )
#define WRITE_SFLASH_CONFIG_AREA                ( 1 << FULL_IMAGE )
#define WRITE_SFLASH_ANY_AREA                   ( 0xFFFF )
#define WRITE_SFLASH_UNPARTITIONED_SPACE        ( 1 << NO_IMAGE_TYPES )

#define MAX_DATA_RETRY_COUNT        10
#define TIMEOUT_WAIT_FOR_DATA       20
#define BUFFER_LENGTH               2048
#define VERSION_LENGTH              13

#define HTTP_RESPONSE_GOOD          "200 OK\r\n"
#define HTTP_RESPONSE_PARTIAL       "206 Partial Content\r\n"
#define HTTP_RESPONSE_BAD           "400 OK\r\n"
#define HTTP_RESPONSE_NOT_FOUND     "404 Not Found"

#define CONTENT_LENGTH              "Content-Length:"
#define ACCEPT_RANGES               "Accept-Ranges: bytes"
#define CRLFCRLF                    "\r\n\r\n"

enum ota_getlatest_image_types_t {
    OTA_IMAGE_SFLASH,
    OTA_IMAGE_MASTER,
    OTA_IMAGE_SLAVE,
    OTA_IMAGE_BETA_SFLASH,
    OTA_IMAGE_BETA_MASTER,
    OTA_IMAGE_BETA_SLAVE,
    OTA_IMAGE_NO_IMAGES
};

enum image_types_t {// Definitions for the 8 DCT_??_APP_INDEX constants are in WICED/platform/include/platform_dct.h
    FACTORY_RESET = DCT_FR_APP_INDEX,           // 0
    DCT           = DCT_DCT_IMAGE_INDEX,        // 1
    LEGACY_OTA    = DCT_OTA_APP_INDEX,          // 2
    RESOURCE_FILE = DCT_FILESYSTEM_IMAGE_INDEX, // 3
    WIFI_DATA     = DCT_WIFI_FIRMWARE_INDEX,    // 4
    APP0          = DCT_APP0_INDEX,             // 5
    APP1          = DCT_APP1_INDEX,             // 6
    APP2          = DCT_APP2_INDEX,             // 7
    FULL_IMAGE,         // 8 - LUT and FULL_IMAGE are not part of the standard 8 DCT_??_APP_INDEX constants
    LUT,                // 9
    NO_IMAGE_TYPES
};


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
struct OTA_CONFIGURATION {
	char site[ IMX_IMATRIX_SITE_LENGTH ], uri[ IMX_IMATRIX_URI_LENGTH ], version[ VERSION_LENGTH ], *buffer;
	wiced_ip_address_t address;
    uint16_t image_no;
    uint16_t image_type;
    uint16_t ota_loader_state;
    uint16_t last_ota_loader_state;
    uint16_t ota_getlatest_state;
    uint16_t packet_count;
    uint32_t max_content_length;
    uint32_t content_received;
    wiced_utc_time_t last_recv_packet_utc_time;
	uint16_t get_latest_retry_count;
    uint16_t data_retry_count;
    uint16_t retry_count;
    uint32_t content_length;
    uint32_t total_content_length;
    uint32_t content_offset;
    uint32_t erase_length;
    uint32_t erase_count;
    uint32_t flash_sector_size;
    uint32_t crc_content_offset;
    uint32_t crc_content_end;
    wiced_tcp_socket_t socket;
    wiced_tcp_stream_t tcp_stream;
    uint32_t checksum;
    uint32_t checksum32;
    uint16_t allowed_sflash_area;
    unsigned int socket_assigned	: 1;
    unsigned int good_load 			: 1;
    unsigned int good_get_latest 	: 1;
    unsigned int good_get_power 	: 1;
    unsigned int load_file 			: 1;
    unsigned int accept_ranges 		: 1;
};

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* OTA_STRUCTURE_H_ */
