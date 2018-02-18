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
 *  The Mode for blinking and flashing is stored in the top nibble of the word - only one be is expected to be set and if multiple bits are set then the first set is the priority
 *
 *  IMX_LED_BLINK_1                 0x1000  // Single Blinking action one or more LEDs
 *  IMX_LED_BLINK_2                 0x2000  // Dual Alternating Blinking
 *  IMX_LED_FLASH                   0x4000  // Indicate this is a flash, this is a 1 Second event For Dual LEDs First LED ON for BLINK 1 second is on for BLINK 2 - off for remainder of 1 Second
 *
 *  When setting for Blinking / Flashing LEDs
 *      The two LEDs are defined as Master and Slave.
 *
 *  When setting for Flashing (This is a 1 Second repeating event) the Master will be on for BLINK_2 count and the Slave will be ON for the BLINK 2 count
 *  the LEDS will be OFF for the remainder of the 1 Second
 *
 *  The Blink / Flash rates are stored in two sections
 *      Blink/Flash 1 0x00F - is the duration in counts of 100 mS
 *      BLink/Flash 1 0xF00 - Right shifted 8 bits is the duration in counts of 100 mS
 *
 * imx_printf( "Setting Alternate blinking: Master: %u & Slave: %u @%u mSec\r\n", (uint16_t) master_led, (uint16_t) slave_led, lcb[ master_led ].blink_rate );
 *
 *  When LEDs are paired in a blinking / flashing operation. The first LED in the name sequence (master) is used for interrupt timer data storage
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

#define LED_TIMER_100MS                 100
#define LED_TIMER_500MS                 5

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
static void void_led_function(uint16_t arg);
static imx_result_t void_led_update_function( uint16_t arg, void *value );
static void select_leds( imx_led_t led, imx_led_t *master_led, imx_led_t *slave_led );
/******************************************************
 *               Variable Definitions
 ******************************************************/
const imx_led_t led_options[ IMX_NO_LED_COMBINATIONS ] = {
        IMX_LED_RED,
        IMX_LED_GREEN,
        IMX_LED_BLUE,
        IMX_LED_RED_GREEN,
        IMX_LED_RED_BLUE,
        IMX_LED_GREEN_RED,
        IMX_LED_GREEN_BLUE,
        IMX_LED_BLUE_RED,
        IMX_LED_BLUE_GREEN,
};
static wiced_semaphore_t wiced_led_semaphore;

