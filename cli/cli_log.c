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
 * cli_log.c
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "wiced.h"
#include "../storage.h"
#include "spi_flash.h"

#include "interface.h"
#include "../device/config.h"
#include "cli_log.h"
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
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief
  * @param  None
  * @retval : None
  */
void cli_log(uint16_t mode)
{
	bool update_config;
	char *token;
	/*
	 *	command format log <on|off>
	 */
	update_config = false;
	token = strtok(NULL, " " );	// Get start if any
	if( token ) {
		if( strcmp( token, "on" ) == 0 ) {
			device_config.send_logs_to_imatrix = true;
			update_config = true;
		} else if( strcmp( token, "on" ) == 0 ) {
		    device_config.send_logs_to_imatrix = false;
		    update_config = true;
		} else
		    imx_printf( "Invalid option, log <on|off>\r\n" );
	} else
	    imx_printf( "iMatrix logging: %s\r\n", device_config.send_logs_to_imatrix == true ? "Enabled" : "Disabled" );
	if( update_config == true )
	    imatrix_save_config();
}
