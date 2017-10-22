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
 *  This is a general routine to manage LEDs on the iMatrix platform.
 *  This code is called from the actual platform, this is determined by the function defined in the iMatrix Configuration block.
 *
 *  When LEDs are paired in a blinking / flashing operation. The first LED in the name sequence is used for timer data
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../cli/interface.h"
#include "../storage.h"
#include "lcb_def.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MASTER_LED                      0x00
#define SLAVE_LED                       0x01

#define MAX_FLASH_COUNT                 10
#define FLASH_HALF_COUNT                5
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
static timer_handler_t flash_led( void *arg );
static timer_handler_t alt_flash_led( void *arg );
static timer_handler_t blink_led( void *arg );
static timer_handler_t alt_blink_led( void *arg );
static void select_leds( imx_led_t led, imx_led_t *master_led, imx_led_t *slave_led );
/******************************************************
 *               Variable Definitions
 ******************************************************/
static wiced_semaphore_t wiced_led_semaphore;

extern led_control_block_t lcb[ IMX_NO_LEDS ];

static imx_led_t master_slave[ 2 ];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Control the LEDs on ISMART ISM 43340 system
  * @param  LED, Status
  * @retval : None
  */
bool imx_ismart_set_led( imx_led_t led, imx_led_state_t mode )
{
    uint16_t i;
    wiced_result_t wiced_result;
    imx_led_t master_led, slave_led;


    imx_printf( "Setting Led: %u, to state: %u\r\n", (uint16_t) led, (uint16_t) mode );
    /*
     * If blinking/flashing - stop it first
     */
    switch( led ) {
        case IMX_LED_RED :
        case IMX_LED_GREEN :
        case IMX_LED_BLUE :
            if( ( ( lcb[ led ].blinking == true ) || ( lcb[ led ].flashing == true ) ) && ( lcb[ led ].in_pair == false ) ) {
                wiced_result = wiced_rtos_stop_timer( &lcb[ led ].led_timer_data );
                if( wiced_result != WICED_SUCCESS )
                    imx_printf( " Failed to stop LED Timer: %u\r\n", wiced_result );
                lcb[ led ].blinking = false;
            }
            break;
        case IMX_LED_RED_GREEN :
            if( lcb[ IMX_LED_RED ].in_pair == true ) {
                wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_RED ].led_timer_data );
                if( wiced_result != WICED_SUCCESS )
                    imx_printf( " Failed to stop LED Timer: %u\r\n", wiced_result );
                lcb[ IMX_LED_RED ].blinking = false;
            }
            break;
        case IMX_LED_RED_BLUE :
            if( lcb[ IMX_LED_RED ].in_pair == true ) {
                wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_RED ].led_timer_data );
                if( wiced_result != WICED_SUCCESS )
                    imx_printf( " Failed to stop LED Timer: %u\r\n", wiced_result );
                lcb[ IMX_LED_RED ].blinking = false;
            }
            break;
        case IMX_LED_GREEN_BLUE :
            if( lcb[ IMX_LED_GREEN ].in_pair == true ) {
                wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_GREEN ].led_timer_data );
                if( wiced_result != WICED_SUCCESS )
                    imx_printf( " Failed to stop LED Timer: %u\r\n", wiced_result );
                lcb[ IMX_LED_RED ].blinking = false;
            }
            break;
        default :   // Unknown LED - Ignore
            return false;
            break;
    }

    switch( mode ) {
        case IMX_LED_OFF :
            switch( led ) {
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    lcb[ led ].blinking = false;
                    lcb[ led ].flashing = false;
                    lcb[ led ].update_led_status( IMX_LED_OFF );
                    break;
                case IMX_LED_RED_GREEN :
                case IMX_LED_GREEN_RED :
                    if( led == IMX_LED_RED_GREEN )
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_RED ].led_timer_data );
                    else
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_GREEN ].led_timer_data );
                    lcb[ IMX_LED_RED ].blinking = false;
                    lcb[ IMX_LED_RED ].flashing = false;
                    lcb[ IMX_LED_RED ].in_pair = false;
                    lcb[ IMX_LED_RED ].update_led_status( IMX_LED_OFF );
                    lcb[ IMX_LED_GREEN ].blinking = false;
                    lcb[ IMX_LED_GREEN ].flashing = false;
                    lcb[ IMX_LED_GREEN ].in_pair = false;
                    lcb[ IMX_LED_GREEN ].update_led_status( IMX_LED_OFF );
                    break;
                case IMX_LED_RED_BLUE :
                case IMX_LED_BLUE_RED :
                    if( led == IMX_LED_RED_BLUE )
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_RED ].led_timer_data );
                    else
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_BLUE ].led_timer_data );
                    lcb[ IMX_LED_RED ].blinking = false;
                    lcb[ IMX_LED_RED ].flashing = false;
                    lcb[ IMX_LED_RED ].in_pair = false;
                    lcb[ IMX_LED_RED ].update_led_status( IMX_LED_OFF );
                    lcb[ IMX_LED_BLUE ].blinking = false;
                    lcb[ IMX_LED_BLUE ].flashing = false;
                    lcb[ IMX_LED_BLUE ].in_pair = false;
                    lcb[ IMX_LED_BLUE ].update_led_status( IMX_LED_OFF );
                    break;
                case IMX_LED_GREEN_BLUE :
                case IMX_LED_BLUE_GREEN :
                    if( led == IMX_LED_RED_BLUE )
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_GREEN ].led_timer_data );
                    else
                        wiced_result = wiced_rtos_stop_timer( &lcb[ IMX_LED_BLUE ].led_timer_data );
                    lcb[ IMX_LED_GREEN ].blinking = false;
                    lcb[ IMX_LED_GREEN ].flashing = false;
                    lcb[ IMX_LED_GREEN ].in_pair = false;
                    lcb[ IMX_LED_GREEN ].update_led_status( IMX_LED_OFF );
                    lcb[ IMX_LED_BLUE ].blinking = false;
                    lcb[ IMX_LED_BLUE ].flashing = false;
                    lcb[ IMX_LED_BLUE ].in_pair = false;
                    lcb[ IMX_LED_BLUE ].update_led_status( IMX_LED_OFF );
                    break;
                default :   // Unknown LED - Ignore
                    return false;
                    break;
            }
            break;
        case IMX_LED_ON :
            switch( led ) {
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    lcb[ led ].update_led_status( true );
                    break;
                default :   // Unknown LED - Ignore
                    return false;
                    break;
            }
            break;
        default :   // Blinking / Flashing Pair
            switch( led ) {
                /*
                 * Single LEDS
                 */
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    if( ( mode & IMX_LED_FLASH ) == IMX_LED_FLASH ) {
                        /*
                         * Set up to flash
                         */
                        lcb[ led ].count = 0;
                        lcb[ led ].flash_duration = ( (mode  & IMX_LED_BLINK_MASK ) - 1 );
                        lcb[ led ].blink_rate = 1000 / MAX_FLASH_COUNT;
                        wiced_rtos_init_timer( &lcb[ led ].led_timer_data, lcb[ led ].blink_rate, (timer_handler_t ) flash_led, (void *) &lcb[ led ].led_no );
                        lcb[ led ].flashing = true;
                    } else {
                        /*
                         * Set up blink with a duty cycle of 50%
                         */
                        lcb[ led ].blink_rate = 1000 / (mode - 1 );
                        wiced_rtos_init_timer( &lcb[ led ].led_timer_data, lcb[ led ].blink_rate, (timer_handler_t ) blink_led, (void *) &lcb[ led ].led_no );
                        lcb[ led ].blinking = true;
                    }
                    wiced_result = wiced_rtos_start_timer( &lcb[ led ].led_timer_data );
                    if( wiced_result != WICED_SUCCESS )
                        imx_printf( " Failed to start LED Timer: %u\r\n", wiced_result );
                    break;
                /*
                 * Pair of LEDS
                 */
                case IMX_LED_RED_GREEN :
                case IMX_LED_RED_BLUE :
                case IMX_LED_GREEN_RED :
                case IMX_LED_GREEN_BLUE :
                case IMX_LED_BLUE_RED :
                case IMX_LED_BLUE_GREEN :
                    /*
                     * Select the master and slave LEDS - master will be used for timer controls
                     */
                    master_led = IMX_LED_RED;
                    slave_led = IMX_LED_GREEN;
                    select_leds( led, &master_led, &slave_led );
                    master_slave[ MASTER_LED ] = master_led;
                    master_slave[ SLAVE_LED ] = slave_led;
                    if( ( mode & IMX_LED_FLASH ) == IMX_LED_FLASH ) {
                        /*
                         * Set up to flash
                         */
                        lcb[ master_led ].count = 0;
                        lcb[ master_led ].flash_duration = ( (mode  & IMX_LED_BLINK_MASK ) - 1 );
                        lcb[ master_led ].blink_rate = 1000 / MAX_FLASH_COUNT;
                        wiced_rtos_init_timer( &lcb[ master_led ].led_timer_data, lcb[ master_led ].blink_rate, (timer_handler_t ) alt_flash_led, (void *) &lcb[ led ].led_no );
                        lcb[ master_led ].flashing = true;
                    } else {
                        /*
                         * Set up to Blink two LEDs alternating. Blink with a duty cycle of 50% each
                         */
                        lcb[ master_led ].blink_rate = 1000 / (mode - 1 );
                        wiced_rtos_init_timer( &lcb[ master_led ].led_timer_data, lcb[ master_led ].blink_rate, (timer_handler_t ) alt_blink_led, (void *) &lcb[ led ].led_no );
                        lcb[ master_led ].blinking = true;
                    }
                    lcb[ master_led ].in_pair = true;
                    lcb[ slave_led ].in_pair = true;
                    break;
                default :   // Unknown LED - Ignore
                    return false;
                    break;
            }
            break;
        case IMX_LED_INIT :
            /*
             * Start Off
             */
            for( i = 0; i < IMX_NO_LEDS; i++ )
                (*lcb[ i ].init_led)();
            /*
             * Set up the timers used for LEDS
             */
            wiced_result = wiced_rtos_init_semaphore( &wiced_led_semaphore );    // to prevent any thread issues
            if( wiced_result != WICED_SUCCESS ) {
                imx_printf( "Failed to initialize Semaphore: %u\r\n", wiced_result );
            }
            break;
    }
    return true;
}
/**
  * @brief blink LED
  * @param  None
  * @retval : None
  */
