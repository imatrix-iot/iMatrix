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
/** @file que_manager.c
 *
 * Queue/List Manager Functions
 *
 */


#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "wiced.h"
#include "imatrix.h"

#include "../device/icb_def.h"
#include "coap.h"
#include "../cli/messages.h"
#include "../cli/interface.h"
#include "../time/ck_time.h"
#include "add_coap_option.h"
#include "que_manager.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
	#define PRINTF(...) if( ( icb.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) imx_log_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif


/******************************************************
 *                    Constants
 ******************************************************/
// 8 bytes
#define TINY_DATA_BYTES 8
#define NUM_MSG_W_TINY_DATA_SIZE 10

// 40 bytes
#define SMALL_DATA_BYTES 40
#define NUM_MSG_W_SMALL_DATA_SIZE 1

//80 bytes
#define MEDIUM_DATA_BYTES 80
#define NUM_MSG_W_MEDIUM_DATA_SIZE 15

//300 bytes
#define LARGE_DATA_BYTES 300
#define NUM_MSG_W_LARGE_DATA_SIZE 10

//1280 bytes
#define HUGE_DATA_BYTES 1280		//	Old size 1148
#define NUM_MSG_W_HUGE_DATA_SIZE 2

#define NUMBER_OF_DATA_SIZES 5

#define TOTAL_NUM_MESSAGE_BUFFERS ( NUM_MSG_W_TINY_DATA_SIZE + NUM_MSG_W_SMALL_DATA_SIZE \
		+ NUM_MSG_W_MEDIUM_DATA_SIZE + NUM_MSG_W_LARGE_DATA_SIZE + NUM_MSG_W_HUGE_DATA_SIZE )

// Each message_t without a data block is 88 bytes: 38 * 88 =
//3344 bytes
#define TOTAL_BYTES_FOR_MESSAGE_DATA (   NUM_MSG_W_TINY_DATA_SIZE * TINY_DATA_BYTES \
							           + NUM_MSG_W_SMALL_DATA_SIZE * SMALL_DATA_BYTES \
							           + NUM_MSG_W_MEDIUM_DATA_SIZE * MEDIUM_DATA_BYTES \
									   + NUM_MSG_W_LARGE_DATA_SIZE * LARGE_DATA_BYTES \
							           + NUM_MSG_W_HUGE_DATA_SIZE * HUGE_DATA_BYTES )

// Total: 9960 bytes less than 10K

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
void dump_list( message_list_t *list );
static uint16_t blocks_remaining_in_free_messages_by_size( uint16_t index );

/******************************************************
 *               Variable Definitions
 ******************************************************/
wiced_mutex_t list_mutex;
wiced_mutex_t udp_xmit_reset_mutex;
wiced_mutex_t last_packet_mutex;
static wiced_time_t last_udp_packet_recv_time = 0;

extern message_list_t list_free;
extern message_list_t list_udp_coap_recv;
extern message_list_t list_udp_coap_xmit;
extern message_list_t list_tcp_coap_recv;
extern message_list_t list_tcp_coap_xmit;
extern iMatrix_Control_Block_t icb;

uint16_t print_msg_saved_min_bytes;
static message_t *all_messages;         // list of all message_t structs with no data arrays added.
static uint16_t message_list_empty_errors;
static uint16_t block_not_found_errors;
static message_size_t free_messages_by_size[ NUMBER_OF_DATA_SIZES ];// one list for each size of msg data.
static message_data_block_t all_data_blocks[ TOTAL_NUM_MESSAGE_BUFFERS ];
static uint8_t *all_data_bytes;         //static uint8_t tiny_bytes_data[ NUM_MSG_W_TINY_DATA_SIZE ][ TINY_DATA_BYTES ];

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Each time get_msg check size of free list for that size packet. Store Max number of in use msg blocks for that size. If free list is empty for that size Add one to Max Size.
 * If cannot allocate any packet add one to Max Size for Huge packet.
 *
 * Store smallest size of freelist and store number of failed attempts to get that sized packet. store number of times no memory block could be assigned at all.
 *
NUMBER_OF_DATA_SIZES

*/
/**
  * @brief  mutex_init - Set up a list mutex
  * @param  None
  * @retval : None
  */
