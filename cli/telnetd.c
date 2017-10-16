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
 *	telnet daemon
 *
 *  Created on: February, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include "wiced.h"

#include "../storage.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../time/ck_time.h"
#include "messages.h"
#include "telnetd.h"
#include "interface.h"
#include "cli.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define TELNET_BUFFER_LENGTH		64		// Must be a power of 2
#define TELNET_SERVER_LISTEN_PORT	23
#define TELNET_TIME_OUT				(30 * 60 )		// 2 mins during debugging
/*
 * Telnet commands
 */
#define TELNET_SE                  	240    // End of sub negotiation parameters.
#define TELNET_NOP                 	241    // No operation.
#define TELNET_Data_Mark           	242    // The data stream portion of a Synch.This should always be accompanied by a TCP Urgent notification.
#define TELNET_Break               	243    // NVT character BRK.
#define TELNET_Interrupt_Process   	244    // The function IP.
#define TELNET_Abort_output        	245    // The function AO.
#define TELNET_Are_You_There       	246    // The function AYT.
#define TELNET_Erase_character     	247    // The function EC.
#define TELNET_Erase_Line          	248    // The function EL.
#define TELNET_Go_ahead            	249    // The GA signal.
#define TELNET_SB                  	250    // Indicates that what follows is sub negotiation of the indicated option.
#define TELNET_WILL					251    // Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
#define TELNET_WONT 				252    // Indicates the refusal to perform, or continue performing, the indicated option.
#define TELNET_DO 					253    // Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
#define TELNET_DONT 				254    // Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
#define TELNET_IAC                 	255    // Data Byte 255.
/*
 * Telnet Options - http://www.thoughtproject.com/libraries/telnet/Reference/Options.htm
 */
#define TRANSMIT_BINARY		00
#define ECHO				01
#define SUPPRESS_GO_AHEAD	03
#define STATUS				05
#define TIMING_MARK			06
#define NAOCRD				10
#define NAOHTS				11
#define NAOHTD				12
#define NAOFFD				13
#define NAOVTS				14
#define NAOVTD				15
#define NAOLFD				16
#define EXTEND_ASCII		17
#define LOGOUT				18
#define BM					19
#define DET					20
#define SEND_LOCATION		23
#define TERMINAL_TYPE		24
#define END_OF_RECORD		25
#define TUID				26
#define OUTMRK				27
#define TTYLOC				28
#define T3270_REGIME		29
#define X_3_PAD				30
#define NAWS				31
#define TERMINAL_SPEED		32
#define TOGGLE_FLOW_CONTROL	33
#define LINEMODE			34
#define X_DISPLAY_LOCATION	35
#define ENVIRON				36
#define AUTHENTICATION		37
#define ENCRYPT				38
#define NEW_ENVIRON			39
#define TN3270E				40
#define CHARSET				42
#define COM_PORT_OPTION		44
#define KERMIT				47
/*
 * Telnet Response strings
 */
#define WILL_MSG			"\xFF\xFB"
#define WONT_MSG			"\xFF\xFC"
#define DO_MSG				"\xFF\xFD"
#define DONT_MSG			"\xFF\xFE"
/******************************************************
 *                   Enumerations
 ******************************************************/
enum TELNET_STATES {
	TELNET_INIT,
	TELNET_MONITOR,
	TELNET_CLOSE_PORT,
	TELNET_IDLE
};