static timer_handler_t blink_led( void *arg )
{
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    lcb[ led ].state = !lcb[ led ].state;
    (*lcb[ led ].update_led_status)( lcb[ led ].state );

    return WICED_SUCCESS;
}
/**
  * @brief blink LED
  * @param  None
  * @retval : None
  */
static timer_handler_t alt_blink_led( void *arg )
{
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED_GREEN ) && ( led != IMX_LED_RED_BLUE ) && ( led != IMX_LED_GREEN_RED ) &&
        ( led != IMX_LED_GREEN_BLUE ) && ( led != IMX_LED_BLUE_RED ) && ( led != IMX_LED_BLUE_GREEN ) )
        return WICED_SUCCESS;   // Ignore this

    lcb[ master_slave[ MASTER_LED ] ].state = !lcb[ master_slave[ MASTER_LED ] ].state;
    (*lcb[master_slave[ MASTER_LED ] ].update_led_status)( lcb[ master_slave[ MASTER_LED ] ].state );

    lcb[ master_slave[ SLAVE_LED ] ].state = !lcb[ master_slave[ SLAVE_LED ] ].state;
    (*lcb[master_slave[ SLAVE_LED ] ].update_led_status)( lcb[ master_slave[ SLAVE_LED ] ].state );

    return WICED_SUCCESS;
}
/**
  * @brief flash LED
  * @param  None
  * @retval : None
  */