void mutex_init(void)
{
    wiced_result_t result;
    result = wiced_rtos_init_mutex( &list_mutex );
    if( result != WICED_SUCCESS )
        imx_printf( "Unable to setup list mutex...\r\n" );

    result = wiced_rtos_init_mutex( &udp_xmit_reset_mutex ); // this mutex is only used by coap_udp_xmit_reset() in coap_udp_xmit.c
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to setup mutex for transmitting reset...\r\n" );
    }

    result = wiced_rtos_init_mutex( &last_packet_mutex ); // this mutex is used to store the timestamp of the last packet.
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to setup mutex for timestamp of last packet...\r\n" );
    }

}

static void set_packet_time( wiced_time_t time, wiced_time_t *packet_recv_time )
{
	wiced_result_t result;
    result = wiced_rtos_lock_mutex( &last_packet_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to lock list mutex...\r\n" );
    }

    *packet_recv_time = time;

    result = wiced_rtos_unlock_mutex( &last_packet_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to un lock list mutex...\r\n" );
    }

}

static wiced_time_t get_packet_time( wiced_time_t *packet_recv_time )
{
	wiced_result_t result;
	wiced_time_t time;

	result = wiced_rtos_lock_mutex( &last_packet_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to lock list mutex...\r\n" );
    }
//wiced_time_get_utc_time_ms( &time );

	time = *packet_recv_time;

	result = wiced_rtos_unlock_mutex( &last_packet_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to un lock list mutex...\r\n" );
    }
    return time;
}

void set_last_packet_time( wiced_time_t time )
{
	set_packet_time( time, &last_udp_packet_recv_time );
}

wiced_time_t get_last_packet_time()
{
	return get_packet_time( &last_udp_packet_recv_time );
}

/**
  * @brief  list_init - Set up a clean list
  * @param  list to initialize
  * @retval : None
  *
  */
void list_init( message_list_t *list )
{
    list->head = NULL;
    list->tail = NULL;

}

/**
  * @brief  add a message to the bottom of the list
  * @param  list, new item
  * @retval : None
  *
  */
void list_add( message_list_t *list, message_t *entry )
{
    // Use 0 for any untimestamped entries and use 0 for retrys
    list_add_at( 0, list, entry, 0);

}

void list_add_at( wiced_time_t timestamp, message_list_t *list, message_t *new_entry, uint8_t send_attempts )
{
    wiced_result_t result;
    message_t* entry;

    result = wiced_rtos_lock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to lock list mutex...\r\n" );
    }
   
    new_entry->coap.send_attempts = send_attempts;
    new_entry->coap.initial_timestamp = 0;
    new_entry->coap.next_timestamp = timestamp;

    if( list->head == NULL ) { // Assume first entry
        list->head = new_entry;
        list->tail = new_entry;
        new_entry->header.next = NULL;
        new_entry->header.prev = NULL;
    }
    else {
        entry = list->tail;
        while ( imx_is_later( entry->coap.next_timestamp, timestamp )  && ( entry->header.prev != NULL )) {
            entry = entry->header.prev;
        }
        if ( imx_is_later( entry->coap.next_timestamp, timestamp ) ) { // then entry.prev == NULL so insert before entry
            list->head = new_entry;
            new_entry->header.prev = NULL;
            new_entry->header.next = entry;
            entry->header.prev = new_entry;
        }
        else { //insert after entry
            new_entry->header.prev = entry;
            new_entry->header.next = entry->header.next;
            if ( entry->header.next != NULL ) {
                entry->header.next->header.prev = new_entry;
            }
            else {
                list->tail = new_entry;
            }
            entry->header.next = new_entry;
        }
   }

    result = wiced_rtos_unlock_mutex( &list_mutex );   // All use same mutex for simplicity
        if( result != WICED_SUCCESS ) {
            imx_printf( "Unable to unlock list mutex...\r\n" );
        }

}
/**
  * @brief  pop a message from the top of the list
  * @param  list head
  * @retval : entry
  */
message_t  *list_pop( message_list_t  *list )
{
	if ( list == &list_free ) {
		return msg_get( HUGE_DATA_BYTES );
	}
    // use a timestamp of 0 for unstamped entries, none of the timestamped entries will have a timestamp less than that.
    return list_pop_before( 0, list );
}

