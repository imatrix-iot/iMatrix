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

/** @file imx_interface.h
 *
 *  Created on: October 26, 2017
 *      Author: greg.phillips
 *
 */

#ifndef IMX_INTERFACE_H_
#define IMX_INTERFACE_H_

/*
 *	Defines for interfacing to IMX controls/sensor system
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
imx_status_t imx_set_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value );
imx_status_t imx_get_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value );
imx_status_t imx_set_sensor( uint16_t sensor_entry, void *value );
imx_status_t imx_get_sensor( uint16_t sensor_entry, void *value );
imx_status_t imx_set_control( uint16_t control_entry, void *value );
imx_status_t imx_get_control( uint16_t control_entry, void *value );
bool imx_parse_value( imx_peripheral_type_t type, uint16_t entry, char *string, data_32_t *value );
#endif /* IMX_INTERFCE_H_ */
