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
 * coap_def.h
 *
 *  Created on: Oct 23, 2013
 *      Author: greg.phillips
 */

#ifndef COAP_DEF_H_
#define COAP_DEF_H_

/** @file
 *
 *  Include file to support coap_def.c
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define NO_TIME_VALUE		0
#define NO_ID_VALUE         0xFFFFFFFF
#define NO_VALUE_VALUE      0xFFFFFFFF
#define NO_FLOAT_VALUE      -999999.9999
#define NO_SETTING_VALUE    0xFFFFFFFF
#define NO_IMAGE_NO			0xFFFFFFFF
#define NO_MODE_VALUE       "no mode"
#define MODE_AUTO           "auto"
#define MODE_ON             "on"
#define MODE_OFF            "off"
#define NONE                "none"


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
uint16_t get_well_known(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
int16_t get_uri_query( char *uri_query, char *term, char *value );
uint16_t isufloat( char  *number );
uint16_t isuint( char  *number );

#ifdef MULTICAST_DEBUGGING
    uint16_t coap_post_control_multicast(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg);
#endif

#endif /* COAP_DEF_H_ */