message_t *list_pop_before( wiced_time_t timestamp, message_list_t *list )
{
    wiced_result_t result;
    message_t *entry;

    result = wiced_rtos_lock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to lock list mutex...\r\n" );
    }

    if ( ( list->head == NULL ) || ( imx_is_later( list->head->coap.next_timestamp, timestamp ) ) ) {// Assume no entries
        entry = NULL;
    }
    else {
        entry = list->head;
        if ( entry->header.next == NULL ) {
            list->head = NULL;
            list->tail = NULL;
        }
        else {
            list->head = entry->header.next;
            list->head->header.prev = NULL;     // This is the top now
        }
        entry->header.next = NULL;
        entry->header.prev = NULL;
    }

    result = wiced_rtos_unlock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to un lock list mutex...\r\n" );
    }
    return entry;

}
/**
 * Create or Initialize all lists for handling message_t structs
 * and the free lists of data blocks that can be assigned to the message_t structs.
 *
 * NOTE: Because it is possible to remove a message_t from a list
 * without instantaneously inserting it into another list,
 * there cannot be a corresponding destroy message lists function
 * without running the risk of a memory leak.
 * The only way to undo this create is to kill the program.
 *
 * written by Eric Thelin 27 February 2016
 */
bool create_msg_lists()
{
    /*
     * Allocate from memory pool
     */
    all_data_bytes = (uint8_t *) imx_allocate_storage( TOTAL_BYTES_FOR_MESSAGE_DATA );
    if( all_data_bytes == NULL )
        return false;
    all_messages = imx_allocate_storage(TOTAL_NUM_MESSAGE_BUFFERS * sizeof( message_t ) );
    if( all_messages == NULL )
        return false;

    // Create lists of data blocks

	uint16_t s;// Index to loop over free_messages_by_size[].
	uint16_t b = 0;// Index to all_data_blocks[].
	uint16_t m;// Index to loop over messages.
	uint16_t i = 0;// Index to all_data_bytes[].

    uint16_t num_msg = NUM_MSG_W_TINY_DATA_SIZE;
	free_messages_by_size[ 0 ].data_size = TINY_DATA_BYTES;

    message_list_empty_errors = 0;
    block_not_found_errors = 0;

	for( s = 0; s < NUMBER_OF_DATA_SIZES; s++ ) {
		free_messages_by_size[ s ].min_free_list_size = num_msg;// For statistical purposes only.
		free_messages_by_size[ s ].list_empty_errors = 0;//        For statistical purposes only.

		message_data_block_t* last_data_block = NULL;

		for( m = 0; m < num_msg; m++ ) {
			all_data_blocks[ b ].release_list_for_data_size = &( free_messages_by_size[ s ] );
			all_data_blocks[ b ].data = &( all_data_bytes[ i ] );
			all_data_blocks[ b ].next = last_data_block;

			i += free_messages_by_size[ s ].data_size;// Go to start of next data block.
			last_data_block = &( all_data_blocks[ b ] );
			b++;// Next data block struct.
		}
		free_messages_by_size[ s ].msg_data = last_data_block;

		// Assign number of data bytes and number of messages for the next bigger size.

		if( free_messages_by_size[ s ].data_size == TINY_DATA_BYTES ) {
			free_messages_by_size[ s + 1 ].data_size = SMALL_DATA_BYTES;
			num_msg = NUM_MSG_W_SMALL_DATA_SIZE;
		}
		else if( free_messages_by_size[ s ].data_size == SMALL_DATA_BYTES ) {
			free_messages_by_size[ s + 1 ].data_size = MEDIUM_DATA_BYTES;
			num_msg = NUM_MSG_W_MEDIUM_DATA_SIZE;
		}
		else if( free_messages_by_size[ s ].data_size == MEDIUM_DATA_BYTES ) {
			free_messages_by_size[ s + 1 ].data_size = LARGE_DATA_BYTES;
			num_msg = NUM_MSG_W_LARGE_DATA_SIZE;
		}
		else if( free_messages_by_size[ s ].data_size == LARGE_DATA_BYTES ) {
			free_messages_by_size[ s + 1 ].data_size = HUGE_DATA_BYTES;
			num_msg = NUM_MSG_W_HUGE_DATA_SIZE;
		}
		else if ( s != NUMBER_OF_DATA_SIZES - 1 ) {// Unless this is the last loop print error message.
			imx_printf("Unrecognized message data size: %u\r\n", free_messages_by_size[ s ].data_size );
			exit(-1);
		}
	}
	if( i != TOTAL_BYTES_FOR_MESSAGE_DATA ) {
		imx_printf( "Incorrect total of message bytes: %u\r\n", i );
		exit(-1);
	}

	// Create list of free message_t structs.

	list_init( &list_free );
	list_init( &list_udp_coap_recv );
	list_init( &list_udp_coap_xmit );
    list_init( &list_tcp_coap_recv );
    list_init( &list_tcp_coap_xmit );

	for ( i = 0; i < TOTAL_NUM_MESSAGE_BUFFERS; i++ ) {
		list_add( &list_free, &all_messages[ i ] );
	}
	imx_printf("%u message_t structs added to the free list.\r\n", TOTAL_NUM_MESSAGE_BUFFERS );
	return true;
}

