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
 * ota_loader.h
 *
 *  Created on: October, 2015
 *      Author: greg.phillips
 */

#ifndef OTA_LOADER_H_
#define OTA_LOADER_H_

/** @file ota_loader.h
 *
 *	support defintions for ota_loader.c
 *
 */

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
 *               Function Definitions
 ******************************************************/
void init_ota_loader(void);
bool ota_is_active(void);
bool ota_get_latest_is_active(void);
void setup_ota_loader( char *site, char *uri, uint16_t port, uint16_t image_no, uint16_t load_file );
void ota_loader(void);
void ota_loader_deinit(void);
void print_lut(uint16_t arg);
void reboot_to_image( uint16_t image_no );
void setup_get_latest_version(uint16_t image_type, char *site );
void cli_get_latest( uint16_t arg );
uint16_t get_latest_version(void);
wiced_result_t protected_wiced_framework_set_boot( uint16_t image_no, uint16_t load_mode );

#endif /* OTA_LOADER_H_ */
