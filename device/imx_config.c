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
 *
 *	imx_config.c
 *	
 *	Read / Write the Applicationconfiguration
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device_app_dct.h"
#include "../cli/interface.h"
#include "icb_def.h"
#include "config.h"

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

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	print saved configuration
  * @param  None
  * @retval : None
  */

wiced_result_t imx_get_config_current_address( void *config_address )
{

	// The address in the DCT becomes invalid as soon as anything is written to the DCT
	uint8_t *temp_app_config = (uint8_t*) ( (uint32_t)wiced_dct_get_current_address( DCT_APP_SECTION ) + OFFSETOF( device_app_dct_t, app_config ) );
	if ( temp_app_config == GET_CURRENT_ADDRESS_FAILED ) {
	    imx_printf( "DCT access error while attempting to print the saved device configuration.\r\n");
	    return WICED_ERROR;
	}

	config_address = (void *) temp_app_config;

    return WICED_SUCCESS;
}
/**
  * @brief  print saved configuration
  * @param  None
  * @retval : None
  */

wiced_result_t imx_save_config( void *data, uint16_t length )
{
   if( length > APP_CONFIG_SIZE ) {
       imx_printf( "Application Configuration exceeds space allocated( %u Bytes): %u Bytes\r\n", APP_CONFIG_SIZE, length );
       return WICED_ERROR;
   }

   return wiced_dct_write( &device_config, DCT_APP_SECTION, OFFSETOF( device_app_dct_t, app_config ), APP_CONFIG_SIZE );

    return WICED_SUCCESS;
}

/**
  * @brief  return the saved serial number
  * @param  None
  * @retval : None
  */

char *imx_get_device_serial_number( void )
{
    return (char *) &device_config.device_serial_number;
}