/**
 * Return an available free message_t struct with a data block large enough for "min_bytes".
 *
 * written by Eric Thelin 27 February 2016
 */
message_t *msg_get( uint16_t min_bytes )
{
    wiced_result_t result;
    message_t *msg = NULL;// Return NULL if not successful.
    uint16_t remaining;

    result = wiced_rtos_lock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
    	icb.print_msg |= MSG_MSG_GET_NO_MUTEX;
        return NULL;
    }

    // Search for smallest free data block that is big enough for "min_bytes."
    uint16_t s;
    for( s = 0; s < NUMBER_OF_DATA_SIZES; s++ ) {// Smallest data blocks are listed first.

    	if ( min_bytes <= free_messages_by_size[ s ].data_size ) {// Data block is big enough.

    		if ( NULL == free_messages_by_size[ s ].msg_data ) {// Out of this size of data block.

    			if ( ( s == 0 ) || ( min_bytes > free_messages_by_size[ s - 1 ].data_size ) ) {// This is the smallest size that will fit.
    				free_messages_by_size[ s ].list_empty_errors++;
    			}
    		}
    		else {// Acceptable data block was found.

        		// Get new message.

        		msg = list_pop_before( 0, &list_free );
        		if ( msg == NULL ) {
        			icb.print_msg |= MSG_MSG_GET_OUT_MEMORY;
        			message_list_empty_errors++;
        			goto unlock_mutex_and_return_msg;
        		}
        		if ( ( msg->coap.data_block != NULL ) || // msg should be empty with no data_block
        		     ( free_messages_by_size[ s ].msg_data->data == NULL ) || // data block must have non-null data array
    				 ( &( free_messages_by_size[ s ] ) != free_messages_by_size[ s ].msg_data->release_list_for_data_size ) ) // correct release list.
        		{
        			icb.print_msg |= MSG_MSG_GET_MEM_LEAK;
        			msg = NULL;
        			goto unlock_mutex_and_return_msg;
        		}

        		// Initialize msg and data array to 0

        		memset( msg, 0, sizeof( message_t ) );
        		memset( free_messages_by_size[ s ].msg_data->data, 0, free_messages_by_size[ s ].data_size );

        		// Assign data block

        		msg->coap.data_block = free_messages_by_size[ s ].msg_data;
        		free_messages_by_size[ s ].msg_data = msg->coap.data_block->next;
        		msg->coap.data_block->next = NULL;

        		// Store minimum size of this free list.

        	    remaining = blocks_remaining_in_free_messages_by_size( s );
        		if ( remaining < free_messages_by_size[ s ].min_free_list_size ) {
        			 free_messages_by_size[ s ].min_free_list_size = remaining;
        		}

        		goto unlock_mutex_and_return_msg;
    		}
    	}
    }
    icb.print_msg |= MSG_MSG_GET_BLOCK_UNAVILABLE;
    print_msg_saved_min_bytes = min_bytes;
    block_not_found_errors++;

unlock_mutex_and_return_msg:
    result = wiced_rtos_unlock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
    	icb.print_msg |= MSG_MSG_GET_NO_UNLOCK_MUTEX;
    }
    return msg;
}

/**
 * Insert "msg" and the data block it contains into the appropriate free list.
 *
 * written by Eric Thelin 27 February 2016
 */
