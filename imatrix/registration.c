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
 * so agrees to indemnity Sierra against all liability.
 */

/** @file registration.c
 *
 *  Created on: May 15, 2017
 *      Author: greg.phillips
 *
 *      Register a Thing on iMatrix
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../coap/coap_transmit.h"
#include "../cli/interface.h"
#include "../device/hal_leds.h"
#include "../device/config.h"
#include "../device/hal_leds.h"
#include "../device/icb_def.h"
#include "../time/ck_time.h"
#include "registration.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define COAP_DELAY              ( 5 * 1000 )
#define COAP_TIMEOUT            ( 10 * 1000 )      // Very generous 10 seconds
#define WIFI_TIMEOUT            ( 30 * 1000 )      // Up to 30 seconds to establish link
#define REGISTRATION_DISPLAY    ( 4 * 1000 )
#define REGISTRATION_TIMEOUT    ( 10 * 1000 )      // Response time from Server
#define REGISTRATION_RETRIES    5       // 5 attempts
/******************************************************
 *                   Enumerations
 ******************************************************/
enum {
    INIT_REGISTRATION,
    CHECK_COAP_QUEUE,
    SETUP_LINK,
    CHECK_WIFI_UP,
    SETUP_REGISTRATION,
    SEND_REGISTRATION,
    CHECK_REGISTRATION_ACK,
    REGISTRATION_SHOW_DONE,
    REGISTRATION_IDLE,
};
enum {
    NO_ERROR = 0,
    COAP_TIMEOUT_ERROR,
    WIFI_TIMEOUT_ERROR,
    REGISTRATION_TIMEOUT_ERROR,
    NO_ERROR_CODES,
};
/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct {
    uint16_t state;
    uint16_t last_state;
    uint16_t timer;
    uint16_t retry_count;
    uint16_t error_code;
    wiced_time_t reg_timer;
    unsigned int ack_received;
} registration_t;
/******************************************************
 *                    Structures
 ******************************************************/
static registration_t registration = {
        .state = REGISTRATION_IDLE,
};
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;   // Defined in device/config.h
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Initialize the Registration process
  * @param  None
  * @retval : None
  */

void init_registration(void)
{
    registration.state = INIT_REGISTRATION;
    icb.registration_in_process = true;
    imatrix_save_config();
}

/**
  * @brief  notification that ack was received
  * @param  current time
  * @retval : None
  */
void ack_registration(void)
{
    registration.ack_received = true;
}

/**
  * @brief  Register a Thing on the iMatrix Server.
  * @param  current time
  * @retval : None
  */
const int32_t LED_OFF = 0;
const int32_t LED_ON = 1;
const int32_t LED_BLINK_1 = 2;
const int32_t LED_BLINK_4 = 5;

