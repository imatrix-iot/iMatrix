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
 * hal_leds.c
 *
 *  Call the Host App to set the LED.
 *
 *  If the function is non null then call it with the passed parameters
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "../cli/interface.h"
#include "../storage.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define LED_TEST_TIME       5000        // mS
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

const char *led_names[ IMX_NO_LED_COMBINATIONS ] = {
        "Red",              // IMX_LED_RED = 0,
        "Green",            // IMX_LED_GREEN,
        "Blue",             // IMX_LED_BLUE,
        "Red & Green",      // IMX_LED_RED_GREEN,
        "Red & Blue",       // IMX_LED_RED_BLUE,
        "Green & Red",      // IMX_LED_GREEN_RED,
        "Green & Blue",     // IMX_LED_GREEN_BLUE,
        "Blue & Red",       // IMX_LED_BLUE_RED,
        "Blue & Green",     // IMX_LED_BLUE_GREEN,
};

extern imx_imatrix_init_config_t imatrix_init_config;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief set a LED state
  * @param  None
  * @retval : None
  */

bool set_host_led( imx_led_t led, imx_led_state_t value )
{
    if( imatrix_init_config.set_led != NULL ) {
        (*imatrix_init_config.set_led)( led, value );
        return true;
    } else
        return false;
}
/**
  * @brief Print the status of the device
  * @param  None
  * @retval : None
  */
void cli_set_led( uint16_t arg)
{
    UNUSED_PARAMETER(arg);
    char *token, *foo;
    imx_led_t led, i;
    imx_led_state_t rate;

    token = strtok(NULL, " " );
    if( token ) {
        if( strcmp( token, "test" ) == 0 ) {
            /*
             * Do a sequence of tests for to show LED functionality
             */
            set_host_led( IMX_LED_RED, IMX_LED_OFF );
            set_host_led( IMX_LED_GREEN, IMX_LED_OFF );
            set_host_led( IMX_LED_BLUE, IMX_LED_OFF );
            /*
            for( i = 0; i < IMX_NO_LEDS; i++ ) {
                imx_printf( "Setting %s to ON..", led_names[ i ] );
                set_host_led( i, IMX_LED_ON );
                wiced_rtos_delay_milliseconds( LED_TEST_TIME );
                set_host_led( i, IMX_LED_OFF );
                imx_printf( "OFF\r\n" );
            }
            for( i = 0; i < IMX_NO_LEDS; i++ ) {
                imx_printf( "Setting %s to Blinking..", led_names[ i ] );
                set_host_led( i, IMX_LED_BLINK_5 );
                wiced_rtos_delay_milliseconds( LED_TEST_TIME );
                set_host_led( i, IMX_LED_OFF );
                imx_printf( "OFF\r\n" );
            }
            for( i = 0; i < IMX_NO_LEDS; i++ ) {
                imx_printf( "Setting %s to Flashing..", led_names[ i ] );
                set_host_led( i, IMX_LED_FLASH | IMX_LED_FLASH_2 | IMX_LED_BLINK_2 );
                wiced_rtos_delay_milliseconds( LED_TEST_TIME );
                set_host_led( i, IMX_LED_OFF );
                imx_printf( "OFF\r\n" );
            }
            */
            for( i = IMX_LED_RED_GREEN; i < IMX_NO_LED_COMBINATIONS; i++ ) {
                imx_printf( "Setting %s to Alternating Blinking..", led_names[ i ] );
                set_host_led( i, IMX_LED_BLINK_5 );
                wiced_rtos_delay_milliseconds( LED_TEST_TIME );
                set_host_led( i, IMX_LED_OFF );
                imx_printf( "OFF\r\n" );
            }
            for( i = IMX_LED_RED_GREEN; i < IMX_NO_LED_COMBINATIONS; i++ ) {
                imx_printf( "Setting %s to Alternating Flashing..", led_names[ i ] );
                set_host_led( i, IMX_LED_FLASH | IMX_LED_FLASH_2 | IMX_LED_BLINK_2 );
                wiced_rtos_delay_milliseconds( LED_TEST_TIME );
                set_host_led( i, IMX_LED_OFF );
                imx_printf( "OFF\r\n" );
            }
            return;
        } else if( strncmp( token, "0x", 2 ) == 0 )
            led = (imx_led_t) strtoul( &token[ 2 ], &foo, 16 );
        else if( strcmp( token, "red" ) == 0 )
            led = IMX_LED_RED;
        else if( strcmp( token, "green" ) == 0 )
            led = IMX_LED_GREEN;
        else if( strcmp( token, "blue" ) == 0 )
            led = IMX_LED_BLUE;
        else
            led = (imx_led_t) strtoul( token, &foo, 10 );
        token = strtok(NULL, " " );
        if( token ) {
            if( strcmp( token, "on" ) == 0 ) {
                rate = IMX_LED_ON;
                set_host_led( led, rate );
            } else if( strcmp( token, "off" ) == 0 ) {
                rate = IMX_LED_OFF;
                set_host_led( led, rate );
            } else if( strcmp( token, "blink" ) == 0 ) {
                token = strtok(NULL, " " );
                if( token ) {
                    if( strncmp( token, "0x", 2 ) == 0 )
                        rate = (imx_led_state_t) strtoul( &token[ 2 ], &foo, 16 );
                    else
                        rate = (imx_led_state_t) strtoul( token, &foo, 10 ) + 1;
                    set_host_led( led, rate );
                } else
                    cli_print( "Need to specify blink rate\r\n" );
            } else
                cli_print( "Need to specify on / off  / blink\r\n" );
        } else
            cli_print( "Need to specify state\r\n" );
    } else
        cli_print( "Need to specify LED - 0 / red or 1 / green\r\n" );
}
