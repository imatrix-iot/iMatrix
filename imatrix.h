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

/** @file imatirx.h
 *
 *  Created on: October 8, 2017
 *      Author: greg.phillips
 *
 */

#ifndef IMX_IMATRIX_H_
#define IMX_IMATRIX_H_

#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "common.h"
/*
 *	Defines for iMatrix API
 *
 */
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/*
 *  Define the number of sensors and controls in the system
 *
 */
#define IMX_MAX_NO_SENSORS              ( 3 + 32 )      // 3 Integrated Wi Fi Channel, RSSI, Noise - 32 User Defined
#define IMX_MAX_NO_CONTROLS             ( 0 + 8 )       // 8 User Defined
/*
 *  Define the number of Smart Arduino controls and sensors in the system
 *
 */
#define IMX_MAX_ARDUINO_CONTROLS        ( 8 )
#define IMX_MAX_ARDUINO_SENSORS         ( 8 )

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
/*
 * Start up and run functions
 */
imx_status_t imx_init( imx_imatrix_init_config_t *init_config, bool override_config, bool run_in_background );
imx_status_t imx_process(void);
imx_status_t imx_deinit(void);
/*
 * System Status
 */
bool imx_setup_mode(void);
/*
 * Configuration management
 */
void *imx_get_config_current_address(void);
wiced_result_t imx_save_config( void *config, uint16_t config_size );
/*
 * Console I/O
 */
uint16_t imx_get_ch( char *ch );
void imx_printf( char *format, ... );
/*
 * Support functions from host
 */
void imx_update_led_red_status( uint16_t status );
#endif /* IMX_IMATRIX_H_ */
