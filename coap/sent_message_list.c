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
* sent_message_list.c
 *
 *  Created on: Jul 12, 2016
 *      Author: Eric Thelin
 */


#include <stdbool.h>
#include "wiced.h"

#include "coap.h"
#include "../cli/interface.h"
#include "../time/ck_time.h"
#include "sent_message_list.h"

/*******************
 * Private Variables
 *******************/
static sent_message_list_t messages_awaiting_response = { .list = {{0,0,0,0,{0},0}}, .top = NOT_FOUND_IN_LIST, .free = 0 };
// The above initialization does not initialize the next index attribute of the list correctly,
// so the list must be created before use.
static uint16_t sent_message_list_requires_creation = true;

/********************************
 * Private Function Declarations
 ********************************/
static void create_free_list_by_connecting_next_indexes_in_sent_msg_list();
static void free_oldest_msg();

/*****************************
 * Public Function Definitions
 *****************************/

/**
 * Retrieve the token and token length from one of the stored sent message lists.
 * Return WICED_ERROR if the sent message cannot be found in a list or NULL is passed in.
 * If successful return WICED_SUCCESS.
 *
 * written by Eric Thelin 13 July 2016
 */
wiced_result_t get_sent_msg_token( sent_message_t sent, token_buffer_t token, uint16_t* token_length )
{
	if ( ( token == NULL ) || ( token_length == NULL ) || sent_message_list_requires_creation ) {
		imx_printf("Null value passed to get_sent_msg_token function or No list from which to get messages.\r\n");
		return WICED_ERROR;
	}
	if ( sent.id >= SENT_MESSAGE_LIST_SIZE ) {
		imx_printf("Index(%u) out of array bounds(%u) in get_sent_msg_token.\r\n", sent.id, SENT_MESSAGE_LIST_SIZE );
		return WICED_ERROR;
	}
	if ( ( sent.may_resend == 0 ) && ( sent.id != NOT_FOUND_IN_LIST ) ) {
		*token_length = messages_awaiting_response.list[ sent.id ].tkl;
		memmove( token, messages_awaiting_response.list[ sent.id ].token, *token_length );
		return WICED_SUCCESS;
	}
	imx_printf("Message not found in get_sent_msg_token.\r\n");
	return WICED_ERROR;// Returning data from coap xmit list isn't supported yet.
}

/**
 * Return 1(true) if the sent message was a multicast.
 * Otherwise return 0(false).
 *
 * written by Eric Thelin 13 July 2016
 */
uint16_t sent_msg_is_multicast( sent_message_t sent )
{
	if ( sent.id >= SENT_MESSAGE_LIST_SIZE ) {
		imx_printf("Index(%u) out of array bounds(%u) in sent_msg_is_multicast.\r\n", sent.id, SENT_MESSAGE_LIST_SIZE );
		return 0;//false
	}
	if ( ( sent.may_resend == 0 ) && ( sent.id != NOT_FOUND_IN_LIST ) && ( sent_message_list_requires_creation == false ) ) {
		return messages_awaiting_response.list[ sent.id ].sent_as_multicast;
	}
	imx_printf("Message not found or no list from which to get message in sent_msg_is_multicast.\r\n");
	return 0;// Returning data from coap xmit list isn't supported yet.
}

/**
 * Return the processing method of the sent message.
 * Otherwise return the default method: IGNORE_RESPONSE
 *
 * written by Eric Thelin 13 July 2016
 */
uint8_t sent_msg_processing_method( sent_message_t sent )
{
	if ( sent.id >= SENT_MESSAGE_LIST_SIZE ) {
		imx_printf("Index(%u) out of array bounds(%u) in sent_msg_processing_method.\r\n", sent.id, SENT_MESSAGE_LIST_SIZE );
		return IGNORE_RESPONSE;
	}
	if ( ( sent.may_resend == 0 ) && ( sent.id != NOT_FOUND_IN_LIST ) && ( sent_message_list_requires_creation == false ) ) {
		return messages_awaiting_response.list[ sent.id ].response_processing_method;
	}
	imx_printf("Message not found or no list from which to get message in sent_msg_processing_method.\r\n");
	return IGNORE_RESPONSE;// Returning data from coap xmit list isn't supported yet.
}

/**
 * Save a short summary of information from the message_t struct for a sent message.
 * The information should be sufficient to process a response,
 * but will not be used to retransmit the message.
 * If a message with the same token is already saved keep the one with the most recent initial timestamp.
 * If no free messages are available, free all expired messages.
 * If there still aren't any free then free the oldest message.
 *
 * written by Eric Thelin 12 July 2016
 */
