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

/**
 * Get random token of token_length and return in a byte string
 * requires that global variable random_seed already be initialized. See coap_setup() function.
 *
 * @param token is filled with 0 to 8 random bytes used as an identifier for a
 * packet being sent that can connect the response coming back to the original request.
 * @param length is the number of random bytes to return.
 *
 * written by Eric Thelin 27 May 2015
 */

#include <stdlib.h>
#include <stdbool.h>

#include "wiced.h"
#include "coap.h"

void* new_coap_token( token_buffer_t token, unsigned int length )
{
    int i;
    unsigned int num;
    for ( i = 0; i < length; i++) {
        num = rand_r( &random_seed );
        token[i] = (char) (num & 0xFF);// store lowest order 8 bits of random number in each char of token
    }
    return (void*)token;
}

