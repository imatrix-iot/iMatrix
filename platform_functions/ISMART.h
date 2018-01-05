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

/** @file ISMART_rev_D.h
 *
 *  Created on: October 20, 2017
 *      Author: greg.phillips
 *
 */

#ifndef ISMART_REV_D_H_
#define ISMART_REV_D_H_

/*
 *	Defines for
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
void imx_init_led_red_ismart(uint16_t arg);
void imx_init_led_green_ismart(uint16_t arg);
void imx_init_led_blue_ismart(uint16_t arg);
void imx_update_led_red_ismart( bool state );
void imx_update_led_green_ismart( bool state );
void imx_update_led_blue_ismart( bool state );
void imx_init_temp(uint16_t arg);
imx_result_t  imx_sample_temp(uint16_t arg, void *value );

#endif /* ISMART_REV_D_H_ */