void expect_response_from( coap_message_t* msg )
{
	sent_message_t sent;
    uint16_t i;

	if ( msg == NULL ) {
		imx_printf( "Null passed to expect_response_from function.\r\n");
		return;
	}
	if ( sent_message_list_requires_creation ) {
		create_free_list_by_connecting_next_indexes_in_sent_msg_list();
	}
	if ( msg->header.tkl > MAX_TOKEN_LENGTH ) {
		imx_printf("Token length out of bounds in find_sent_msg.\r\n");
		return;
	}

	// Duplicate elimination

	sent = find_sent_msg( msg );
	if ( sent.id != NOT_FOUND_IN_LIST ) {

		imx_printf("Attempt to save a new message with the same token as a message that is already in the sent message list.\r\n");

		// If the matching message in the list is more recent, discard the passed in message without saving anything.

		if ( imx_is_later( messages_awaiting_response.list[ sent.id ].initial_timestamp, msg->initial_timestamp ) ) {
			imx_printf("Rejecting new message because it is too old.");
			return;
		}
		else imx_printf("Keeping new message.\r\n");
	}
	else {// Find a free message into which to save the passed in message.

		if (  messages_awaiting_response.free == NOT_FOUND_IN_LIST ) {

			free_all_expired_sent_msg();

			if (  messages_awaiting_response.free == NOT_FOUND_IN_LIST ) {
				free_oldest_msg();
			}
		}

		if ( messages_awaiting_response.free == NOT_FOUND_IN_LIST ) {
			imx_printf("No free messages available in expect_response_from.\r\n");
			return;
		}
		else {// Use the first free message.

			sent.id = messages_awaiting_response.free;
		    sent.may_resend = 0;
			messages_awaiting_response.free = messages_awaiting_response.list[ sent.id ].next;

			// Move this to the top of the list.

			messages_awaiting_response.list[ sent.id ].next = messages_awaiting_response.top;
			messages_awaiting_response.top = sent.id;
		}
	}

	// Copy data from the passed in message.

	messages_awaiting_response.list[ sent.id ].initial_timestamp = msg->initial_timestamp;
	messages_awaiting_response.list[ sent.id ].sent_as_multicast = is_multicast_ip( &( msg->ip_addr ) );
	messages_awaiting_response.list[ sent.id ].response_processing_method = msg->response_processing_method;
	messages_awaiting_response.list[ sent.id ].tkl = msg->header.tkl;
	for ( i = 0; i < msg->header.tkl; i++ ) {
		messages_awaiting_response.list[ sent.id ].token[ i ] = msg->data_block->data[ i ];
	}
}

/**
 * Return the sent message identifier that matches the token in a received response.
 * The identifier can be used to extract the processing method
 * and a few other things from cached information from the sent message.
 * If no sent message entry matches the token, return an id of NOT_FOUND_IN_LIST.
 *
 * written by Eric Thelin 12 July 2016
 */
sent_message_t find_sent_msg( coap_message_t* response_msg  )
{
	//uint16_t byte = 0;
	//uint16_t token_matches = true;
    sent_message_t entry = { .may_resend = 0, .id = NOT_FOUND_IN_LIST };

	if ( ( response_msg == NULL ) || ( response_msg->data_block == NULL ) ) {
		imx_printf("Null found or sent message list has not been created in find_sent_msg.\r\n");
		return entry;// NOT FOUND.
	}

	if ( response_msg->header.tkl > MAX_TOKEN_LENGTH ) {
		imx_printf("Token length out of bounds in find_sent_msg.\r\n");
		return entry;// NOT FOUND.
	}

	if ( sent_message_list_requires_creation ) {
		return entry;// NOT FOUND.
	}

	entry.id =  messages_awaiting_response.top;

	while ( entry.id != NOT_FOUND_IN_LIST ) {

		if ( ( response_msg->header.tkl == messages_awaiting_response.list[ entry.id ].tkl ) &&
        		( 0 == memcmp( response_msg->data_block->data, messages_awaiting_response.list[ entry.id ].token, response_msg->header.tkl ) ) ) {
        	return entry;
        }
		entry.id = messages_awaiting_response.list[ entry.id ].next;
	}
	return entry;
}

/**
 * Free all sent message entries where the initial_timestamp is more than SENT_MESSAGE_EXPIRATION ms ago.
 *
 * written by Eric Thelin 12 July 2016
 */
