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
/*
 * coap_msg_get_store.h
 *
 *  Created on: Dec 28, 2016
 *      Author: greg.phillips
 */

#ifndef COAP_MSG_GET_STORE_H_
#define COAP_MSG_GET_STORE_H_

#include "../coap/coap.h"

/** @file
 *
 *	Function defines for coap_msg_get_store.c
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

#endif /* COAP_MSG_GET_STORE_H_ */
/*
 *  Created on: Apr 22, 2015
 *      Author: eric thelin
 */


wiced_result_t coap_store_response_header( coap_message_t* msg_out, uint16_t code_in, uint16_t type_in, uint16_t *header_size );
wiced_result_t coap_store_response_data(coap_message_t* msg_out, uint16_t code_in,
    uint16_t type_in, char* data_str, uint16_t media_type );
wiced_result_t coap_store_response_binary( coap_message_t* msg_out, uint16_t code_in,
    uint16_t type_in, uint8_t* data, uint16_t data_size, uint16_t media_type );
wiced_result_t coap_append_response_payload(uint16_t media_type, coap_message_t* msg_out, uint8_t* data, uint16_t size);
//wiced_result_t coap_append_response_binary_payload(coap_message_t* msg_out, uint8_t* data, uint16_t size);
//wiced_result_t coap_append_response_payload_with_string(coap_message_t* msg_out, char* str);
wiced_result_t coap_append_response_payload_using_printf( uint16_t media_type, coap_message_t* msg_out, char* format,  ... );