enum TELNET_MSGS {
	TELNET_MSG_TIMEOUT,
	TELNET_MSG_DISCONNECT,
	TELNET_MSG_CONNECT,
	TELNET_MSG_CONNECT_DISCONNECT,
	TELNET_MSG_CONNECT_ABORT,
	TELNET_MSG_FAILED_TO_SEND,
	TELNET_MSG_UNABLE_TO_DISCONNECT,
	TELNET_MSG_UNABLE_TO_STOP,
	TELNET_MSG_UNABLE_TO_DEINIT,
	TELNET_MSG_FAILED_TO_START,
	TELNET_MSG_FAILED_TO_INITIALIZE,
	TELNET_MSG_SERVER_STARTED,
	TELNET_MSG_FAILED_TO_FLUSH,
	TELNET_MSG_SERVER_CONNECT_CALLBACK,
	TELNET_MSG_SERVER_CONNECT_CALLBACK_FAILED,
	TELNET_MSG_SERVER_DISCONNECT_CALLBACK,
	TELNET_MSG_SERVER_DISCONNECT_CALLBACK_FAILED,
	NO_TELNET_MSGS
};
/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef enum {
	SOCKET_MESSAGE_EVENT,
	SOCKET_CONNECT_EVENT,
	SOCKET_DISCONNECT_EVENT,
} telnet_tcp_server_event_t;

typedef struct {
	wiced_tcp_socket_t			*socket;
	telnet_tcp_server_event_t	event_type;
} server_event_message_t;

typedef struct {
	wiced_tcp_server_t	tcp_server;
	wiced_thread_t		thread;
} telnet_tcp_server_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
char * print_telnet_msgs[ NO_TELNET_MSGS ] = {
		"Telnet Timeout: Disconnecting...\r\n",
		"Telnet: Disconnect\r\n",
		"Telnet: Connect request\r\n",
		"Telnet: Connect request\r\nDisconnecting previous connection\r\n",
		"Connection Accepted\r\n",
		"Failed to send to TCP Stream\r\n",
		"Unable to disconnect Telnetd socket...\r\n",
		"Unable to stop Telnetd server...\r\n",
		"Unable to deinit Telnetd queue...\r\n",
		"Failed to start Telnet Server\r\n",
		"Failed to initialize queue\r\n",
		"Telnet TCP server started. Listening on port 23\r\n",
		"Failed to flush TCP Stream with error code\r\n"
		"Telnet Server Connect Callback successfully added to queue\r\n",
		"Telnet Server Connect Callback Failure to add to queue\r\n",
		"Telnet Server Disconnect Callback successfully added to queue\r\n",
		"Telnet Server Disconnect Callback Failure to add to queue\r\n",
};

struct TELNET_CONFIG {
	uint16_t state;
	uint16_t start;
	uint16_t length;
	uint8_t buffer[ TELNET_BUFFER_LENGTH ];
	wiced_tcp_server_t tcp_server;
    wiced_tcp_socket_t *socket;
    wiced_tcp_stream_t tcp_stream;
    wiced_time_t last_telnet_time;
    unsigned int processing_IAC 			: 1;
    unsigned int processing_will			: 1;
    unsigned int processing_wont			: 1;
    unsigned int processing_do				: 1;
    unsigned int processing_dont			: 1;
    unsigned int local_suppress_go_ahead	: 1;
    unsigned int remote_suppress_go_ahead	: 1;
    unsigned int local_echo					: 1;
    unsigned int remote_echo				: 1;
	unsigned int active 					: 1;
	unsigned int quit						: 1;

} telnet_config = { .state = TELNET_INIT, .active = 0, .socket = NULL, .buffer = {0} };

static uint16_t arg;
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;

/******************************************************
 *               Function Definitions
 ******************************************************/
static wiced_result_t telnet_tcp_server_connect_callback( wiced_tcp_socket_t* socket, void* arg );
static wiced_result_t telnet_tcp_server_disconnect_callback( wiced_tcp_socket_t* socket, void* arg );
static wiced_result_t telnet_tcp_server_receive_callback( wiced_tcp_socket_t* socket, void* arg );

static wiced_queue_t server_event_queue;
/**
  * @brief	telnetd_init()
  * @param  None
  * @retval : None
  */
