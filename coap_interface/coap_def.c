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
 * CoAP Definition file - define nodes and interface routines
 *
 */


#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "wiced.h"

#include "../storage.h"
#include "../json/mjson.h"
#include "../cli/interface.h"
#include "../CoAP/coap.h"
#include "../CoAP/add_coap_option.h"
#include "../CoAP/que_manager.h"
#include "../CoAP_interface/imx_get_uint_from_query_str.h"
#include "../device/icb_def.h"
#include "../ota_loader/ota_loader.h"
#include "../ota_loader/ota_structure.h"
#include "../time/ck_time.h"
#include "coap_msg_get_store.h"
#include "coap_def.h"

#include "coap_config_imatrix.h"
#include "coap_control_cs_ctrl.h"
#include "coap_control_getlatest.h"
#include "coap_control_imatrix.h"
#include "coap_control_otaupdate.h"
#include "coap_control_reboot.h"
#include "coap_control_securessid.h"
#include "coap_control_security.h"
#include "coap_sensor_rssi.h"
#include "token_string.h"
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
#define NON_GROUP_DATA          -1

// MULTICAST_STRING_LENGTH,URI_STRING_LENGTH, MAX_CHARS_FOR_IP are used by coap_post_control_multicast()
#define MULTICAST_STRING_LENGTH 500
#define URI_STRING_LENGTH 		50
#define QUERY_STRING_LENGTH 	50
#define MAX_CHARS_FOR_IP 		41
#define IMAGE_NAME_LENGTH 		15
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
 *               Variable Declarations
 ******************************************************/
extern IOT_Device_Config_t device_config;   // Defined in storage.h
/******************************************************
 *               Variable Definitions
 ******************************************************/
// NO_COAP_ENTRIES defined in coap.c
CoAP_entry_t CoAP_entries[ NO_IMATRIX_COAP_ENTRIES ] = // The structure must be fully initialized and ALL URI's are sorted in alphabetical sequence
{
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/.well-known/core",
       .node.att.title = "well-known",
       .node.att.rt = "",
       .node.att.if_desc = "all uri's",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = get_well_known,
       .node.put_function = NULL,
       .node.post_function = NULL,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/config/iMatrix",
       .node.att.title = "Set iMatrix End point",
       .node.att.rt = "End_point",
       .node.att.if_desc = "config",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = NULL,
       .node.put_function = NULL,
       .node.post_function = coap_post_config_imatrix,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/cs_ctrl",
       .node.att.title = "Control Sensor setting",
       .node.att.rt = "settings",
       .node.att.if_desc = "control/sensor",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = coap_get_control_cs_ctrl,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_cs_ctrl,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/reboot",
       .node.att.title = "Reboot Fixture",
       .node.att.rt = "Reboot",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = NULL,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_reboot,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/iMatrix",
       .node.att.title = "Get SN and Product ID",
       .node.att.rt = "SN_Produc_ID",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = coap_get_control_imatrix,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_imatrix,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/securessid",
       .node.att.title = "Set SSID & WPA2-PSK",
       .node.att.rt = "securessid",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = coap_get_control_securessid,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_securessid,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/security",
       .node.att.title = "Set Certs",
       .node.att.rt = "security",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = coap_get_control_security,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_security,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/otagetlatest",
       .node.att.title = "Update an Image",
       .node.att.rt = "Site, URI, image number",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = NULL,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_otagetlatest,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/control/otaupdate",
       .node.att.title = "Update an Image",
       .node.att.rt = "Site, URI, image number",
       .node.att.if_desc = "control",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = NULL,
       .node.put_function = NULL,
       .node.post_function = coap_post_control_otaupdate,
       .node.delete_function = NULL
   },
   {
       .header.next = NULL,
       .header.prev = NULL,
       .node.uri = "/sensor/rssi",
       .node.att.title = "Fixture RSSI",
       .node.att.rt = "rssi-c",
       .node.att.if_desc = "sensor",
       .node.att.sz = 0,
       .node.arg = 0,
       .node.get_function = coap_get_sensor_rssi,
       .node.put_function = NULL,
       .node.post_function = NULL,
       .node.delete_function = NULL
   },
};

/******************************************************
 *               Function Definitions
 ******************************************************/


uint16_t get_well_known(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( cd );
    UNUSED_PARAMETER( arg );

    if ( msg == NULL ) {
        PRINTF( "NULL value sent to get_well_known function.\r\n" );
        return COAP_NO_RESPONSE;
    }

    uint16_t payload_index = 1080;// Suggest a minimum number of bytes for the message_t struct's data block.
    uint16_t i;
    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    PRINTF( "GET Request - '/.well-know/core'\r\n");
    /*
     * Fill in the msg with the data
     */
	if( coap_store_response_header( msg, CONTENT, response_type, &payload_index )  != WICED_SUCCESS ) {
		PRINTF( "Failed to create response.\r\n" );
		return COAP_NO_RESPONSE;
	}

    for( i = 0; i < NO_IMATRIX_COAP_ENTRIES; i++ ) {

    	char* comma = "";
    	if ( i > 0 )
    	    comma = ",";

        if ( coap_append_response_payload_using_printf( LINK_MEDIA_TYPE, msg,
        		"%s<%s>;title=\"%s\";rt=\"%s\";if=\"%s\"", comma,
        		CoAP_entries[ i ].node.uri, CoAP_entries[ i ].node.att.title,
        		CoAP_entries[ i ].node.att.rt, CoAP_entries[ i ].node.att.if_desc  ) != WICED_SUCCESS )
        {
        	break;// Skip any that don't fit in the message.
        }
    }

    char* payload = coap_msg_payload( msg );
    if ( payload != NULL ) {
        PRINTF( "Sending Well Known response '%s'\r\n", payload );
    }

    return COAP_SEND_RESPONSE;
}

