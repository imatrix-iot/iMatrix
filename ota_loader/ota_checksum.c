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
 * ota_checksum.h
 *
 *  Created on: Dec 28, 2016
 *      Author: greg.phillips
 */
/*
 * ota_checksum.c
 *
 *  Created on: Mar 19, 2016
 *      Author: eric
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../cli/interface.h"

#define CRCPOLY 0xEDB88320
#define CRCINV 0x5B358FD3
#define INITXOR 0xFFFFFFFF
#define FINALXOR 0xFFFFFFFF

enum checksum_state_t {
	CHECKSUM_UNINITIALIZED,
	CHECKSUM_IN_PROCESS,
	CHECKSUM_FINALIZED,
};

static uint32_t ota_checksum = INITXOR;
static uint16_t ota_checksum_status = CHECKSUM_UNINITIALIZED;

void start_ota_checksum()
{
	ota_checksum = INITXOR;
	ota_checksum_status = CHECKSUM_IN_PROCESS;
}

void calc_ota_checksum_partial( uint8_t *buf, uint16_t length )
{
	if ( ( ota_checksum_status == CHECKSUM_UNINITIALIZED ) || ( ota_checksum_status == CHECKSUM_FINALIZED ) ) {
		imx_printf( "Attempted to calculate checksum for OTA without initializing it.\r\n" );
		return;
	}
    uint16_t i, j;
    for (j = 0; j < length; ++j) {
        uint8_t b = buf[j];
        for (i = 0; i < 8; ++i) {
            if ((ota_checksum ^ b) & 1) {
            	ota_checksum = (ota_checksum >> 1) ^ CRCPOLY;
            } else {
            	ota_checksum >>= 1;
            }
            b >>=1;
        }
    }
}

uint32_t get_ota_checksum()
{
	if ( ota_checksum_status == CHECKSUM_UNINITIALIZED ) {
		imx_printf( "Attempted to retrieve checksum for OTA without calculating it.\r\n" );
		return INITXOR;
	}
	if ( ota_checksum_status == CHECKSUM_FINALIZED ) {
		return ota_checksum;
	}
	// else CHECKSUM_IN_PROCESS
	ota_checksum_status = CHECKSUM_FINALIZED;
	return ( ota_checksum ^= FINALXOR );
}

void destroy_ota_checksum()
{
	ota_checksum = INITXOR;
	ota_checksum_status = CHECKSUM_UNINITIALIZED;
}

/*
int checksum(uint8_t *buf, uint16_t length)
{
    uint16_t i, j;
    uint32_t crcreg = 0xFFFFFFFF;

    for (j = 0; j < length; ++j) {
        uint8_t b = buf[j];
        for (i = 0; i < 8; ++i) {
            if ((crcreg ^ b) & 1) {
                crcreg = (crcreg >> 1) ^ OxEDB88320;
            } else {
                crcreg >>= 1;
            }
            b >>=1;
        }
    }
    return crcreg ^ 0xFFFFFFFF;
}
*/

