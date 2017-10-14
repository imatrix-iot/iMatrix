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
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "wiced_time.h"

#include "imatrix.h"

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define APP_CONFIG_SIZE                     ( 1024 )        // 1K For Application Configuration
#define HARDCODED_DCT_APP_SECTION_OFFSET    ( 0x1f40 )
#define OTA_IMAGE_URL_LENGTH                ( 130 )
#define OTA_IMAGE_TYPE_LENGTH               ( 32 )
// The tick timer comparisons are only valid for 1/4 of the range of possible values so that is max allowed delay
#define MAX_PERMITTED_DELAY_FOR_OTA         ( 0x40000000 )
// Do not allow extra random delays for OTA more than 2 days in milliseconds
#define MAX_PERMITTED_RANDOM_DELAY_FOR_OTA ( 2 * DAYS )


/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/


// OTA image URL is 128 bytes + 2 bytes padding to make sure app_dct_header is aligned.
// This allows for strings as long as 129 bytes long.

typedef struct app_header {
    uint8_t dct_version ;
    struct bit_flags_indicating_if_known_good_image_is_in_the_default_location {// 0 means default, 1 means alternate location
        unsigned int system_image:1;
        unsigned int ota_image:1;
        unsigned int psoc_image:1;
    } changed_known_good;
    struct bit_flags_indicating_if_default_or_alternate_image_is_currently_in_use {// 0 means default, 1 means alternate
        unsigned int system_image:1;
        unsigned int ota_image:1;
        unsigned int psoc_image:1;
    } using_alternate;
    unsigned int ota_app_version:2;// version 1 by default. Version 0 means legacy OTA that does not interact with DCT.
    // If version is 0, and OTA app ever interacts with this DCT, it will make itself the known good and update this.
    // Otherwise if version > 0 the OTA app will only update this version if the current app is already known good.
    char ota_image_url[ OTA_IMAGE_URL_LENGTH ];
} app_dct_header_t;

typedef struct full_app_section
{
    app_dct_header_t header;
    IOT_Device_Config_t config;
    uint8_t app_config[ APP_CONFIG_SIZE ];
} device_app_dct_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
device_app_dct_t *get_dct_config(void);
