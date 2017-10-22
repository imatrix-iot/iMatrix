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
 * coap.h
 *
 *  Created on: Dec, 2014
 *      Author: greg.phillips
 */

#ifndef COAP_H_
#define COAP_H_

/** @file coap.h
 *
 *  Define CoAP Constants and structures
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/
#define MSG_CLASS(code)     ( (code & 0xe0 ) >> 5 )
#define MSG_DETAIL(code)    ( code & 0x1F )
/******************************************************
 *                    Constants
 ******************************************************/
#ifdef MULTICAST_DEBUGGING
    #define EXTRA_ENTRIES 1
#else
    #define EXTRA_ENTRIES 0
#endif

uint16_t no_user_coap_entries(void);
#define NO_COAP_ENTRIES     	( no_user_coap_entries() + EXTRA_ENTRIES )
#define MAX_URI_QUERY_STR   	20
#define NO_FREE_MSG_BUFFERS 	10
#define COAP_HEADER_LENGTH  	4
#define MAX_MSG_LENGTH      	1152
#define MAX_PAYLOAD_LENGTH  	1024
#define DEFAULT_COAP_PORT   	5683
#define COAP_RECV_SLEEP     	50
#define BAD_URI_QUERY       	32767
#define MAX_TOKEN_LENGTH        ( 8 )

/*
 * CoAP Message Types
 */
#define CONFIRMABLE         0
#define NON_CONFIRMABLE     1
#define ACKNOWLEDGEMENT     2
#define RESET               3

/*
 * CoAP Class Types
 */
#define REQUEST             0
#define SUCCESS_RESPONSE    2
#define CLIENT_ERROR        4
#define SERVER_ERROR        5
/*
 * Method Codes Class 0
 */
#define GET                 1
#define POST                2
#define PUT                 3
#define DELETE              4
/*
 * CoAP Detail Types Response Codes
 *
 * 0.00
 */
#define EMPTY_MSG           0
/*
 * 1.01-1.31    Reserved
 *
 * 2.xx
 */
#define CREATED                 ( (0x40) | 1 )
#define DELETED                 ( (0x40) | 2 )
#define VALID                   ( (0x40) | 3 )
#define CHANGED                 ( (0x40) | 4 )
#define CONTENT                 ( (0x40) | 5 )
/*
 * 4.xx
 */
#define BAD_REQUEST             ( (0x80) | 0 )
#define UNAUTHORIZED            ( (0x80) | 1 )
#define BAD_OPTION              ( (0x80) | 2 )
#define FORBIDDEN               ( (0x80) | 3 )
#define NOT_FOUND               ( (0x80) | 4 )
#define METHOD_NOT_ALLOWED      ( (0x80) | 5 )
#define NOT_ACCEPTABLE          ( (0x80) | 6 )
#define PRECONDTION_FAILED      ( (0x80) | 12 )
#define REQUEST_ENTITY_TOO_BIG  ( (0x80) | 13 )
#define UNSUPPORTED_CONTENT     ( (0x80) | 15 )
/*
 * 5.xx
 */
#define INTERNAL_SERVER_ERROR   ( (0xA0) | 0 )
#define NOT_IMPLEMENTED         ( (0xA0) | 1 )
#define BAD_GATEWAY             ( (0xA0) | 2 )
#define SERVICE_UNAVAILABLE     ( (0xA0) | 3 )
#define GATEWAY_TIMEOUT         ( (0xA0) | 4 )
#define PROXY_NOT_SUPPORTED     ( (0xA0) | 5 )
/*
 * Internal Control
 */
// No Response should only be used for responses to multicasts. See RFC 7390
#define COAP_NO_RESPONSE                0xFF
#define COAP_SEND_RESPONSE              0xFE
#define COAP_SEND_EMPTY_ACKNOWLEDGEMENT 0xFD // Only used in coap_recv.c for sending an acknowledgement to a separate confirmable response.

// For routines that partially process a message, but may send an error response using the 2 constants above,
#define COAP_MESSAGE_MAY_BE_PROCESSED 0xFD

/*
 * CoAP Option Numbers Registry
 */
#define RESERVED                0x00
#define IF_MATCH                0x01
#define URI_HOST                0x03
#define ETAG                    0x04
#define IF_NONE_MATCH           0x05
#define URI_PORT                0x07
#define LOCATION_PATH           0x08
#define URI_PATH                0x0B
#define CONTENT_FORMAT          0x0C
#define MAX_AGE                 0x0E
#define URI_QUERY               0x0F
#define ACCEPT                  0x12
#define LOCATION_QUERY          0x14
#define BLOCK2                  0x17
#define BLOCK1					0x1B
#define PROXY_URI               0x23
#define PROXY_SCHEME            0x27
#define SIZE1                   0x3C

/*
 * Selected items from CoAP Content-Formats Registry.
 */
#define TEXT_MEDIA_TYPE    0
#define LINK_MEDIA_TYPE   40
#define BINARY_MEDIA_TYPE 42
#define JSON_MEDIA_TYPE   50

/*
 * CoAP Option related defines
 */
#define MAX_URI_LENGTH          256     // 255 + 0x00
#define PAYLOAD_START           0xFF
#define OPTION_LENGTH_8BITS     0x0D
#define OPTION_LENGTH_16BITS    0x0E
#define OPTION_LENGTH_ERROR     0x0F

