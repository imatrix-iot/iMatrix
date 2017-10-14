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
 * que_manager.h
 *
 *  Created on: Oct 23, 2013
 *      Author: greg.phillips
 */

#ifndef QUE_MANAGER_H_
#define QUE_MANAGER_H_

/** @file
 *
 * que_manager.h
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
void mutex_init(void);
void set_last_packet_time( wiced_time_t time );
wiced_time_t get_last_packet_time();
void list_init( message_list_t *list );
void list_add( message_list_t *list, message_t *entry  );
void list_add_at( wiced_time_t timestamp, message_list_t *list, message_t *new_entry, uint8_t retry );
message_t *list_pop( message_list_t  *list );
message_t *list_pop_before( wiced_time_t timestamp, message_list_t *list);
void create_msg_lists(void);
void list_release_all( message_list_t *list );
message_t *msg_get( uint16_t min_bytes );
wiced_result_t msg_release( message_t *msg );
wiced_result_t coap_msg_resize( coap_message_t* coap, uint16_t min_bytes );
uint16_t coap_msg_data_size( coap_message_t* coap );
void * coap_msg_payload( coap_message_t* coap );
void list_release(message_list_t *free_list, message_t *msg);
message_t *list_pop_confirmable_match( message_list_t *xmit_list, wiced_ip_address_t *remote_ip,
        uint16_t remote_port, uint16_t id );
uint8_t confirmable_match( message_t *entry, wiced_ip_address_t *remote_ip,
        uint16_t remote_port, uint16_t id );
void print_msg( message_t *msg );
uint16_t get_messaging_list_empty_errors();
uint16_t get_block_not_found_errors();
uint16_t get_block_list_details( uint16_t index, uint16_t *smallest_freelist_size, uint16_t *errors );
int list_size( message_list_t *list );
void dump_lists(void);
void print_free_msg_sizes();
uint16_t max_packet_size(void);

#endif /* QUE_MANAGER_H_ */