wiced_result_t msg_release( message_t *msg )
{
	if ( msg == NULL ) {
		imx_printf( "NULL passed to msg_release function.\r\n");
		return WICED_ERROR;
	}
    wiced_result_t result, return_result = WICED_SUCCESS;
    message_size_t *release_list = NULL;

    if ( msg->coap.data_block != NULL ) {// Release data part of message if it exists.

		result = wiced_rtos_lock_mutex( &list_mutex );   // All use same mutex for simplicity
		if( result != WICED_SUCCESS ) {
			imx_printf( "Unable to lock list mutex...\r\n" );
		}

		if ( ( msg->coap.data_block->next != NULL ) || // Data block must be disconnected from a free list before releasing it.
		     ( msg->coap.data_block->data == NULL ) || // Data array must exist in every data block.
		     ( msg->coap.data_block->release_list_for_data_size == NULL ) ) // Release list must exist.
		{
			imx_printf("Memory Leak Error in list_release function.\r\n");
			return_result = WICED_ERROR;
		}
		else {
			release_list = msg->coap.data_block->release_list_for_data_size;
			msg->coap.data_block->next = release_list->msg_data;
			release_list->msg_data = msg->coap.data_block;
			msg->coap.data_block = NULL;

			// Initialize data block array to 0
			memset( release_list->msg_data->data, 0, release_list->data_size );
		}
		result = wiced_rtos_unlock_mutex( &list_mutex );   // All use same mutex for simplicity
		if( result != WICED_SUCCESS ) {
			imx_printf( "Unable to unlock list mutex...\r\n" );
		}
    }

    // Initialize message to 0 and release

	memset( msg, 0, sizeof( message_t) );
	list_add( &list_free, msg );

    return return_result;
}

/**
 * Release all messages in the list.
 * NOTE: See the imx_is_later() function for details on how a roll over is handled
 * and how to force it to return false in list_pop_before().
 *
 * written by Eric Thelin 29 June 2016
 */
void list_release_all( message_list_t *list )
{
    const wiced_time_t biggest_time = 0xFFFFFFFF;
    const wiced_time_t range_low_quarter = 0x4000000;

    message_t *entry;

    // Loop through the list discarding all entries.

    while( ( entry = list_pop_before( biggest_time, list ) ) ) {
    	msg_release( entry );
    }

    // Discard any entries where the time may have rolled over.

    while( ( entry = list_pop_before( range_low_quarter, list ) ) ) {
    	msg_release( entry );
    }

    // This should never happen but just check to make sure the list is empty.

    if ( ( list->head != NULL ) || ( list->tail != NULL ) ) {
    	imx_printf( "Failed to release all list members in list_release_all function.\r\n" );
    }
}

/**
 * Ensure that the data array in the "coap" message struct is large enough to contain "min_bytes".
 * Resize to the best fitting available free data array if a better fitting array size is available.
 * Truncate data in array if needed.
 *
 * written by Eric Thelin 27 February 2016
 */
wiced_result_t coap_msg_resize( coap_message_t* coap, uint16_t min_bytes )
{
	if ( coap == NULL ) {
		imx_printf( "NULL passed to coap_msg_resize function.\r\n" );
		return WICED_ERROR;
	}
    wiced_result_t result, return_result = WICED_ERROR;
    uint16_t remaining;

	if ( ( coap->data_block != NULL ) &&
			( ( coap->data_block->release_list_for_data_size == NULL ) ||
			  ( coap->data_block->data == NULL ) ) )
	{
		imx_printf( "Attempted to access invalid data block in coap_msg_resize function.\r\n" );
		return WICED_ERROR;
	}

    result = wiced_rtos_lock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to lock list mutex...\r\n" );
		return WICED_ERROR;
    }

    // Search for smallest free data block that is big enough for "min_bytes."
    uint16_t s;
    for( s = 0; s < NUMBER_OF_DATA_SIZES; s++ ) {// Smallest data blocks are listed first.

    	// If found a data block copy old data into it and swap.
    	if ( ( min_bytes <= free_messages_by_size[ s ].data_size ) &&
    	     ( NULL != free_messages_by_size[ s ].msg_data ) ) {

    		if ( coap->data_block != NULL ) {// copy data into new message data block

				// The currently attached data block is the best we can do, don't copy & resize.
				if ( ( free_messages_by_size[ s ].data_size >= coap->data_block->release_list_for_data_size->data_size ) &&
					 ( min_bytes <= coap->data_block->release_list_for_data_size->data_size ) ) {
					return_result = WICED_SUCCESS;

					goto unlock_mutex;
				}

				// Initialize new data block to 0
				memset( free_messages_by_size[ s ].msg_data->data, 0, free_messages_by_size[ s ].data_size );

				// if the old message does not fit copy as much as possible
    			if ( coap->msg_length > free_messages_by_size[ s ].data_size ) {
    				coap->msg_length = free_messages_by_size[ s ].data_size;
    			}

    			memmove( free_messages_by_size[ s ].msg_data->data, coap->data_block->data, coap->msg_length );
    		}

    		// Release old data block from message.
    		message_size_t *release_list = coap->data_block->release_list_for_data_size;
    		coap->data_block->next = release_list->msg_data;
    		release_list->msg_data = coap->data_block;
    		coap->data_block = NULL;

    		// Assign new data block.
    		coap->data_block = free_messages_by_size[ s ].msg_data;
    		free_messages_by_size[ s ].msg_data = coap->data_block->next;
    		coap->data_block->next = NULL;

    		// Store minimum size of list of free blocks.

    	    remaining = blocks_remaining_in_free_messages_by_size( s );
    		if ( remaining < free_messages_by_size[ s ].min_free_list_size ) {
    			 free_messages_by_size[ s ].min_free_list_size = remaining;
    		}

    		return_result = WICED_SUCCESS;
    		goto unlock_mutex;
    	}
    }
    imx_printf( "Unable to find big enough memory block(%u bytes) in coap_msg_resize function.\r\n", min_bytes );
    return_result = WICED_ERROR;