void telnetd_init(void)
{
	switch( telnet_config.state ) {

	    // Start the telnet server on the first call to telnetd

    	case TELNET_IDLE:
        case TELNET_INIT:
    	    telnet_config.state = TELNET_INIT;
            break;

        // If telnet is already running, do not restart

        case TELNET_MONITOR:
        case TELNET_CLOSE_PORT:
    	    ;
    	    break;

        default:
        	break;
 	}
}
/**
* @brief	telnetd_deinit()
* @param  None
* @retval : None
*/
void telnetd_deinit(void)
{
	switch( telnet_config.state ) {

	    // Stop the telnet server immediately if it is currently running or about to stop.

	    case TELNET_MONITOR:
	    case TELNET_CLOSE_PORT:
		    telnet_config.state = TELNET_CLOSE_PORT;
		    telnetd();
		    break;

		// If the telnet server has not started yet, do not let it start.

	    case TELNET_INIT:
	    case TELNET_IDLE:
	    	telnet_config.state = TELNET_IDLE;
	    	break;

	    default:
	    	break;
	}

}
/**
* @brief	telnet_active
* @param  None
* @retval : status of telnet
*/

uint16_t telnet_active(void)
{
	return( telnet_config.active );
}

void telnetd(void)
{

	wiced_interface_t interface;
	wiced_result_t result;
    uint8_t local_buffer[ TELNET_BUFFER_LENGTH ], *reply_string;
    uint16_t i;
    uint32_t local_buffer_length;
	server_event_message_t current_event;
	wiced_time_t time = 0;

	wiced_time_get_time( &time );// Need current time to see if telnet session should time out.

	switch( telnet_config.state ) {
		case TELNET_MONITOR :
			if( wiced_rtos_pop_from_queue(&server_event_queue, &current_event, 0) != WICED_SUCCESS ) {

				// Check for timeout.

				if( ( telnet_config.socket != NULL ) && ( timer_timeout( time, telnet_config.last_telnet_time, TELNET_TIME_OUT * 1000 ) ) ) {
					icb.print_msg |= MSG_TELNET;
					icb.print_telnet_msg = TELNET_MSG_TIMEOUT;
			    	telnetd_write( "Telnet Timeout due to inactivity.\r\n>", 36 );
			    	result =  wiced_tcp_stream_flush( &telnet_config.tcp_stream );
					wiced_tcp_server_disconnect_socket( &telnet_config.tcp_server, telnet_config.socket );//current_event.socket);
					telnet_config.socket = NULL;
					memset( telnet_config.buffer, 0, TELNET_BUFFER_LENGTH );
					telnet_config.active = false;
				}
			}
			else {// Got an event off the queue.

				switch (current_event.event_type) {
					case SOCKET_DISCONNECT_EVENT:
						icb.print_msg |= MSG_TELNET;
						icb.print_telnet_msg = TELNET_MSG_DISCONNECT;
						wiced_tcp_server_disconnect_socket(&telnet_config.tcp_server, current_event.socket);
						telnet_config.socket = NULL;
						memset( telnet_config.buffer, 0, TELNET_BUFFER_LENGTH );
						telnet_config.active = false;
						break;
					case SOCKET_CONNECT_EVENT:
						icb.print_msg |= MSG_TELNET;
						icb.print_telnet_msg = TELNET_MSG_CONNECT;
						if( telnet_config.active == true ) {
							icb.print_msg |= MSG_TELNET;
							icb.print_telnet_msg = TELNET_MSG_CONNECT_DISCONNECT;
							wiced_tcp_server_disconnect_socket(&telnet_config.tcp_server, telnet_config.socket);
							telnet_config.socket = NULL;
							memset( telnet_config.buffer, 0, TELNET_BUFFER_LENGTH );
						}
						wiced_tcp_server_accept(&telnet_config.tcp_server, current_event.socket);
						telnet_config.socket = current_event.socket;	// Use this when we support multiple instances
						telnet_config.active = true;
						result = wiced_tcp_stream_init( &telnet_config.tcp_stream, telnet_config.socket );
						if( result == WICED_TCPIP_SUCCESS ) {
							/*
							 * Set current modes
							 */
							telnet_config.processing_IAC 			=
							telnet_config.processing_will			=
							telnet_config.processing_wont			=
							telnet_config.processing_do				=
							telnet_config.processing_dont			=
							telnet_config.local_suppress_go_ahead 	=
							telnet_config.remote_suppress_go_ahead 	=
							telnet_config.local_echo 				=
							telnet_config.remote_echo 				= false;
						    wiced_time_get_time( &telnet_config.last_telnet_time );

						    icb.print_msg |= MSG_TELNET_ACCEPT;
							/*
							 * Tell the remote what settings we want
							 */
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, DONT_MSG, 2 );
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, "\x01", 1 );		// DONT ECHO
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, DONT_MSG, 2 );
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, "\x01", 1 );		// DONT FLOWCONTROL
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, DO_MSG, 2 );
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}
			    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, "\x03",  1 );		// DO SUPPORESS GO AHEAD OPTION
			    			if ( result != WICED_TCPIP_SUCCESS ) {
			    				icb.print_msg |= MSG_TELNET;
			    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
			    			}

	    			    	telnetd_write( "IoT Device Command Line Processor\r\n>", 36 );
	    			    	result =  wiced_tcp_stream_flush( &telnet_config.tcp_stream );
						    wiced_time_get_time( &telnet_config.last_telnet_time );	// Last time we got data
						}
						break;
					case SOCKET_MESSAGE_EVENT:
						result = wiced_tcp_stream_read_with_count( &telnet_config.tcp_stream, local_buffer, TELNET_BUFFER_LENGTH, 0, &local_buffer_length );
						if( result == WICED_TCPIP_SUCCESS ) { // Got some data process it - First off this will be the negotiation
							// imx_printf( "\r\nReceived: %lu Bytes >", local_buffer_length );
					    	for( i = 0; i < local_buffer_length; i++ ) {
					    		/*
					    		if( isprint( local_buffer[ i ] ) )
					    			imx_printf( "%c", local_buffer[ i ] );
					    		else */
					    			// imx_printf( "[0x%02x]", local_buffer[ i ] );

					    		if( telnet_config.processing_IAC == true ) {
					    			switch( (uint16_t ) local_buffer[ i ] ) {
					    				case TELNET_SE :
					    					break;
					    				case TELNET_NOP : 				// No operation.
					    					break;
					    				case TELNET_Data_Mark :			// The data stream portion of a Synch.This should always be accompanied by a TCP Urgent notification.
				    						break;
					    				case TELNET_Break :				// NVT character BRK.
					    					break;
					    				case TELNET_Interrupt_Process :	// The function IP.
											break;
					    				case TELNET_Abort_output :		// The function AO.
											break;
					    				case TELNET_Are_You_There :		// The function AYT.
											break;
					    				case TELNET_Erase_character :	// The function EC.
											break;
					    				case TELNET_Erase_Line :		// The function EL.
											break;
					    				case TELNET_Go_ahead :			// The GA signal.
											break;
					    				case TELNET_SB :				// Indicates that what follows is sub negotiation of the indicated option.
											break;
					    				case TELNET_WILL :				// Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
					    					telnet_config.processing_will = true;
					    					break;
					    				case TELNET_WONT:				// Indicates the refusal to perform, or continue performing, the indicated option.
					    					telnet_config.processing_wont = true;
					    					break;
					    				case TELNET_DO:					// Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
					    					telnet_config.processing_do = true;
					    					break;
					    				case TELNET_DONT:				// Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
					    					telnet_config.processing_dont = true;
					    					break;
					    				case TELNET_IAC	:				// Data byte 255
							    			telnet_config.processing_IAC = true;
											break;
					    				default :
					    					break;
					    			}
					    			telnet_config.processing_IAC = false;
					    		} else if ( telnet_config.processing_will == true ) {
					    			/*
					    			 * Only ack modes we support
					    			 */
					    			reply_string = NULL;
					    			if( local_buffer[ i ] == SUPPRESS_GO_AHEAD ) {
					    				if( telnet_config.remote_suppress_go_ahead == false ) {	// Only ack a change
						    				telnet_config.remote_suppress_go_ahead = true;
						    				reply_string = (uint8_t *) WILL_MSG;
					    				}
					    			} else if( local_buffer[ i ] == ECHO ) {
					    				reply_string = (uint8_t *) WONT_MSG;	// We are not supporting this
					    			} else
					    				 reply_string = (uint8_t *) WONT_MSG;
									telnet_config.processing_will = false;
					    			goto send_response;
					    		} else if ( telnet_config.processing_wont == true ) {
					    			/*
					    			 * Only ack modes we support
					    			 */
					    			reply_string = NULL;
					    			if( local_buffer[ i ] == SUPPRESS_GO_AHEAD ) {
					    				if( telnet_config.remote_suppress_go_ahead == true ) {	// Only ack a change
						    				telnet_config.remote_suppress_go_ahead = false;
						    				reply_string = (uint8_t *) WONT_MSG;
					    				} else
					    					continue;
					    			} else if( local_buffer[ i ] == ECHO ) {
					    				if( telnet_config.local_echo == true ) {	// Only ack a change
						    				telnet_config.local_echo = false;
						    				reply_string = (uint8_t *) WONT_MSG;
					    				}
					    			} else
					    				 reply_string = (uint8_t *) WONT_MSG;
									telnet_config.processing_wont = false;
					    			goto send_response;
					    		} else if ( telnet_config.processing_do == true ) {
					    			/*
					    			 * Only ack modes we support
					    			 */
					    			reply_string = NULL;
					    			if( local_buffer[ i ] == SUPPRESS_GO_AHEAD ) {
					    				if( telnet_config.local_suppress_go_ahead == false ) {	// Only ack a change
						    				telnet_config.remote_suppress_go_ahead = true;
						    				reply_string = (uint8_t *) WILL_MSG;
					    				} else
					    					continue;
					    			} else if( local_buffer[ i ] == ECHO ) {
					    				if( telnet_config.local_echo == false ) {
					    					telnet_config.local_echo = true;
					    					reply_string = (uint8_t *) WILL_MSG;	// We are supporting this
					    				} else
					    					continue;	// Already set no ack
					    			} else
					    				 reply_string = (uint8_t *) WONT_MSG;
									telnet_config.processing_do = false;
				    				goto send_response;
					    		} else if ( telnet_config.processing_dont == true ) {
					    			reply_string = NULL;
					    			if( local_buffer[ i ] == SUPPRESS_GO_AHEAD ) {
					    				if( telnet_config.local_suppress_go_ahead == true ) {	// Only ack a change
						    				telnet_config.local_suppress_go_ahead = false;
						    				reply_string = (uint8_t *) WONT_MSG;
					    				} else
					    					continue;
					    			} else if( local_buffer[ i ] == ECHO ) {
					    				if( telnet_config.local_echo == false ) {
					    					telnet_config.local_echo = false;
					    					reply_string = (uint8_t *) DONT_MSG;	// We are supporting this
					    				} else
					    					continue;	// Already set no ack
					    			} else
					    				 reply_string = (uint8_t *) WONT_MSG;
									telnet_config.processing_dont = false;
				    				goto send_response;
					    		} else {
					    			if( local_buffer[ i ] == TELNET_IAC )
					    				telnet_config.processing_IAC = true;
					    			else {
					    				/*
					    				 * Do we echo it?
					    				 */
					    				if( telnet_config.local_echo == true ) {
					    					result = wiced_tcp_stream_write( &telnet_config.tcp_stream, &local_buffer[ i ], (uint32_t) 1 );
					    					if ( result != WICED_TCPIP_SUCCESS ) {
							    				icb.print_msg |= MSG_TELNET;
							    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
					    					}
					    				}
						    			/*
						    			 * Just a regular character - add to buffer to be processed in CLI
						    			 */
					    				if( telnet_config.length == TELNET_BUFFER_LENGTH )
					    					;	// Drop Character
					    				else {
					    					// imx_printf( "Adding character [0x%02x] at start: %u and length: %u\r\n", (uint16_t ) local_buffer[ i ], telnet_config.start, telnet_config.length );
						    				if( telnet_config.start + telnet_config.length > TELNET_BUFFER_LENGTH )
						    					telnet_config.buffer[ ( telnet_config.start + telnet_config.length ) - TELNET_BUFFER_LENGTH ] = local_buffer[ i ];
						    				else
						    					telnet_config.buffer[ ( telnet_config.start + telnet_config.length ) ] = local_buffer[ i ];
					    					telnet_config.length += 1;
					    				}
					    			}
					    		}
					    		continue;
send_response:
				    			if( reply_string != NULL ) {
					    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, reply_string, (uint32_t) strlen( (char *) reply_string ) );
					    			if ( result != WICED_TCPIP_SUCCESS ) {
					    				icb.print_msg |= MSG_TELNET;
					    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
					    				telnet_config.state = TELNET_CLOSE_PORT;
					    			}
					    			result = wiced_tcp_stream_write( &telnet_config.tcp_stream, &local_buffer[ i ], (uint32_t) 1 );
					    			if ( result != WICED_TCPIP_SUCCESS ) {
					    				icb.print_msg |= MSG_TELNET;
					    				icb.print_telnet_msg = TELNET_MSG_FAILED_TO_SEND;
					    				telnet_config.state = TELNET_CLOSE_PORT;
					    			}
				    			}

					    	}// End of for loop.

					    	// imx_printf( "<\r\n" );
					    	result =  wiced_tcp_stream_flush( &telnet_config.tcp_stream );
						    wiced_time_get_time( &telnet_config.last_telnet_time );	// Last time we got data
					    	return;
						}
						break;
					default:
						break;
				}
			}
			break;
		case TELNET_CLOSE_PORT :
			if( telnet_config.active == true ) {
				if( wiced_tcp_server_disconnect_socket( &telnet_config.tcp_server, telnet_config.socket ) != WICED_SUCCESS ) {
					icb.print_msg |= MSG_TELNET;
					icb.print_telnet_msg = TELNET_MSG_UNABLE_TO_DISCONNECT;
				}
			}
			if( wiced_tcp_server_stop(&telnet_config.tcp_server) != WICED_SUCCESS ) {
				icb.print_msg |= MSG_TELNET;
				icb.print_telnet_msg = TELNET_MSG_UNABLE_TO_STOP;
			};
			if( wiced_rtos_deinit_queue(&server_event_queue) != WICED_SUCCESS ) {
				icb.print_msg |= MSG_TELNET;
				icb.print_telnet_msg = TELNET_MSG_UNABLE_TO_DEINIT;
			}
			telnet_config.active = false;
			telnet_config.state = TELNET_INIT;
			break;
		case TELNET_INIT :
			if( icb.wifi_up ) {
				telnet_config.length = 0;
				telnet_config.start = 0;
				// Normal Operating Mode - connect to a remote AP and use that connection for all communications.
				if( device_config.AP_setup_mode == true )
					interface = WICED_AP_INTERFACE;
				else
					interface = WICED_STA_INTERFACE;
			    /*
			     *  Create a TCP telnet server socket
			     */
				memset( &telnet_config.tcp_server, 0x00, sizeof( telnet_config.tcp_server ) );

				/*
				 * Initialize the server - only 1 socket
				 */
				result = wiced_tcp_server_start( &telnet_config.tcp_server, interface, TELNET_SERVER_LISTEN_PORT, 1,
												 telnet_tcp_server_connect_callback,
											 	 telnet_tcp_server_receive_callback,
												 telnet_tcp_server_disconnect_callback, (void*) &arg );
				if (result != WICED_SUCCESS) {
					icb.print_msg |= MSG_TELNET;
					icb.print_telnet_msg = TELNET_MSG_FAILED_TO_START;
					return;
				}

				result = wiced_rtos_init_queue(&server_event_queue, NULL, sizeof(server_event_message_t), 20);
				if (result != WICED_SUCCESS) {
					icb.print_msg |= MSG_TELNET;
					icb.print_telnet_msg = TELNET_MSG_FAILED_TO_INITIALIZE;
					goto stop_tcp_server;
				}

				icb.print_msg |= MSG_TELNET;
				icb.print_telnet_msg = TELNET_MSG_SERVER_STARTED;
				telnet_config.state = TELNET_MONITOR;
				return;

	stop_tcp_server:
				wiced_tcp_server_stop(&telnet_config.tcp_server);
			}
			break;
		case TELNET_IDLE :
		default :
			break;
	}
}
/*
 * Only have one event available for each
 */
