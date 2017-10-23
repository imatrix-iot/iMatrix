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

/** @file ISMART_ism43340.c
 *
 *  Created on: October 20, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../cli/interface.h"
#include "../storage.h"
#include "../device/lcb_def.h"
#include "ISMART_ism43340.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define ON_BOARD_LED_RED                WICED_GPIO_12
#define ON_BOARD_LED_GREEN              WICED_GPIO_11

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

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_red_ismart43340( void )
{
    wiced_gpio_init( ON_BOARD_LED_RED, OUTPUT_PUSH_PULL );  // GPIO 3 - Red Led
    imx_update_led_red_status_ismart43340( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_green_ismart43340( void )
{
    wiced_gpio_init( ON_BOARD_LED_GREEN, OUTPUT_PUSH_PULL );  // GPIO 4 - Green Led
    imx_update_led_green_status_ismart43340( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
void imx_init_led_blue_ismart43340( void )
{
    // Add init for Blue
    imx_update_led_blue_status_ismart43340( false );
}

/**
  * @brief update_led_red_status
  * @param  None
  * @retval : None
  */
void imx_update_led_red_status_ismart43340( bool state )
{
    if( state == true ) {
        wiced_gpio_output_high( ON_BOARD_LED_RED );
    } else {
        wiced_gpio_output_low( ON_BOARD_LED_RED );
    }
}
/**
  * @brief update_led_green_status
  * @param  None
  * @retval : None
  */
void imx_update_led_green_status_ismart43340( bool state )
{
    if( state == true ) {
        wiced_gpio_output_high( ON_BOARD_LED_GREEN );
    } else {
        wiced_gpio_output_low( ON_BOARD_LED_GREEN );
    }

}
/**
  * @brief update_led_blue_status
  * @param  None
  * @retval : None
  */
void imx_update_led_blue_status_ismart43340( bool state )
{
    if( state == true ) {
        // wiced_gpio_output_high( ON_BOARD_LED_BLUE );
    } else {
        // wiced_gpio_output_low( ON_BOARD_LED_BLUE );
    }

}