unlock_mutex:
    result = wiced_rtos_unlock_mutex( &list_mutex );   // All use same mutex for simplicity
    if( result != WICED_SUCCESS ) {
        imx_printf( "Unable to unlock list mutex...\r\n" );
    }
    return return_result;
}

/**
 * Return the size of the data array in the "coap" message struct.
 *
 * written by Eric Thelin 27 February 2016
 */
uint16_t coap_msg_data_size( coap_message_t* coap )
{
	if ( coap == NULL ) {
		imx_printf( "NULL passed to coap_msg_data_size function.\r\n");
		return 0;
	}
	if ( coap->data_block == NULL ) {// OK: No data block means no data.
		return 0;
	}
	if ( coap->data_block->release_list_for_data_size == NULL ) {
		imx_printf( "Data block missing required release list in coap_msg_data_size function.r\n" );
		return 0;
	}
	return coap->data_block->release_list_for_data_size->data_size;
}

/**
 * Return a pointer to the beginning of the payload in coap->data_block->data.
 * Return NULL if there is no payload.
 *
 * NOTE: This function assumes that 0xFFFF is not an Option number used in the coap message and
 * that the option insertion point for inserting 0xFFFF will be after all options in the message,
 * so the insertion point would be the correct place to put the payload start marker 0xFF if there is a payload.
 * Also coap->msg_length must accurately reflect the end of the payload if there is one.
 *
 * written by Eric Thelin 31 March 2016
 */
void * coap_msg_payload( coap_message_t* coap )
{
	uint16_t payload_start_index;

	if ( ( coap != NULL ) && ( coap->data_block != NULL ) && ( coap->data_block->data != NULL ) ) {// Find payload start.

		if ( coap_find_numeric_option( 0xFFFF, coap, NULL, &payload_start_index, NULL ) == WICED_NOT_FOUND ) {

			if ( ( payload_start_index <  coap->msg_length - 1 ) &&// Must have room for both payload start marker and 1 byte of payload.
					( coap->data_block->data[ payload_start_index ] == PAYLOAD_START ) )
			{
				return &( coap->data_block->data[payload_start_index + 1] );// Skip past the payload start marker.
			}
		}
	}
	imx_printf( "Unable to find a payload in coap_msg_payload.\r\n" );
	return NULL;
}

/**
 * Manage freeing of memory.
 * For single use "is_temporary" messages, dispose of the memory malloced for it.
 * All other messages are freed by adding them to free_list.
 *
 * msg must have already been removed from any list before calling this routine.
 * All data in msg might be destroyed.
 *
 * written by Eric Thelin 22 December 2015
 */
void list_release(message_list_t *free_list, message_t *msg)
{
	UNUSED_PARAMETER( free_list );

	msg_release( msg );
}