void registration_process(wiced_time_t current_time )
{
    if( registration.last_state != registration.state ) {
        registration.last_state = registration.state;
        imx_printf( "Changing Registration state to: %u\r\n", registration.state );
    }
    switch( registration.state ) {
        case INIT_REGISTRATION :
            registration.error_code = NO_ERROR;
            registration.reg_timer = current_time;
            imx_printf( "Registration in process\r\n" );
            set_host_led( IMX_LED_GREEN, IMX_LED_BLINK_1 );     // Set Green Led to Blink 1 times a second
            set_host_led( IMX_LED_RED, IMX_LED_OFF );           // Set RED Led to off
            registration.state = CHECK_COAP_QUEUE;
            break;
        case CHECK_COAP_QUEUE :
             /*
             * We need to make sure that the ack for the request is sent back to the mobile app
             */
            if( coap_udp_xmit_empty() == true )
                registration.state = SETUP_LINK;
            if( is_later( current_time, registration.reg_timer + COAP_TIMEOUT ) == true ) {
                /*
                 * Something strange going on - should never have traffic still going on at this point.
                 */
                imx_printf( "Aborting Registration process due to pending CoAP traffic\r\n" );
                registration.error_code = COAP_TIMEOUT_ERROR;
                /*
                 * Failure Show RED LED
                 */
                set_host_led( IMX_LED_GREEN, IMX_LED_OFF );
                set_host_led( IMX_LED_RED, IMX_LED_ON );           // Set RED Led to on
                registration.reg_timer = current_time;
                registration.state = REGISTRATION_SHOW_DONE;
            }
            break;
        case SETUP_LINK :
            /*
             * Wait a bit to make sure CoAP transmit fully flushed
             */
            if( is_later( current_time, registration.reg_timer + COAP_DELAY ) == true ) {
                device_config.AP_setup_mode = false;
                imatrix_save_config();
                registration.reg_timer = current_time;
                icb.wifi_up = false;
                registration.state = CHECK_WIFI_UP;
            }
            break;
        case CHECK_WIFI_UP :
            if( icb.wifi_up == true )
                registration.state = SETUP_REGISTRATION;
            else if( is_later( current_time, registration.reg_timer + WIFI_TIMEOUT ) == true ) {
                imx_printf( "Wi Fi Failed to come up\r\n" );
                device_config.AP_setup_mode = true;     // Come back up in AP mode
                imatrix_save_config();
                registration.error_code = WIFI_TIMEOUT_ERROR;
                /*
                 * Failure Show RED LED
                 */
                set_host_led( IMX_LED_GREEN, IMX_LED_OFF );
                set_host_led( IMX_LED_RED, IMX_LED_ON );           // Set RED Led to on
                registration.reg_timer = current_time;
                registration.state = REGISTRATION_SHOW_DONE;
            }
            break;
    case SETUP_REGISTRATION :
        registration.retry_count = 0;
        registration.ack_received = false;
        set_host_led( IMX_LED_GREEN, IMX_LED_BLINK_4 );             // Set Green Led to Blink 4 times a second
        registration.state = SEND_REGISTRATION;
        break;
    case SEND_REGISTRATION :
        imx_printf( "Sending Registration request\r\n" );
        icb.send_registration = true;
        registration.reg_timer = current_time;
        registration.state = CHECK_REGISTRATION_ACK;
        break;
    case CHECK_REGISTRATION_ACK :
        if( registration.ack_received == true ) {
            /*
             * All good we are registered - Code already set to NO_ERROR, and we are online and running - all done
             */
            /*
             * Success Show GREEN LED
             */
            set_host_led( IMX_LED_GREEN, IMX_LED_ON );
            set_host_led( IMX_LED_RED, IMX_LED_OFF );
            registration.reg_timer = current_time;
            registration.state = REGISTRATION_SHOW_DONE;
        } else if( is_later( current_time, registration.reg_timer + REGISTRATION_TIMEOUT ) == true ) {
            if( registration.retry_count >= REGISTRATION_RETRIES ) {
                imx_printf( "Aborting due to retry accounts exceeded\r\n" );
                /*
                 * Failed to get a registration response - abort and return to provisioning mode
                 */
                device_config.AP_setup_mode = true;     // Come back up in AP mode
                icb.wifi_up = false;
                registration.error_code = REGISTRATION_TIMEOUT_ERROR;
                /*
                 * Failure Show RED LED
                 */
                set_host_led( IMX_LED_GREEN, IMX_LED_OFF );
                set_host_led( IMX_LED_RED, IMX_LED_ON );           // Set RED Led to on
                registration.reg_timer = current_time;
                registration.state = REGISTRATION_SHOW_DONE;
            }
        }

        break;
    case REGISTRATION_SHOW_DONE :
        if( is_later( current_time, registration.reg_timer + REGISTRATION_DISPLAY ) == true ) {
            set_host_led( IMX_LED_GREEN, IMX_LED_OFF );
            set_host_led( IMX_LED_RED, IMX_LED_OFF );           // Set RED Led to on
            registration.state = REGISTRATION_IDLE;
            device_config.provisioned = true;
            icb.registration_in_process = false;
            imatrix_save_config();
        }
        break;
    case REGISTRATION_IDLE :
        break;
    default :
        registration.state = INIT_REGISTRATION;
        break;
    }
}
