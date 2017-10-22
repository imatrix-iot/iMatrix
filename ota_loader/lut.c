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

/** @file .c
 *
 *  Created on: October 19, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "wiced.h"
#include "../storage.h"
#include "ota_structure.h"
#include "../device_app_dct.h"

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief
  * @param  None
  * @retval : None
  */
app_header_t apps_lut[ FULL_IMAGE ];// Look Up Table descriptions of the 8 entries that may be used as bootable images by the bootloader.
/*
* This LUT is for a 2MB Serial Flash Part. Only items stored are Factory Reset App, DCT and One Downloaded image
*/
app_header_t default_lut[ FULL_IMAGE ] = {
[ DCT_FR_APP_INDEX ] = {
.count = 1,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0001,
.count = 0x00C8,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_DCT_IMAGE_INDEX ] = {
.count = 1,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x00C9,
.count = 0x0004,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_OTA_APP_INDEX ] = {
.count = 0,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0000,
.count = 0x000,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_FILESYSTEM_IMAGE_INDEX ] = {
.count = 0,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0000,
.count = 0x000,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_WIFI_FIRMWARE_INDEX ] = {
.count = 1,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0000,
.count = 0x000,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_APP0_INDEX ] = {
.count = 1,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x00CD,
.count = 0x00C8,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_APP1_INDEX ] = {
.count = 0,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0000,
.count = 0x000,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
[ DCT_APP2_INDEX ] = {
.count = 0,     // Number of Entries
#ifndef BOOTLOADER_APP_LUT_NO_SECURE_FLAG
.secure = 0,    //Is this app secure (Signed/Encrypted) or not - Added in SDK-3.4.0
#endif
.sectors[ 0 ] = {
.start = 0x0000,
.count = 0x000,
},
.sectors[ 1 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 2 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 3 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 4 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 5 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 6 ] = {
.start = 0x0000,
.count = 0x0000,
},
.sectors[ 7 ] = {
.start = 0x0000,
.count = 0x0000,
},
},
};