/**
 * Return NULL if not enough memory to allocate a new message_t structure.
 * OTHERWISE return the newly allocated message_t structure with the is_temporary bit flag set to 1.
 *
 * PURPOSE: create message structures that will be freed when list_release() is executed on them.
 * Some message_t structures are allocated in coap_setup.c. Those messages are
 * added to list_free instead when list_release() is called.
 *
 * written by Eric Thelin 22 December 2015
 *
message_t *list_create_temporary_message( void )
{
    message_t *new_msg;

    if ( ( new_msg = (message_t *) malloc( sizeof( message_t) ) ) ) {
        memset( new_msg, 0x00, sizeof( message_t) );
        new_msg->coap.is_temporary_bit_flag = 1; // deleted
    }
    else {
        imx_printf( "Out of memory\r\n" );
    }

    return new_msg;
}
*/

/**
 * If entry is not NULL and entry is confirmable and has been transmitted at least one time
 * and matches all the other parameters RETURN true
 * OTHERWISE false.
 *
 * written by Eric Thelin 22 December 2015
*/
uint8_t confirmable_match( message_t *entry, wiced_ip_address_t *remote_ip, uint16_t remote_port, uint16_t id)
{
    if ( entry == NULL ) {
        return WICED_FALSE;
    }
    if ( ( CONFIRMABLE == entry->coap.header.t ) &&
         ( 0 < entry->coap.send_attempts ) &&
         ( id == entry->coap.header.id ) &&
         ( remote_ip->ip.v4 == entry->coap.ip_addr.ip.v4 ) &&
         ( remote_port == entry->coap.port ) ) {
        return WICED_FALSE;
    }
    return WICED_FALSE;
}

/**
 * Remove and return the last confirmable match in xmit_list to the parameters after xmit_list.
 * Only remove one entry -- the one closest to the tail.
 *
 * written by Eric Thelin 22 December 2015
 */
message_t *list_pop_confirmable_match( message_list_t *xmit_list, wiced_ip_address_t *remote_ip,
        uint16_t remote_port, uint16_t id )
{
    //hunt through list backwards to find oldest matching message.
    message_t *entry = xmit_list->tail;

    while ( ( entry != NULL ) && ( confirmable_match( entry, remote_ip,
            remote_port, id ) == WICED_FALSE ) ) {
        entry = entry->header.prev;
    }
    if ( confirmable_match( entry, remote_ip, remote_port, id ) ) {
        message_t *prev = entry->header.prev;
        message_t *next = entry->header.next;

        next->header.prev = prev;
        prev->header.next = next;

        if ( xmit_list->head == entry ) {
            xmit_list->head = next;
        }
        if ( xmit_list->tail == entry ) {
            xmit_list->tail = prev;
        }

        entry->header.prev = NULL;
        entry->header.next = NULL;

        return entry;
    }
    return NULL;
}

/**
  * @brief  Dump the contents to the serial port
  * @param  pointer to message
  * @retval : none
  */
void print_msg( message_t *msg )
{
    uint16_t i;
    bool payload = false;


   PRINTF( "Message Contents\r\n");
   PRINTF( "  Next: %lu, Prev %lu\r\n", (uint32_t)msg->header.next, (uint32_t) msg->header.prev );
   PRINTF( "  CoAP: IP Address: %u.%u.%u.%u, Port: %u\r\n", (uint16_t) ( ( (uint32_t)( msg->coap.ip_addr.ip.v4 ) >> 24 ) & 0xff ),
            (uint16_t) ( ( (uint32_t)( msg->coap.ip_addr.ip.v4) >> 16 ) & 0xff ),
            (uint16_t) ( ( (uint32_t)( msg->coap.ip_addr.ip.v4) >> 8 ) & 0xff ),
            (uint16_t) ( ( (uint32_t)( msg->coap.ip_addr.ip.v4) >> 0 ) & 0xff ), msg->coap.port );
    PRINTF( "        Header: Ver: %u, Type: %u, Code: 0x%02x, Message ID: %u\r\n", msg->coap.header.ver, msg->coap.header.t, msg->coap.header.code, msg->coap.header.id );
    PRINTF( "        Message Length: %u: ", msg->coap.msg_length );
    for( i = 0; i < msg->coap.msg_length; i++ ) {
        if( msg->coap.data_block->data[ i ] == 0xff )
            payload = true;
        if( ( isprint( msg->coap.data_block->data[ i ] ) == true ) && ( payload == false ) )
            PRINTF( "%c", msg->coap.data_block->data[ i ] );
        else
            PRINTF( "[0x%02x]", msg->coap.data_block->data[ i ] );
    }
    PRINTF( "\r\n" );

}

