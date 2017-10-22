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
#include "lcb_def.h"
#include "ISMART_LEDS.h"

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
static void init_led_red( void );
static void init_led_green( void );
static void init_led_blue( void );
static void update_led_red_status( imx_led_state_t state );
static void update_led_green_status( imx_led_state_t state );
static void update_led_blue_status( imx_led_state_t state );

/******************************************************
 *               Variable Definitions
 ******************************************************/

led_control_block_t lcb[ IMX_NO_LEDS ] = {
        {
                .led_no = IMX_LED_RED,
                .init_led = init_led_red,
                .update_led_status = update_led_red_status,
                .blink_rate = 0,
                .count  = 0,
                .flash_duration = 0,
                .led_timer_data = 0,
                .in_pair = false,
                .blinking = false,
                .flashing = false,
                .state = false,
        },
        {
                .led_no = IMX_LED_GREEN,
                .init_led = init_led_green,
                .update_led_status = update_led_green_status,
                .blink_rate = 0,
                .count  = 0,
                .flash_duration = 0,
                .led_timer_data = 0,
                .in_pair = false,
                .blinking = false,
                .flashing = false,
                .state = false,
        },
        {
                .led_no = IMX_LED_BLUE,
                .init_led = init_led_blue,
                .update_led_status = update_led_blue_status,
                .blink_rate = 0,
                .count  = 0,
                .flash_duration = 0,
                .led_timer_data = 0,
                .in_pair = false,
                .blinking = false,
                .flashing = false,
                .state = false,
        },
};

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Control the LEDs on ISMART ISM 43340 system
  * @param  LED, Status
  * @retval : None
  */
bool imx_ismart_ism43340_set_led( imx_led_t led, imx_led_state_t mode )
{
    return imx_ismart_set_led( led, mode );

}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
static void init_led_red( void )
{
    wiced_gpio_init( ON_BOARD_LED_RED, OUTPUT_PUSH_PULL );  // GPIO 3 - Red Led
    update_led_red_status( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
static void init_led_green( void )
{
    wiced_gpio_init( ON_BOARD_LED_GREEN, OUTPUT_PUSH_PULL );  // GPIO 4 - Green Led
    update_led_green_status( false );
}
/**
  * @brief init_led_red
  * @param  None
  * @retval : None
  */
static void init_led_blue( void )
{
    // Add init for Blue
    update_led_blue_status( false );
}

/**
  * @brief update_led_red_status
  * @param  None
  * @retval : None
  */
static void update_led_red_status( uint16_t state )
{
    if( state == true ) {
        lcb[ IMX_LED_RED ].state = true;
        wiced_gpio_output_high( ON_BOARD_LED_RED );
    } else {
        lcb[ IMX_LED_RED ].state = false;
        wiced_gpio_output_low( ON_BOARD_LED_RED );
    }
}
/**
  * @brief update_led_green_status
  * @param  None
  * @retval : None
  */
static void update_led_green_status( uint16_t state )
{
    if( state == true ) {
        lcb[ IMX_LED_GREEN ].state = true;
        wiced_gpio_output_high( ON_BOARD_LED_GREEN );
    } else {
        lcb[ IMX_LED_GREEN ].state = false;
        wiced_gpio_output_low( ON_BOARD_LED_GREEN );
    }

}
/**
  * @brief update_led_blue_status
  * @param  None
  * @retval : None
  */
static void update_led_blue_status( uint16_t state )
{
    if( state == true ) {
        lcb[ IMX_LED_BLUE ].state = true;
        // wiced_gpio_output_high( ON_BOARD_LED_GREEN );
    } else {
        lcb[ IMX_LED_BLUE ].state = false;
        // wiced_gpio_output_low( ON_BOARD_LED_GREEN );
    }

}
