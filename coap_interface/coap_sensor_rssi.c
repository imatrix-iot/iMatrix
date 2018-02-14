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
 *
 *  Created on: December, 2015
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "wiced.h"

#include "../device/icb_def.h"
#include "../coap/coap.h"
#include "../coap_interface/coap_def.h"
#include "../device/hal_wifi.h"
#include "coap_sensor_rssi.h"
#include "coap_msg_get_store.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif
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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;   // Defined in storage.h

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_get_rssi
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_get_sensor_rssi(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( cd );
    UNUSED_PARAMETER( arg );

    if ( msg == NULL ) {
        PRINTF( "Invalid NULL arguments passed to coap_get_sensor_rssi.\r\n");
        return COAP_NO_RESPONSE;
    }
    char json_out[100];
    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    sprintf( json_out, "{ \"wifiRssi\" : %ld, \"signalNoise\" : %ld }", hal_get_wifi_rssi(), hal_get_wifi_rf_noise() );


    if ( coap_store_response_data( msg, CONTENT, response_type, json_out, JSON_MEDIA_TYPE ) != WICED_SUCCESS ) {
		PRINTF( "Failed to create response.\r\n" );
		return COAP_NO_RESPONSE;
    }

    return COAP_SEND_RESPONSE;
}
