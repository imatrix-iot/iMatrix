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

/** @file sflash.h
 *
 *  Created on: September 9, 2017
 *      Author: greg.phillips
 *
 */


#ifndef SFLASH_H_
#define SFLASH_H_

#include "spi_flash.h"

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
uint32_t get_sflash_sector_size(void);
uint16_t init_serial_flash(void);
int read_serial_flash_status(void);
void get_apps_lut_if_needed();
wiced_result_t erase_64KB_sector( uint32_t sector_start_address, uint16_t allowed_area );
int protected_sflash_write(const sflash_handle_t* const handle, unsigned long device_address, /*@observer@*/ const void* const data_addr, unsigned int size, uint16_t allowed_areas  );
int protected_sflash_sector_erase( const sflash_handle_t* const handle, unsigned long device_address, uint16_t allowed_area );
void test_sflash( wiced_system_monitor_t* watchdog, uint32_t watchdog_delay );
wiced_result_t write_config(uint16_t config_no );
wiced_result_t erase_64KB_sector( uint32_t sector_start_address, uint16_t allowed_area );
void reboot_if( uint16_t failed, char* msg );

#endif /* SFLASH_H_ */
