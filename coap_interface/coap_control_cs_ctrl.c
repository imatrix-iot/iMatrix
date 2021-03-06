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

/** @file control_cs_ctrl.c
 *
 *  Created on: May 15, 2017
 *      Author: greg.phillips
 *
 *  CoAP interface to controls and sensor values
 *
 *  GET /control/cs_ctrl?type=<control|sensor>&id=<control/sensor ID>
 *  POST only works with controls
 */




#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"
#include "base64.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/var_data.h"
#include "../coap/coap.h"
#include "../coap/imx_coap.h"
#include "../cs_ctrl/hal_event.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../CoAP_interface/imx_get_uint_from_query_str.h"
#include "../cs_ctrl/imx_cs_interface.h"
#include "../device/var_data.h"
#include "../wifi/wifi.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "coap_control_cs_ctrl.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"
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
#define MAX_STRING_SIZE     1024     // for now - change this
#define BASE64_MAX_LENGTH   ( 4 * ( MAX_STRING_SIZE + 2 ) / 3 )
#define JSON_MAX_SIZE       ( 34 + IMX_CONTROL_SENSOR_NAME_LENGTH + BASE64_MAX_LENGTH )
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
extern IOT_Device_Config_t device_config;   // Defined in device\config.h
extern control_sensor_data_t *cd;
extern control_sensor_data_t *sd;
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_get_control_cs_ctrl
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */

/**
 * Process a control/cs_ctrl get request stored in the "msg" input parameter
 * by extracting "type=??" and "id=??" from the query string in the "coap_cd" input parameter.
 *
 * coap://10.3.0.107/control/cs_ctrl?type=0&id=481201181
 *
 * Scan control or sensor control blocks for ID and return "msg" updated to be a response
 * containing the JSON with name and current value in it.
 *
 * @param msg is the request, and returned as the response.
 * @param coap_cd  contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE unless the request was a multicast that does not need a response.
 *         In that case return COAP_NO_RESPONSE.
 *
 */
uint16_t coap_get_control_cs_ctrl(coap_message_t *msg, CoAP_msg_detail_t *coap_cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );
    // Respond with value for ID.
    char json_out[ JSON_MAX_SIZE ];     // Allocate this from heap later
    char base64_output[ BASE64_MAX_LENGTH ];
    int32_t result;
    uint16_t type, i;
    uint32_t id = 0;
    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( ( msg == NULL ) || ( coap_cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_get_control_param function.\r\n" );
        return COAP_NO_RESPONSE;
    }

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    PRINTF( "Get Configuration - '/control/cs_ctrl'\r\n");
    PRINTF( "URI Query: %s\r\n", coap_cd->uri_query );

    if ( WICED_SUCCESS != imx_get_uint_from_query_str( "type", &type, coap_cd->uri_query ) ) {// Require type
        if( coap_store_response_header( msg, BAD_REQUEST, response_type, NULL )  != WICED_SUCCESS ) {
            PRINTF( "Failed to create response.\r\n" );
            return COAP_NO_RESPONSE;
        }
        if( ( type != IMX_CONTROLS ) || ( type != IMX_SENSORS ) ) {
            PRINTF( "Failed to create response - invalid type.\r\n" );
            return COAP_NO_RESPONSE;
        }
    }  else {// type is OK.

        if ( WICED_SUCCESS == imx_get_uint32_from_query_str( "id", &id, coap_cd->uri_query ) ) {

            if ( imx_is_multicast_ip( &( msg->my_ip_from_request ) ) ) {
                return COAP_NO_RESPONSE;
            }
            /*
             * Scan array for entry
             */
            PRINTF( "Scanning for %u, id: %lu, 0x%08lx\r\n", type, id, id );
            if( type == IMX_CONTROLS ) {
                for( i = 0; i < device_config.no_controls; i++ )
                    if( device_config.ccb[ i ].id == id ) {
                        switch( device_config.ccb[ i ].data_type ) {
                            case IMX_UINT32 :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"uint_value\" : %lu }", device_config.ccb[ i ].name, cd[ i ].last_value.uint_32bit );
                                break;
                            case IMX_INT32 :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"int_value\" : %ld }", device_config.ccb[ i ].name, cd[ i ].last_value.uint_32bit );
                                break;
                            case IMX_FLOAT :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"float_value\" : %f }", device_config.ccb[ i ].name, cd[ i ].last_value.float_32bit );
                                break;
                            case IMX_VARIABLE_LENGTH :
                                if( cd[ i ].last_value.var_data->length > 0 ) {
                                    result = base64_encode( (unsigned char* ) cd[ i ].last_value.var_data->data, cd[ i ].last_value.var_data->length,
                                            (unsigned char*) base64_output, BASE64_MAX_LENGTH, BASE64_STANDARD );
                                    if( result < 0 ) {
                                        /*
                                         * Error
                                         */
                                        if( coap_store_response_header( msg, REQUEST_ENTITY_TOO_BIG, response_type, NULL )  != WICED_SUCCESS ) {
                                            PRINTF( "Failed to create response.\r\n" );
                                            return COAP_NO_RESPONSE;
                                        }
                                    }
                                }
                                sprintf( json_out, "{ \"name\" : \"%s\", \"var_value\" : \"%s\" }",
                                        device_config.ccb[ i ].name, ( cd[ i ].last_value.var_data == NULL ? "" : (char* ) base64_output ) );
                                break;
                        }
                        goto done;
                    }
            } else {
                for( i = 0; i < device_config.no_sensors; i++ )
                    if( device_config.scb[ i ].id == id ) {
                        switch( device_config.scb[ i ].data_type ) {
                            case IMX_UINT32 :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"uint_value\" : %lu }", device_config.scb[ i ].name, sd[ i ].last_value.uint_32bit );
                                break;
                            case IMX_INT32 :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"int_value\" : %ld }", device_config.scb[ i ].name, sd[ i ].last_value.uint_32bit );
                                break;
                            case IMX_FLOAT :
                                sprintf( json_out, "{ \"name\" : \"%s\", \"float_value\" : %f }", device_config.scb[ i ].name, sd[ i ].last_value.float_32bit );
                                break;
                            case IMX_VARIABLE_LENGTH :
                                result = base64_encode( (unsigned char* ) sd[ i ].last_value.var_data->data, sd[ i ].last_value.var_data->length,
                                        (unsigned char*) base64_output, BASE64_MAX_LENGTH, BASE64_STANDARD );
                                if( result < 0 ) {
                                    /*
                                     * Error
                                     */
                                    if( coap_store_response_header( msg, REQUEST_ENTITY_TOO_BIG, response_type, NULL )  != WICED_SUCCESS ) {
                                        PRINTF( "Failed to create response.\r\n" );
                                        return COAP_NO_RESPONSE;
                                    }
                                } else {
                                    sprintf( json_out, "{ \"name\" : \"%s\", \"var_value\" : \"%s\" }",
                                            device_config.scb[ i ].name, ( sd[ i ].last_value.var_data == NULL ? "" : (char* ) base64_output ) );

                                }
                                break;
                        }
                        goto done;
                    }
            }
            /*
             * Failed to find ID
             */
            sprintf( json_out, "{ \"id\" : %lu, \"value\" : \"Not_Found\" }", id );