#define COAP_PAYLOAD_LENGTH     ( MAX_MSG_LENGTH - sizeof( coap_header_t ) )

#define IGNORE_CHECKSUM32       0xFFFFFFFF

/******************************************************
 *                   Enumerations
 ******************************************************/
enum coap_security_types {
    NoSec,
    RawPublicKey
};
/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef uint8_t token_buffer_t[ MAX_TOKEN_LENGTH ];


/******************************************************
 *                    Structures
 ******************************************************/
/*
 * Stucture holding processed info for msg
 */
typedef struct {
    uint8_t blocksize;
    char uri[ MAX_URI_LENGTH ];
    char uri_query[ MAX_URI_LENGTH ];
    char *payload;
    uint16_t payload_length;
} CoAP_msg_detail_t;

typedef struct list_entry {
    struct message *prev;
    struct message *next;
} list_entry_t;

struct describe_message_size;

typedef struct describe_data_block{
	struct describe_message_size *release_list_for_data_size;
	uint8_t* data;
	struct describe_data_block *next;
} message_data_block_t;

typedef struct describe_message_size{
	uint16_t data_size;
	message_data_block_t *msg_data;
	uint8_t min_free_list_size;// For statistical purposes only.
	uint8_t list_empty_errors;//  For statistical purposes only.
} message_size_t;

typedef struct {
    unsigned int tkl        : 4;    // Token Length
    unsigned int t          : 2;    // Type
    unsigned int ver        : 2;    // Version
    unsigned int code       : 8;    // Code - Class & Detail
    unsigned int id         : 16;   // Message ID
} coap_header_t;

typedef struct {
    wiced_ip_address_t ip_addr;
    uint16_t port;
    coap_header_t header;
    uint16_t msg_length;   // Length of data
    message_data_block_t *data_block;//allows the release and assignment of data.
    wiced_ip_address_t my_ip_from_request; //for multicasting, my_IP will be the multicast address instead of the regular one.
    unsigned int send_attempts : 3;
    unsigned int has_payload_bit_flag : 1;
    unsigned int response_processing_method : 3;
    unsigned int received_as_multicast : 1;
    wiced_time_t initial_timestamp, next_timestamp;
} coap_message_t;

typedef struct message {
    list_entry_t header;
    coap_message_t coap;
} message_t;

typedef struct {
    message_t *head;
    message_t *tail;
} message_list_t;

typedef struct {
    char *title;    // Human readable name
    char *rt;       // Resource Type
    char *if_desc;  // Interface Description
    uint32_t sz;    // Max Size Estimate
} CoAP_att_t;

typedef struct {
    char *uri;
    CoAP_att_t att;
    uint16_t arg;
    uint16_t (*get_function)(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
    uint16_t (*put_function)(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
    uint16_t (*post_function)(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
    uint16_t (*delete_function)(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
} CoAP_node_t;

typedef struct {
    list_entry_t header;    // Fill in Pointers after initialization to support inserts and deletes in this list - TBD
    CoAP_node_t node;
} CoAP_entry_t;


/******************************************************
 *               Function Declarations
 ******************************************************/
uint16_t in_my_groups( uint16_t group );
uint16_t add_options_from_string( uint16_t option_number, char separator, char *str_in, uint16_t *current_option_number, uint8_t *buffer, uint16_t buf_length );
uint16_t add_coap_uint_option( uint16_t option_number, uint16_t option_value, uint16_t *current_option_number, uint8_t *buffer, uint16_t buf_length );
wiced_result_t coap_append_response_payload_using_printf( uint16_t media_type, coap_message_t* msg_out, char* format,  ... );
wiced_result_t coap_store_response_header( coap_message_t* msg_out, uint16_t code_in, uint16_t type_in, uint16_t *header_size );
wiced_result_t get_uint_from_query_str( char* name, uint16_t *value, char* query_str );
uint16_t add_coap_str_option( uint16_t option_number, uint16_t *current_option_number, char *uri, uint8_t *buffer, uint16_t buf_length );
wiced_result_t coap_append_response_payload( uint16_t media_type, coap_message_t* msg_out, uint8_t* data, uint16_t size );
wiced_result_t coap_store_response_data(coap_message_t* msg_out, uint16_t code_in, uint16_t type_in, char* data_str, uint16_t media_type );
uint16_t process_coap_msg( message_t *msg, CoAP_msg_detail_t *cd );
uint16_t is_multicast_ip( wiced_ip_address_t *addr );
void * coap_msg_payload( coap_message_t* coap );
message_t *msg_get( uint16_t min_bytes );
uint16_t get_messaging_list_empty_errors(void);
void list_add( message_list_t *list, message_t *entry  );
void list_add_at( wiced_time_t timestamp, message_list_t *list, message_t *new_entry, uint8_t retry );
uint16_t get_block_not_found_errors();
uint16_t get_block_list_details( uint16_t index, uint16_t *smallest_freelist_size, uint16_t *errors );
void* new_coap_token( token_buffer_t token, unsigned int length );
bool coap_transmit_empty(void);
void coap_transmit( uint16_t process_to_end );
/******************************************************
 *               Global Variable Declarations
 ******************************************************/
extern unsigned int random_seed; // single definition and initialization for this global var
//is in coap_setup.c

/******************************************************
 *               Inline Function Definitions
 ******************************************************/

#endif /* COAP_H_ */