int list_size( message_list_t *list )
{
    message_t* entry = list->head;

    if ( entry == NULL ) {
        return 0;
    }
    int size = 0;
    while ( entry != NULL ) {
        size++;
        entry = entry->header.next;
    }
    return size;
}

void dump_lists(void)
{

	/*
	 * Dump out the lists to see their structure
	 */
	cli_print( "Free List:\r\n" );
	dump_list( &list_free );
	cli_print( "UDP Recv List:\r\n" );
	dump_list( &list_udp_coap_recv );
	cli_print( "UDP Xmit List:\r\n" );
	dump_list( &list_udp_coap_xmit );
	cli_print( "TCP Recv List:\r\n" );
    dump_list( &list_tcp_coap_recv );
    cli_print( "TCP Xmit List:\r\n" );
    dump_list( &list_tcp_coap_xmit );

}

void dump_list( message_list_t *list )
{
	message_t* entry = list->head;

	    if ( entry == NULL ) {
	        cli_print( "No Entries\r\n" );
	    }
	    while ( entry != NULL ) {
	        cli_print( "Entry: 0x%08lX\r\n", (uint32_t) entry);
	        entry = entry->header.next;
	    }
}

/**
 * Return the total number of times that there were no message_t structs available when one was required.
 *
 * written by Eric Thelin 5 April 2016
 */
uint16_t get_messaging_list_empty_errors()
{
	return message_list_empty_errors;
}

/**
 * Return the total number of times that it was impossible to assign any data block large enough
 * to a message_t struct since boot.
 *
 * written by Eric Thelin 5 April 2016
 */
uint16_t get_block_not_found_errors()
{
	return block_not_found_errors;
}

/**
 * There are lists of data blocks of different sizes. Each data block may be assigned to a message_t struct.
 * "index" is a 0 based index that indicates which list is being referenced.
 * The number of bytes in the data array of every block in the list is the function's return value.
 * "smallest_freelist_size" is returned with the smallest number of free blocks in the list since boot.
 * "errors" is returned with the number of times a data block of this size
 * would have been the best fit to assign to a message_t struct since boot,
 * but could not be assigned because the list was empty.
 *
 * written by Eric Thelin 5 APRIL 2016
 */
uint16_t get_block_list_details( uint16_t index, uint16_t *smallest_freelist_size, uint16_t *errors )
{
	//static message_size_t free_messages_by_size[ NUMBER_OF_DATA_SIZES ];// one list for each size of msg data.
    if ( index >= NUMBER_OF_DATA_SIZES ) {
    	return 0;
    }
    *errors = free_messages_by_size[ index ].list_empty_errors;
    *smallest_freelist_size = free_messages_by_size[ index ].min_free_list_size;
    return free_messages_by_size[ index ].data_size;
}

/**
 * Return the length of the list of free blocks in free_messages_by_size[ index ]
 *
 * written by Eric Thelin 5 April 2016
 */
static uint16_t blocks_remaining_in_free_messages_by_size( uint16_t index )
{
	uint16_t count = 0;
	message_data_block_t *block = free_messages_by_size[ index ].msg_data;
	while ( block != NULL ) {
		count++;
		block = block->next;
	}
	return count;
}

/**
 * List the number of available data blocks for each possible size of data block.
 *
 * written by Eric Thelin March 2016
 */
void print_free_msg_sizes()
{
	uint16_t s;
	cli_print( "Free Sizes: ");
	for ( s = 0; s < NUMBER_OF_DATA_SIZES; s++ ) {
/*
		uint16_t count = 0;
		message_data_block_t *block = free_messages_by_size[ s ].msg_data;
		while ( block != NULL ) {
			count++;
			block = block->next;
		}
*/
		cli_print("%u[ %u ] ", free_messages_by_size[ s ].data_size, blocks_remaining_in_free_messages_by_size( s ) );
	}
	cli_print( "\r\n" );
}

uint16_t max_packet_size(void)
{
	return HUGE_DATA_BYTES;
}