static wiced_result_t telnet_tcp_server_connect_callback( wiced_tcp_socket_t* socket, void* arg )
{
    static server_event_message_t current_event;
    wiced_result_t result;

    current_event.event_type = SOCKET_CONNECT_EVENT;
    current_event.socket = (wiced_tcp_socket_t *)socket;
    result = wiced_rtos_push_to_queue(&server_event_queue, &current_event, WICED_NO_WAIT);
	icb.print_msg |= MSG_TELNET;
    if (result != WICED_SUCCESS) {
    	icb.print_telnet_msg = TELNET_MSG_SERVER_CONNECT_CALLBACK;
    } else {
    	icb.print_telnet_msg = TELNET_MSG_SERVER_CONNECT_CALLBACK_FAILED;
    }

    return WICED_SUCCESS;

}
static wiced_result_t telnet_tcp_server_disconnect_callback( wiced_tcp_socket_t* socket, void* arg )
{
	static server_event_message_t current_event;
    wiced_result_t result;

	current_event.event_type = SOCKET_DISCONNECT_EVENT;
	current_event.socket = (wiced_tcp_socket_t *)socket;
    result = wiced_rtos_push_to_queue(&server_event_queue, &current_event, WICED_NO_WAIT);
	icb.print_msg |= MSG_TELNET;
    if (result != WICED_SUCCESS) {
    	icb.print_telnet_msg = TELNET_MSG_SERVER_DISCONNECT_CALLBACK;
    } else {
    	icb.print_telnet_msg = TELNET_MSG_SERVER_DISCONNECT_CALLBACK_FAILED;
    }

	return WICED_SUCCESS;
}

