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
 *  Created on: December, 2016
 *      Author: greg.phillips
 */

#ifndef CERT_DEFS_H_
#define CERT_DEFS_H_

/** @file cert_defs.h
 *
 *	Defines for certificates
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MIN_CERT_LENGTH			512
#define MAX_CERT_LENGTH			1800
#define ROOT_CA_OFFSET			2048
#define BEGIN_CERT_LENGTH		( strlen( CERT_BEGIN ) + 2 )					// The extra 2 bytes are the CR/LF
#define BEGIN_END_CERT_LENGTH	( BEGIN_CERT_LENGTH + strlen( CERT_END ) + 2 )
#define BEGIN_KEY_LENGTH		( strlen( KEY_BEGIN ) + 2 )					// The extra 2 bytes are the CR/LF
#define BEGIN_END_KEY_LENGTH	( BEGIN_KEY_LENGTH + strlen( KEY_END ) + 2 )

/******************************************************
 *                   Enumerations
 ******************************************************/
enum {
	CA_ROOT_CERTIFICATE,
	WIFI_CERTIFICATE_KEY,
	WIFI_CERTIFICATE,
	DTLS_CERTIFICATE,
	TLS_CERTIFICATE,
};
/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* CERT_DEFS_H_ */