static timer_handler_t flash_led( void *arg )
{
    bool update;
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    update = false;
    if( lcb[ led ].count < lcb[ led ].flash_duration ) {
        if( lcb[ led ].state == true ) {
            lcb[ led ].state = false;
            update = true;
        }
    } else {
        if( lcb[ led ].state == false ) {
            lcb[ led ].state = true;
            update = true;
        }
    }
    if( update == true ) {
        (*lcb[ led ].update_led_status)( lcb[ led ].state );
    }
    lcb[ led ].count += 1;
    if( lcb[ led ].count >= MAX_FLASH_COUNT )
        lcb[ led ].count = 0;

    return WICED_SUCCESS;
}

/**
  * @brief flash LED
  * @param  None
  * @retval : None
  */
static timer_handler_t alt_flash_led( void *arg )
{
    bool update;
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED_GREEN ) && ( led != IMX_LED_RED_BLUE ) && ( led != IMX_LED_GREEN_RED ) &&
        ( led != IMX_LED_GREEN_BLUE ) && ( led != IMX_LED_BLUE_RED ) && ( led != IMX_LED_BLUE_GREEN ) )
        return WICED_SUCCESS;   // Ignore this

    update = false;
    if( lcb[ master_slave[ MASTER_LED ] ].count < FLASH_HALF_COUNT ) {
        if( lcb[ master_slave[ MASTER_LED ] ].state == false ) {
            lcb[ master_slave[ MASTER_LED ] ].state = true;
            lcb[ master_slave[ SLAVE_LED ] ].state = false;
            update = true;
        }
    } else if( lcb[ master_slave[ MASTER_LED ] ].count < FLASH_HALF_COUNT + lcb[ master_slave[ MASTER_LED ] ].flash_duration ) {
        if( lcb[ master_slave[ MASTER_LED ] ].state == true ) {
            lcb[ master_slave[ MASTER_LED ] ].state = false;
            lcb[ master_slave[ SLAVE_LED ] ].state = true;
            update = true;
        }
    } else {
        if( lcb[ master_slave[ SLAVE_LED ] ].state == true ) {
            lcb[ master_slave[ MASTER_LED ] ].state = false;
            lcb[ master_slave[ SLAVE_LED ] ].state = false;
            update = true;
        }
    }

    if( update == true ) {
        (*lcb[ master_slave[ MASTER_LED ] ].update_led_status)( lcb[ master_slave[ MASTER_LED ] ].state );
        (*lcb[ master_slave[ SLAVE_LED ] ].update_led_status)( lcb[ master_slave[ SLAVE_LED ] ].state );
    }
    lcb[ master_slave[ MASTER_LED ] ].count += 1;
    if( lcb[ master_slave[ MASTER_LED ] ].count >= MAX_FLASH_COUNT )
        lcb[ master_slave[ MASTER_LED ] ].count = 0;

    return WICED_SUCCESS;
}