static wiced_result_t telnet_tcp_server_receive_callback( wiced_tcp_socket_t* socket, void* arg )
{
	static server_event_message_t current_event;

	current_event.event_type = SOCKET_MESSAGE_EVENT;
	current_event.socket = (wiced_tcp_socket_t *)socket;
	wiced_rtos_push_to_queue(&server_event_queue, &current_event, WICED_NO_WAIT);

	return WICED_SUCCESS;

}

uint16_t telnetd_getch( char *ch )
{
	if( telnet_config.length > 0 ) {
		*ch = (char) telnet_config.buffer[ telnet_config.start ];
		//imx_printf( "Got character [0x%02x] at start: %u and length: %u\r\n", (uint16_t ) *ch, telnet_config.start, telnet_config.length );
		telnet_config.start += 1;
		if( telnet_config.start == TELNET_BUFFER_LENGTH )
			telnet_config.start = 0;	// Wrap
		telnet_config.length -= 1;
		//imx_printf( "length now %u\r\n", telnet_config.length );
		return true;
	} else
		return false;
}

uint16_t telnetd_write( char *buffer, uint16_t length )
{
	wiced_result_t result;

	result = wiced_tcp_stream_write( &telnet_config.tcp_stream, (void *) buffer, (uint32_t) length );
	if ( result != WICED_TCPIP_SUCCESS ) {
		imx_printf( "Failed to send to TCP Stream\r\n" );
		return false;
	}
	result =  wiced_tcp_stream_flush( &telnet_config.tcp_stream );
	if ( result != WICED_TCPIP_SUCCESS ) {
		imx_printf( "Failed to flush TCP Stream with error code: %u\r\n", result );
		return false;
	}
	return true;
}

void print_telnet_state(void)
{
	cli_print( "Telnet State: %s State: %u \r\n", telnet_config.active ? "Active" : "Idle", telnet_config.state );

}