led_control_block_t lcb[ IMX_NO_LEDS ] = {
        {
                .led_no = IMX_LED_RED,
                .init_led = void_led_function,
                .update_led_status = void_led_update_function,
                .blink_1_count = 0,
                .blink_2_count = 0,
                .count  = 0,
                .flash_count = 0,
                .led_timer_data = { 0 },
                .in_pair = false,
                .blinking = false,
                .flashing = false,
                .state = false,
        },
        {
                .led_no = IMX_LED_GREEN,
                .init_led = void_led_function,
                .update_led_status = void_led_update_function,
                .blink_1_count = 0,
                .blink_2_count = 0,
                .count  = 0,
                .flash_count = 0,
                .led_timer_data = { 0 },
                .in_pair = false,
                .blinking = false,
                .flashing = false,
                .state = false,
        },
        {
                .led_no = IMX_LED_BLUE,
                .init_led = void_led_function,
                .update_led_status = void_led_update_function,
                .blink_1_count = 0,
                .blink_2_count = 0,
                .count  = 0,
                .flash_count = 0,
                .led_timer_data = { 0 },
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
bool imx_set_led( imx_led_t led, imx_led_state_t state, uint16_t mode_details )
{
    bool start_timer;
    uint16_t i;
    uint32_t value;
    wiced_result_t wiced_result;
    imx_led_t master_led, slave_led;

//    imx_printf( "Setting Led: %u, to state: %u, Mode: 0x%04x\r\n", (uint16_t) led, (uint16_t) state, mode_details );
    /*
     * If blinking/flashing - stop it first
     */
    master_led = IMX_LED_RED;   // Just to prevent warnings
    slave_led = IMX_LED_GREEN;
    start_timer = false;

    switch( state ) {
        case IMX_LED_ALL_OFF :
            for( i = 0; i < IMX_NO_LEDS; i++ ) {
                /*
                 * If this is doing a timer function - disable
                 */
                if( ( lcb[ i ].blinking == true ) || ( lcb[ i ].flashing == true ) ) {
                    wiced_result = wiced_rtos_stop_timer( &lcb[ i ].led_timer_data );
                    if( wiced_result != WICED_SUCCESS )
                        imx_printf( " Failed to stop LED: %u Timer: %u\r\n", led, wiced_result );
                }
                lcb[ i ].blinking = false;
                lcb[ i ].flashing = false;
                lcb[ i ].state = false;
                lcb[ i ].in_pair = false;
                lcb[ i ].count = 0;
                value = false;
                lcb[ i ].update_led_status( 0, &value );
            }
            break;
        case IMX_LED_OFF :
            switch( led ) {
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    /*
                     * If this is doing a timer function - disable
                     */
                    if( ( lcb[ led ].blinking == true ) || ( lcb[ led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", led, wiced_result );
                    }
                    lcb[ led ].blinking = false;
                    lcb[ led ].flashing = false;
                    lcb[ led ].in_pair = false;
                    lcb[ led ].state = false;
                    lcb[ led ].count = 0;
                    value = false;
                    lcb[ led ].update_led_status( 0, &value );
                    break;
                case IMX_LED_RED_GREEN :
                case IMX_LED_RED_BLUE :
                case IMX_LED_GREEN_RED :
                case IMX_LED_GREEN_BLUE :
                case IMX_LED_BLUE_RED :
                case IMX_LED_BLUE_GREEN :
                    /*
                     * Select the master and slave LEDS - master will be used for timer controls - set known defaults.
                     */
                    select_leds( led, &master_led, &slave_led );
                    if( ( lcb[ master_led ].blinking == true ) || ( lcb[ master_led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ master_led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", master_led, wiced_result );
                    }
                    lcb[ master_led ].blinking = false;
                    lcb[ master_led ].flashing = false;
                    lcb[ master_led ].state = false;
                    lcb[ master_led ].count = 0;
                    lcb[ master_led ].in_pair = false;
                    lcb[ slave_led ].blinking = false;
                    lcb[ slave_led ].flashing = false;
                    lcb[ slave_led ].state = false;
                    lcb[ slave_led ].count = 0;
                    lcb[ slave_led ].in_pair = false;
                    value = false;
                    lcb[ master_led ].update_led_status( 0, &value );
                    value = false;
                    lcb[ slave_led ].update_led_status( 0, &value);
                    break;
                default :   // Unknown LED - Ignore
                    return false;
                    break;
            }
            break;
        case IMX_LED_ON :   // Single or a pair
            switch( led ) {
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    /*
                     * If this is doing a timer function - disable
                     */
                    if( ( lcb[ led ].blinking == true ) || ( lcb[ led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", led, wiced_result );
                    }
                    lcb[ led ].state = true;
                    value = true;
                    lcb[ led ].update_led_status( 0, &value );
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
                    select_leds( led, &master_led, &slave_led );
                    /*
                     * If this is doing a timer function - disable
                     */
                    if( ( lcb[ master_led ].blinking == true ) || ( lcb[ master_led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ master_led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", master_led, wiced_result );
                    }
                    value = true;
                    lcb[ master_led ].update_led_status( 0, &value );
                    value = true;
                    lcb[ slave_led ].update_led_status( 0, &value );
                    break;
                default :   // Unknown LED - Ignore
                    return false;
                    break;
            }
            break;
        case IMX_LED_OTHER :
        default :   // Blinking / Flashing
            switch( led ) {
                /*
                 * Single LEDS
                 */
                case IMX_LED_RED :
                case IMX_LED_GREEN :
                case IMX_LED_BLUE :
                    /*
                     * If this is doing a timer function - disable
                     */
                    if( ( lcb[ led ].blinking == true ) || ( lcb[ led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", led, wiced_result );
                    }
                    lcb[ led ].state = false;
                    lcb[ led ].count = 0;
                    lcb[ led ].in_pair = false;
                    if( ( mode_details & IMX_LED_BLINK_1 ) == IMX_LED_BLINK_1 ) {
                        /*
                         * Set up to Blink
                         */
                        lcb[ led ].blink_1_count = ( mode_details & IMX_LED_BLINK_1_MASK );
                        lcb[ led ].blink_2_count = 0;
                        lcb[ led ].flashing = false;
                        lcb[ led ].blinking = true;
                        /*
                         * Initialize Timer setup
                         */
                        wiced_rtos_init_timer( &lcb[ led ].led_timer_data, (uint32_t) LED_TIMER_100MS, (timer_handler_t) blink_led, (void *) &led_options[ led ] );
                        start_timer = true;
                    } else if( ( mode_details & IMX_LED_BLINK_2 ) == IMX_LED_BLINK_2 ) {
                        /*
                         * Ignore this as single led called with dual LED Blinking
                         */

                    } else if( ( mode_details & IMX_LED_FLASH ) == IMX_LED_FLASH ) {
                        /*
                         * Set up to Flash
                         */
                        lcb[ led ].blink_1_count = ( mode_details & IMX_LED_BLINK_1_MASK );
                        lcb[ led ].blink_2_count = 0;
                        lcb[ led ].flash_count = 10 * ( ( mode_details & IMX_LED_FLASH_MASK ) >> 8 );
                        lcb[ led ].flashing = true;
                        lcb[ led ].blinking = false;
                        wiced_rtos_init_timer( &lcb[ led ].led_timer_data, (uint32_t) LED_TIMER_100MS, (timer_handler_t) flash_led, (void *) &led_options[ led ] );
                        start_timer = true;
                    }
                    if( start_timer == true ) {
                        wiced_result = wiced_rtos_start_timer( &lcb[ led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to start LED:%u Timer: %u\r\n", master_led, wiced_result );
                    }
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
                    select_leds( led, &master_led, &slave_led );
                    /*
                     * If this is doing a timer function - disable
                     */
                    if( ( lcb[ master_led ].blinking == true ) || ( lcb[ master_led ].flashing == true ) ) {
                        wiced_result = wiced_rtos_stop_timer( &lcb[ master_led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to stop LED: %u Timer: %u\r\n", master_led, wiced_result );
                    }
                    lcb[ master_led ].state = false;
                    lcb[ master_led ].count = 0;
                    lcb[ slave_led ].state = false;
                    lcb[ master_led ].count = 0;
                    lcb[ master_led ].in_pair = true;
                    lcb[ master_led ].pair = slave_led;
                    if( ( mode_details & IMX_LED_BLINK_1 ) == IMX_LED_BLINK_1 ) {
                        /*
                         * Set up to Blink as a pair
                         */
                        lcb[ master_led ].blink_1_count = ( mode_details & IMX_LED_BLINK_1_MASK );
                        lcb[ master_led ].blink_2_count = 0;
                        lcb[ master_led ].flashing = false;
                        lcb[ master_led ].blinking = true;
                        lcb[ master_led ].in_pair = true;
                        lcb[ master_led ].pair = slave_led;
                        /*
                         * Initialize Timer setup
                         */
                        wiced_rtos_init_timer( &lcb[ master_led ].led_timer_data, (uint32_t) LED_TIMER_100MS, (timer_handler_t) blink_led, (void *) &led_options[ master_led ] );
                        start_timer = true;
                    } else if( ( mode_details & IMX_LED_BLINK_2 ) == IMX_LED_BLINK_2 ) {
                        /*
                         * Set up to Blink two LEDs independently
                         */
                        lcb[ master_led ].blink_1_count = ( mode_details & IMX_LED_BLINK_1_MASK );
                        lcb[ master_led ].blink_2_count = ( ( mode_details & IMX_LED_BLINK_2_MASK ) >> 4 );
                        lcb[ master_led ].state = false;
                        lcb[ slave_led ].state = false;
                        lcb[ master_led ].flashing = false;
                        lcb[ master_led ].blinking = true;
                        lcb[ master_led ].in_pair = true;
                        lcb[ master_led ].pair = slave_led;
                        lcb[ master_led ].count = 0;
                        /*
                         * Initialize Timer setup
                         */
//                        imx_printf( "Setting Alternate blinking: Master: %u & Slave: %u @%u00 mSec master, %u00 mSec slave\r\n", (uint16_t) master_led, (uint16_t) slave_led, lcb[ master_led ].blink_1_count, lcb[ master_led ].blink_2_count );
                        wiced_rtos_init_timer( &lcb[ master_led ].led_timer_data, (uint32_t) LED_TIMER_100MS, (timer_handler_t) alt_blink_led, (void *) &led_options[ master_led ] );
                        start_timer = true;
                    } else if( ( mode_details & IMX_LED_FLASH ) == IMX_LED_FLASH ) {
                        lcb[ master_led ].blink_1_count = ( mode_details & IMX_LED_BLINK_1_MASK );
                        lcb[ master_led ].blink_2_count = ( ( mode_details & IMX_LED_BLINK_2_MASK ) >> 4 );       // Only unused for alternate mode
                        lcb[ master_led ].flash_count = 10 * ( ( mode_details & IMX_LED_FLASH_MASK ) >> 8 );
                        lcb[ master_led ].state = false;
                        lcb[ slave_led ].state = false;
                        lcb[ master_led ].flashing = true;
                        lcb[ master_led ].blinking = false;
                        lcb[ master_led ].in_pair = true;
                        lcb[ master_led ].pair = slave_led;
                        lcb[ master_led ].count = 0;
                        /*
                         * Initialize Timer setup
                         */
//                        imx_printf( "Setting Alternate flash( %u mSec Period): Master: %u & Slave: %u @%u00 mSec master, %u00 mSec slave\r\n",
//                                lcb[ master_led ].flash_count * 100, (uint16_t) master_led, (uint16_t) slave_led, lcb[ master_led ].blink_1_count, lcb[ master_led ].blink_2_count );
                        wiced_rtos_init_timer( &lcb[ master_led ].led_timer_data, (uint32_t) LED_TIMER_100MS, (timer_handler_t) alt_flash_led, (void *) &led_options[ master_led ] );
                        start_timer = true;
                    }
                    if( start_timer == true ) {
                        wiced_result = wiced_rtos_start_timer( &lcb[ master_led ].led_timer_data );
                        if( wiced_result != WICED_SUCCESS )
                            imx_printf( " Failed to start LED:%u Timer: %u\r\n", master_led, wiced_result );
                    }
                    break;
                default :   // Unknown LED - Ignore
                    imx_printf( "Unknown LED: %u\r\n", led );
                    return false;
                    break;
            }
            break;
        case IMX_LED_INIT :
            /*
             * Start Off
             */
//            imx_printf( "Initializing Product LEDs\r\n" );
            for( i = 0; i < IMX_NO_LEDS; i++ ) {
//                imx_printf( "Initializing LED %u\r\n", i );
                if( lcb[ i ].init_led != NULL ) {
//                   imx_printf( "Calling init for LED %u\r\n", i );
                    (*lcb[ i ].init_led)( i );
                }
            }
//            imx_printf( "Done Initializing LEDs\r\n" );
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

bool imx_get_led_state( imx_led_t led )
{
    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return false;
    else
        return( lcb[ led ].state );
}
/**
  * @brief Initialize LED functions from Host provided entries
  * @param  LED functions from Host
  * @retval : None
  */
void imx_init_led_functions( imx_led_functions_t *led_functions )
{
    uint16_t i;

    imx_printf( "Setting up LED Functions\r\n" );
    for( i = 0; i < IMX_NO_LEDS; i++ ) {
        if( led_functions[ i ].init_led != NULL )
            lcb[ i ].init_led = led_functions[ i ].init_led;
        else
            lcb[ i ].init_led = NULL;
        if( led_functions[ i ].set_led != NULL )
            lcb[ i ].update_led_status = led_functions[ i ].set_led;
        else
            lcb[ i ].update_led_status = NULL;
    }
    imx_set_led( 0, IMX_LED_INIT, 0 );
    imx_printf( "LED Functions set & Initialized\r\n" );
}

/**
  * @brief Void Dummy filler function for LED handler
  * @param  None
  * @retval : false
  */
static void void_led_function(uint16_t arg)
{
    UNUSED_PARAMETER(arg);
    /*
     * Do Nothing :)
     */
    return;
}
/**
  * @brief Void Dummy filler function for LED handler
  * @param  None
  * @retval : false
  */
static imx_result_t void_led_update_function( uint16_t arg, void *value )
{
    UNUSED_PARAMETER(arg);
    UNUSED_PARAMETER(value);
    /*
     * Do Nothing :)
     */
    return IMATRIX_SUCCESS;
}
/**
  * @brief blink LED
  * @param  None
  * @retval : None
  */
static timer_handler_t blink_led( void *arg )
{
    imx_led_t led;
    uint32_t value;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    lcb[ led ].state = !lcb[ led ].state;
    value = lcb[ led ].state;
    (*lcb[ led ].update_led_status)( 0, &value );
    if( lcb[ led ].in_pair == true ) {
        lcb[ lcb[ led ].pair ].state = !lcb[ lcb[ led ].pair ].state;
        value = lcb[ lcb[ led ].pair ].state;
        (*lcb[ lcb[ led ].pair ].update_led_status)( 0, &value );
    }
    return WICED_SUCCESS;
}
/**
  * @brief blink LED
  * @param  master LED
  * @retval : None
  */
static timer_handler_t alt_blink_led( void *arg )
{
    bool update;
    uint32_t value;
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    update = false;
    if( lcb[ led ].count < lcb[ led ].blink_1_count) {       // Master On Period
        lcb[ led  ].count += 1;
        if( lcb[ led ].state == false ) {
            lcb[ led ].state = true;
            lcb[ lcb[ led ].pair ].state = false;
            update = true;
        }
    } else if( lcb[ led ].count < ( lcb[ led ].blink_1_count + lcb[ led ].blink_2_count ) ) {     // Slave On Period
        lcb[ led ].count += 1;
        if( lcb[ led ].state == true ) {
            lcb[ led ].state = false;
            if( lcb[ led ].in_pair == true ) {          // Should be :)
                lcb[ lcb[ led ].pair ].state = true;
            }
            update = true;
        }
    } else {
        lcb[ led ].count = 0;    // Start again
    }

    if( update == true ) {
        value = lcb[ led ].state;
        (*lcb[ led ].update_led_status)( 0, &value );
        if( lcb[ led ].in_pair == true ) {          // Should be
            value = lcb[ lcb[ led ].pair ].state;
            (*lcb[ lcb[ led ].pair ].update_led_status)( 0, &value );
        }
    }

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
    uint32_t value;
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    update = false;
    if( lcb[ led ].count < lcb[ led ].blink_1_count ) {
        lcb[ led ].count += 1;
        if( lcb[ led ].state == false ) {
            lcb[ led ].state = true;
            if( lcb[ led ].in_pair == true ) {
                lcb[ lcb[ led ].pair ].state = true;
            }
            update = true;
        }
    } else if( lcb[ led ].count < lcb[ led ].flash_count ){
        lcb[ led ].count += 1;
        if( lcb[ led ].in_pair == true ) {
            if( lcb[ lcb[ led ].pair ].state == true ) {
                lcb[ lcb[ led ].pair ].state = false;       // turn off pair
                lcb[ led ].state = false;
                update = true;
            }
        } else {
            if( lcb[ led ].state == true ) {
                lcb[ led ].state = false;
                update = true;
            }
        }
    } else {    // End of flash - start again
        lcb[ led ].count = 0;
    }
    if( update == true ) {
        value = lcb[ led ].state;
        (*lcb[ led ].update_led_status)( 0, &value );
        if( lcb[ led ].in_pair == true ) {
            value = lcb[ lcb[ led ].pair ].state;
            (*lcb[ lcb[ led ].pair ].update_led_status)( 0, &value );
        }
    }

    return WICED_SUCCESS;
}

/**
  * @brief Alternately flash LED
  * @param  None
  * @retval : None
  */
static timer_handler_t alt_flash_led( void *arg )
{
    bool update;
    uint32_t value;
    imx_led_t led;

    memcpy( &led, arg, sizeof( imx_led_t ) );

    if( ( led != IMX_LED_RED ) && ( led != IMX_LED_GREEN ) && ( led != IMX_LED_BLUE ) )
        return WICED_SUCCESS;   // Ignore this

    update = false;
    if( lcb[ led ].count < lcb[ led ].blink_1_count ) {
        lcb[ led ].count += 1;
        if( lcb[ led ].state == false ) {                    // Start with Master on
            lcb[ led ].state = true;
            if( lcb[ led ].in_pair == true ) {
                lcb[ lcb[ led ].pair ].state = false;
            }
            update = true;
        }
    } else if( lcb[ led ].count < (lcb[ led ].blink_1_count + lcb[ led ].blink_2_count ) ) {
        lcb[ led ].count += 1;
        if( lcb[ led ].state == true ) {                    // Turn master off and pair on
            lcb[ led ].state = false;
            if( lcb[ led ].in_pair == true ) {
                lcb[ lcb[ led ].pair ].state = true;
            }
            update = true;
        }
    } else if( lcb[ led ].count < lcb[ led ].flash_count ){
        lcb[ led ].count += 1;
        if( lcb[ led ].in_pair == true ) {
            if( lcb[ lcb[ led ].pair ].state == true ) {
                lcb[ led ].state = false;                   // Master off, pair off
                lcb[ lcb[ led ].pair ].state = false;
                update = true;
            }
        } else {
            lcb[ led ].state = false;                       // Should not get here as we should be in a pair - but just turn Master off
            update = true;
        }
    } else {    // End of flash - start again
        lcb[ led ].count = 0;
    }
    if( update == true ) {
        value = lcb[ led ].state;
        (*lcb[ led ].update_led_status)( 0, &value );
        if( lcb[ led ].in_pair == true ) {
            value = lcb[ lcb[ led ].pair ].state;
            (*lcb[ lcb[ led ].pair ].update_led_status)( 0, &value );
        }
    }

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
        imx_cli_print( "Led Status: %s: ", led_names[ i ] );
        if( lcb[ i ].blinking == true )
            imx_cli_print( "Blink Count 1 %u %umS, Count 2 %u %umS", lcb[ i ].blink_1_count * 10, lcb[ i ].blink_2_count * 10 );
        else if( lcb[i ].flashing == true )
            imx_cli_print( "Flashing duration %u Sec", ( lcb[ i ].flash_count / 10 ) );
        else if( lcb[ i ].state == true )
            imx_cli_print( "On" );
        else
            imx_cli_print( "Off" );
        imx_cli_print( "\r\n" );

    }
}
static void select_leds( imx_led_t led, imx_led_t *master_led, imx_led_t *slave_led )
{
    switch( led ) {
    case IMX_LED_RED_GREEN :
        *master_led = IMX_LED_RED;
        *slave_led = IMX_LED_GREEN;
        break;
    case IMX_LED_RED_BLUE :
        *master_led = IMX_LED_RED;
        *slave_led = IMX_LED_BLUE;
        break;
    case IMX_LED_GREEN_RED :
        *master_led = IMX_LED_GREEN;
        *slave_led = IMX_LED_RED;
        break;
    case IMX_LED_GREEN_BLUE :
        *master_led = IMX_LED_GREEN;
        *slave_led = IMX_LED_BLUE;
        break;
    case IMX_LED_BLUE_RED :
        *master_led = IMX_LED_BLUE;
        *slave_led = IMX_LED_RED;
        break;
    case IMX_LED_BLUE_GREEN :
        *master_led = IMX_LED_BLUE;
        *slave_led = IMX_LED_GREEN;
        break;
    default :
        break;
    }
    // imx_printf( "Master: %u, Slave: %u\r\n", *master_led, *slave_led );

}
