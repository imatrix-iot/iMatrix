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
 * .h
 *
 *  Created on: April, 2015
 *      Author: greg.phillips
 */

#ifndef ADD_COAP_OPTION_H_
#define ADD_COAP_OPTION_H_

/** @file add_option.h
 *
 *
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
 *               Function Declarations
 ******************************************************/
uint16_t coap_insert_numeric_option( uint16_t option_number, uint16_t *previous_option_number, uint16_t option_value,
		uint16_t start_array_index, coap_message_t *msg );
wiced_result_t coap_find_numeric_option( uint16_t option_number_in, coap_message_t *msg,
		uint16_t *option_value_out, uint16_t *option_index_out, uint16_t *preceeding_option_number_out );
uint16_t create_coap_option_header( uint8_t *header, uint16_t option_number, uint16_t option_length, uint16_t current_option_number );
uint16_t add_coap_uint_option( uint16_t option_number, uint16_t option_value, uint16_t *current_option_number,
		uint8_t *buffer, uint16_t buf_length );
uint16_t add_coap_str_option( uint16_t option_number, uint16_t *current_option_number, char *option_string, uint8_t *buffer, uint16_t buf_length );

uint16_t add_coap_uri_option( uint16_t *current_option_number, uint8_t *buffer, char *uri );
uint16_t add_options_from_string( uint16_t option_number, char separator, char *str_in,
        uint16_t *current_option_number, uint8_t *buffer, uint16_t buf_length );

#endif /* ADD_COAP_OPTION_H_ */