extern char *led_names[];

/**
  * @brief print_led_status
  * @param  None
  * @retval : None
  */
void print_led_status( void )
{
    uint16_t i;

    for( i = 0; i < IMX_NO_LEDS; i++ ) {
        cli_print( "Led Status: %s: ", led_names[ i ] );
        if( lcb[i ].blinking == true )
            cli_print( "Blinking every %umS", lcb[ i ].blink_rate );
        else if( lcb[i ].flashing == true )
            cli_print( "Flashing ON %umS", ( lcb[ i ].blink_rate & IMX_LED_BLINK_MASK ) );
        else if( lcb[ i ].state == true )
            cli_print( "On" );
        else
            cli_print( "Off" );
        cli_print( "\r\n" );

    }
}
static void select_leds( imx_led_t led, imx_led_t *master_led, imx_led_t *slave_led )
{
    if( ( led == IMX_LED_RED_GREEN ) || ( IMX_LED_RED_BLUE  ) ) {
        *master_led = IMX_LED_RED;
        if( led == IMX_LED_RED_GREEN )
            *slave_led = IMX_LED_GREEN;
        else
            *slave_led = IMX_LED_BLUE;
    } else if( ( led == IMX_LED_GREEN_RED ) || ( IMX_LED_GREEN_BLUE  ) ) {
        *master_led = IMX_LED_GREEN;
        if( led == IMX_LED_GREEN_RED )
            *slave_led = IMX_LED_RED;
        else
            *slave_led = IMX_LED_BLUE;
    } else {
        *master_led = IMX_LED_BLUE;
        if( led == IMX_LED_BLUE_RED )
            *slave_led = IMX_LED_RED;
        else
            *slave_led = IMX_LED_GREEN;
    }

}
