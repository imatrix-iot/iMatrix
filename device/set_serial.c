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
/** @file
 *	set_serial.c
 *
 *  Created on: February, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/config.h"
#include "../cli/interface.h"

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
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	set_serial_number
  * @param  None
  * @retval : None
  *
  * In this implementation the system will use the serial number Internally in the ARM Processor
  */
#define SERIAL_NUM_ADDR	0x1FFF7A10

void set_serial_number(void)
{
	uint32_t *ptr;

	/*
	 * Get the serial number from the device
	 */
#ifdef USE_STM32
	ptr = (uint32_t*) SERIAL_NUM_ADDR;
    device_config.sn.serial1 = *(ptr);
    device_config.sn.serial2 = *(ptr + 1);
    device_config.sn.serial3 = *(ptr + 2);
#endif

#ifdef USE_CYW943907
	//ptr = (uint32_t*) SERIAL_NUM_ADDR;
	device_config.sn.serial1 = 1; //*(ptr);
	device_config.sn.serial2 = 2; //*(ptr + 1);
	device_config.sn.serial3 = 3; //*(ptr + 2);
#endif

	imatrix_save_config();

}

void print_serial_number(void)
{

	cli_print( "Serial numbers Hardware: 3 %lu, Hardware: 2 %lu, Hardware 1: %lu\r\n", device_config.sn.serial3, device_config.sn.serial2, device_config.sn.serial1 );

}