void free_all_expired_sent_msg()
{
	uint8_t entry_id, last_entry_id, next_entry_id;
	wiced_time_t now;

	if ( sent_message_list_requires_creation ) {
//		imx_printf("No list from which to free messages in free_all_expired_sent_msg.\r\n");//This can legitimately happen, so don't send an error message.
		return;
	}

	entry_id = messages_awaiting_response.top;
	last_entry_id = entry_id;
	wiced_time_get_time( &now );

	while ( entry_id != NOT_FOUND_IN_LIST ) {
		if ( imx_is_later( now, messages_awaiting_response.list[ entry_id ].initial_timestamp + SENT_MESSAGE_EXPIRATION ) ) {

			// Free expired entry;

			next_entry_id = messages_awaiting_response.list[ entry_id ].next;
			if ( messages_awaiting_response.top == entry_id ) {
				last_entry_id = next_entry_id;
				messages_awaiting_response.top = next_entry_id;
			}
			else {
				messages_awaiting_response.list[ last_entry_id ].next = next_entry_id;
			}

			// Add entry to free list.

			memset( &( messages_awaiting_response.list[ entry_id ] ), 0, sizeof( coap_message_summary_t ) );
			messages_awaiting_response.list[ entry_id ].next = messages_awaiting_response.free;
			messages_awaiting_response.free = entry_id;

			entry_id = next_entry_id;
		}
		else {
			last_entry_id = entry_id;
			entry_id = messages_awaiting_response.list[ entry_id ].next;
		}
	}
}

uint16_t sent_msg_list_length()
{
	uint16_t this, length = 0;

	if ( sent_message_list_requires_creation ) {
		return 0;
	}

	this = messages_awaiting_response.top;
    while ( this != NOT_FOUND_IN_LIST) {
    	length++;
    	this = messages_awaiting_response.list[ this ].next;
    }
    return length;
}

uint16_t free_sent_msg_list_length()
{
	uint16_t this, length = 0;

	if ( sent_message_list_requires_creation ) {
		return 0;
	}

	this = messages_awaiting_response.free;
    while ( this != NOT_FOUND_IN_LIST) {
    	length++;
    	this = messages_awaiting_response.list[ this ].next;
    }
    return length;
}

/******************************
 * Private Function Definitions
 ******************************/

/**
 * Connect each struct in the sent message array to the following struct by assigning the correct index value to next.
 * The last struct is assigned next = NOT_FOUND_IN_LIST.
 *
 * ASSUME: All values in the messages_awaiting_response list should be initialized
 * except for the next indexes for the free list.
 * This is intended to be called once before anything is added to the list,
 * and never called again until the next boot.
 *
 * written by Eric Thelin 16 July 2016
 */
static void create_free_list_by_connecting_next_indexes_in_sent_msg_list()
{
	uint16_t i;
	for ( i = 0; i < SENT_MESSAGE_LIST_SIZE; i++ ) {
		messages_awaiting_response.list[ i ].next = i + 1;
	}
	messages_awaiting_response.list[ SENT_MESSAGE_LIST_SIZE - 1 ].next = NOT_FOUND_IN_LIST;
	sent_message_list_requires_creation = false;
}

/**
 * When no free sent message entries are available,
 * free the oldest entry so that it may be used for a new entry.
 *
 * written by Eric Thelin 12 July 2016
 */
static void free_oldest_msg()
{
	uint8_t entry, next_entry, oldest, before_oldest;// = { .may_resend = 0, .id =  };
	//sent_message_t oldest = entry;

	if ( sent_message_list_requires_creation ) {
//		imx_printf("No list from which to free messages in free_oldest_msg.\r\n");//This can legitimately happen, so don't send an error message.
		return;
	}

	// Initialize using first entry.

	entry = messages_awaiting_response.top;
	if ( entry == NOT_FOUND_IN_LIST ) {
		return;
	}
	oldest = entry;
	before_oldest = entry;
	next_entry = messages_awaiting_response.list[ entry ].next;

	// Loop over remaining entries.

	while ( next_entry != NOT_FOUND_IN_LIST ) {
		if ( imx_is_later( messages_awaiting_response.list[ oldest ].initial_timestamp,
				messages_awaiting_response.list[ next_entry ].initial_timestamp ) ) {
            before_oldest = entry;
			oldest = next_entry;
		}
		entry = next_entry;
		next_entry = messages_awaiting_response.list[ next_entry ].next;
	}

	// Remove oldest entry from sent messages list.

	if ( oldest == messages_awaiting_response.top ) {
		messages_awaiting_response.top = messages_awaiting_response.list[ oldest ].next;
	}
	else {
		messages_awaiting_response.list[ before_oldest ].next =
				messages_awaiting_response.list[ oldest ].next;
	}

	// Free oldest entry.

	memset( &( messages_awaiting_response.list[ oldest ] ), 0, sizeof( coap_message_summary_t ) );
	messages_awaiting_response.list[ oldest ].next = messages_awaiting_response.free;
	messages_awaiting_response.free = oldest;

	return;
}

void test_is_later()
{
	imx_printf("Initial time[0] %lu ",  messages_awaiting_response.list[ 0 ].initial_timestamp);
	if ( imx_is_later( messages_awaiting_response.list[ 0 ].initial_timestamp,
					messages_awaiting_response.list[ 49].initial_timestamp ) ) {
		imx_printf(">");
	} else imx_printf("<=");
	imx_printf(" %lu time[49]",  messages_awaiting_response.list[ 49 ].initial_timestamp);
}
