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
 * sent_message_list.h
 *
 *  Created on: Jul 14, 2016
 *      Author: Eric Thelin
 */

#ifndef APPS_QUICKLEDAUTO_COAP_SENT_MESSAGE_LIST_H_
#define APPS_QUICKLEDAUTO_COAP_SENT_MESSAGE_LIST_H_

/******************************************************
 *                    Constants
 ******************************************************/
#define SENT_MESSAGE_LIST_SIZE  ( 0x32 )
#define NOT_FOUND_IN_LIST       ( 0xFF )
// NOT_FOUND_IN_LIST must be bigger than any index in the array so NOT_FOUND_IN_LIST >= SENT_MESSAGE_LIST_SIZE
#define SENT_MESSAGE_EXPIRATION ( 1 * MINUTES )

/******************************************************
 *                   Enumerations
 ******************************************************/
enum response_processing_directive {
    IGNORE_RESPONSE,
    POST_REMOTE_OCCUPANCY_ON_RESPONSE,
    POST_REMOTE_OCCUPANCY_OFF_RESPONSE,
	GET_OCCUPANCY_RESPONSE,
	PUT_HISTORY_RESPONSE,
};

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct describe_coap_message_summary {// A summary of information in the message_t struct defined in coap.h.
	// This summary should be sufficient to process a response, but should not include everything needed to resend a message.
    uint8_t        next;
    unsigned int   tkl        : 4;    // Token Length
    unsigned int   response_processing_method : 3;
    unsigned int   sent_as_multicast : 1;
    token_buffer_t token;
    wiced_time_t   initial_timestamp;
} coap_message_summary_t;

typedef struct describe_sent_message {
    unsigned int may_resend : 1;
    unsigned int id : 15;
} sent_message_t;

typedef struct describe_sent_message_list{
	coap_message_summary_t list[ SENT_MESSAGE_LIST_SIZE ];
	uint8_t top, free;
} sent_message_list_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t get_sent_msg_token( sent_message_t sent, token_buffer_t token, uint16_t* token_length );//PASSED all
uint16_t sent_msg_is_multicast( sent_message_t sent );//PASSED all
uint8_t sent_msg_processing_method( sent_message_t sent );//PASSED all
void expect_response_from( coap_message_t* msg );// Add msg to list of already sent messages.//PASSED all
sent_message_t find_sent_msg( coap_message_t* response_msg  );//PASSED input validation
void free_all_expired_sent_msg();// PASSED all
uint16_t sent_msg_list_length();// PASSED all
uint16_t free_sent_msg_list_length();// PASSED all

//static void create_free_list_by_connecting_next_indexes_in_sent_msg_list() PASSED all
//static void free_oldest_msg()// PASSED all

#endif /* APPS_QUICKLEDAUTO_COAP_SENT_MESSAGE_LIST_H_ */
