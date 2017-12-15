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

/** @file messages.h
 *
 *  Created on: October 14, 2017
 *      Author: greg.phillips
 *
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

/*
 *	Defines for messages
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
/*
 * Debug message defines
 */
#define DEBUGS_GENERAL                  (0x00000001)
#define DEBUGS_BLE                      (0x00000002)
#define DEBUGS_FOR_BASIC_MESSAGING      (0x00000004)
#define DEBUGS_FOR_XMIT                 (0x00000008)
#define DEBUGS_FOR_RECV                 (0x00000010)
#define DEBUGS_FOR_COAP_DEFINES         (0x00000020)
#define DEBUGS_FOR_HAL                  (0x00000040)
#define DEBUGS_FOR_IMX_UPLOAD           (0x00000080)
#define DEBUGS_FOR_INIT                 (0x00000010)
#define DEBUGS_FOR_SFLASH               (0x00000100)
#define DEBUGS_FOR_APPLICATION_START    (0x00000200)
#define DEBUGS_FOR_EVENTS_DRIVEN        (0x00000400)
#define DEBUGS_FOR_SAMPLING             (0x00000800)
/*
 * Background (callback) messages to print in main loop
 */
#define MSG_WIFI_DN                     (0x00000001)
#define MSG_WIFI_UP                     (0x00000002)
#define MSG_UDP_OUT_OF_MEMORY           (0x00000004)
#define MSG_UDP_MEMORY_LEAK             (0x00000008)
#define MSG_MSG_GET_OUT_MEMORY          (0x00000010)
#define MSG_MSG_GET_MEM_LEAK            (0x00000020)
#define MSG_MSG_GET_BLOCK_UNAVILABLE    (0x00000040)
#define MSG_MSG_GET_NO_MUTEX            (0x00000080)
#define MSG_MSG_GET_NO_UNLOCK_MUTEX     (0x00000100)
#define MSG_COAP_XMIT_PACKET_FAILED     (0x00000200)
#define MSG_COAP_XMIT_SMALL_PACKET      (0x00000400)
#define MSG_COAP_XMIT_NO_MUTEX          (0x00000800)
#define MSG_COAP_XMIT_RST_FAILED        (0x00001000)
#define MSG_COAP_XMIT_NO_UNLOCK_MUTEX   (0x00002000)
#define MSG_TELNET                      (0x00003000)
#define MSG_TELNET_ACCEPT               (0x00008000)
#define MSG_UDP_REC_ERROR               (0x00010000)
#define MSG_UDP_GET_INFO_ERROR          (0x00020000)
#define MSG_TCP_OUT_OF_MEMORY           (0x00040000)
#define MSG_TCP_MEMORY_LEAK             (0x00080000)
#define MSG_TCP_REC_ERROR               (0x00100000)

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

#endif /* MESSAGES_H_ */