done:

            // else Respond to unicast with VALID(successfully did nothing).
            if( coap_store_response_header( msg, VALID, response_type, NULL )  != WICED_SUCCESS ) {
                return COAP_NO_RESPONSE;
            }

            if ( coap_store_response_data( msg, CONTENT, response_type, json_out, JSON_MEDIA_TYPE ) != WICED_SUCCESS ) {
                PRINTF( "Failed to create response.\r\n" );
                return COAP_NO_RESPONSE;
            }
        }
    }
    return COAP_SEND_RESPONSE;
}

/**
 * Set the control with "id":??? to "value":???
 * using "id" and "value" from the request's JSON payload.
 *
 * {"id":481201181, "uint_value":1 }
 *
 * @param msg initially the request, is returned as the response.
 * @param coap_cd contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE or
 *         COAP_NO_RESPONSE if the inbound message is so messed up it can't be interpreted or
 *         COAP_NO_RESPONSE when the inbound message was a multicast and the response is anything other than CHANGED.
 */
uint16_t coap_post_control_cs_ctrl(coap_message_t *msg, CoAP_msg_detail_t *coap_cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );

    char string_value[ BASE64_MAX_LENGTH ];
    char base64_result[ BASE64_MAX_LENGTH ];
    uint16_t i;
    unsigned int id, uint_value;
    int int_value, result;
    double double_value;
    float  float_value;
    imx_data_32_t value;
    imx_status_t imx_status;

    if ( ( msg == NULL ) || ( coap_cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_post_control_cs_ctrl.\r\n" );
        return COAP_NO_RESPONSE;
    }

    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }
    // if no response code is assigned assume a server error
    uint16_t response_code = INTERNAL_SERVER_ERROR;

    struct json_attr_t json_attrs[] = {
        {"id", t_uinteger, .addr.uinteger = &id, .dflt.uinteger = NO_ID_VALUE},
        {"uint_value", t_uinteger, .addr.uinteger = &uint_value, .dflt.uinteger = NO_VALUE_VALUE},
        {"int_value", t_uinteger, .addr.integer = &int_value, .dflt.integer = NO_VALUE_VALUE},
        {"float_value", t_real, .addr.real = &double_value, .dflt.real = NO_FLOAT_VALUE},
        {"var_value", t_string, .addr.string = &string_value[ 0 ], .len = MAX_STRING_SIZE },
        {NULL}
    };

    PRINTF( "POST Configuration - '/control/cs_ctrl'\r\n");
    /*
     * Process the passed URI Query
     */
    if( strlen( coap_cd->uri_query ) > 0 ) {
        PRINTF( "Query string sent to coap_post_control_cs_ctrl instead of JSON object.\r\n");
        response_code = BAD_REQUEST;
        goto create_response_and_exit;
    }
    PRINTF( "URI Query: %s\r\n", coap_cd->uri_query );
    PRINTF( "URI Payload: %s\r\n", coap_cd->payload );

    json_read_object( coap_cd->payload, json_attrs, NULL );

    float_value = (float) double_value;
    /*
     * Scan array for entry
     */
    PRINTF( "Updating: %lu, 0x%08lx - to: %lu, %ld, %f\r\n", id, id, uint_value, int_value, float_value );
    for( i = 0; i < device_config.no_controls; i++ )
        if( device_config.ccb[ i ].id == id ) {
            switch( device_config.ccb[ i ].data_type ) {
                case IMX_UINT32 :
                    if( uint_value != NO_VALUE_VALUE ) {
                        if( imx_set_control_sensor( IMX_CONTROLS,  i, &uint_value ) != IMX_SUCCESS ) {
                            response_code = BAD_REQUEST;
                            goto create_response_and_exit;
                        }
                    } else {
                        response_code = BAD_REQUEST;
                        goto create_response_and_exit;
                    }
                    break;
                case IMX_INT32 :
                    if( int_value != NO_VALUE_VALUE ) {
                        if( imx_set_control_sensor( IMX_CONTROLS,  i, &int_value ) != IMX_SUCCESS ) {
                            response_code = BAD_REQUEST;
                            goto create_response_and_exit;
                        }
                    } else {
                        response_code = BAD_REQUEST;
                        goto create_response_and_exit;
                    }
                    break;
                case IMX_FLOAT :
                    if( float_value != NO_FLOAT_VALUE ) {
                        if( imx_set_control_sensor( IMX_CONTROLS,  i, &float_value ) != IMX_SUCCESS ) {
                            response_code = BAD_REQUEST;
                            goto create_response_and_exit;
                        }
                    } else {
                        response_code = BAD_REQUEST;
                        goto create_response_and_exit;

                    }
                    break;
                case IMX_VARIABLE_LENGTH :
                    /*
                     * Convert base 64 to binary
                     */
                    result = base64_decode( (unsigned char* ) string_value, strlen( string_value ), (unsigned char* ) base64_result, BASE64_MAX_LENGTH, BASE64_STANDARD );
                    if( result < 0 ) {
                        /*
                         * Error
                         */
                        if( coap_store_response_header( msg, REQUEST_ENTITY_TOO_BIG, response_type, NULL )  != WICED_SUCCESS ) {
                            PRINTF( "Failed to create response.\r\n" );
                            return COAP_NO_RESPONSE;
                        }
                    }
                    value.var_data = imx_get_var_data( result );
                    if( value.var_data != NULL ) {
                        memcpy( value.var_data->data, base64_result, result );
                        value.var_data->length = result; // Add space for the NULL - all data is returned 0 from allocation routine
                        imx_status = imx_set_control_sensor( IMX_CONTROLS,  i, &value );
                        imx_add_var_free_pool( value.var_data );
                        if( imx_status != IMX_SUCCESS ) {
                            response_code = BAD_REQUEST;
                            goto create_response_and_exit;
                        }
                    } else {
                        response_code = SERVICE_UNAVAILABLE;
                        goto create_response_and_exit;

                    }

                    break;
            }
            goto done;
        }
    /*
     * Failed to find id
     */
    PRINTF( "Invalid JSON object in coap_post_control_cs_ctrl function.\r\n");
    response_code = BAD_REQUEST;
    goto create_response_and_exit;

done:
    response_code = CHANGED;

create_response_and_exit:
    // if inbound message is so badly messed up a response can't be created properly, don't bother.
    if( WICED_SUCCESS != coap_store_response_header( msg, response_code, response_type, NULL ) ) {
        return COAP_NO_RESPONSE;
    }

    // Suppress all responses to multicast except CHANGED.
    if ( imx_is_multicast_ip( &( msg->my_ip_from_request ) ) && ( response_code != CHANGED ) ) {
        return COAP_NO_RESPONSE;
    }
    else {
        return COAP_SEND_RESPONSE;
    }
}
