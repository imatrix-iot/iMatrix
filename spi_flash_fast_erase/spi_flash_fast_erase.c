/*
 * spi_flash_fast_erase.c
 *
 *  Created on: Dec 15, 2017
 *      Author: Eric Thelin
 */

#include "spi_flash.h"
#include "spi_flash_internal.h"

#include "spi_flash_fast_erase.h"

#define BLOCKSIZE_64KB (0x10000)
#define SECTORSIZE_4KB (0x1000)

/**
 * Erase as much space as required using the 64KB fast erase command whenever possible.
 * required_length = the area that has to be erased.
 * max_length = the maximum area which may be erased.
 *
 * written by Eric Thelin 18 December 2017
 */
int sflash_erase_area( const sflash_handle_t* const handle, unsigned long device_address,
        unsigned long required_length, unsigned long max_length )
{
    static unsigned long last_device_id = 0;
    static unsigned long chip_size = 0;

    unsigned long finished_erasing_address = device_address + required_length;
    unsigned long min_blocksize;
    unsigned long extra_required;
    unsigned long required_in_last_block;


    if ( handle == NULL ) {
        return -1;
    }

    // device_address must be aligned on the smallest erasable block boundaries

#ifdef SFLASH_SUPPORT_MICRON_PARTS
    if ( handle->device_id == SFLASH_ID_M25P32 ) {
        if ( ( ( device_address % BLOCKSIZE_64KB ) != 0 ) || ( ( max_length % BLOCKSIZE_64KB ) != 0 ) ) {
            return -1;
        }
    }
    else
#endif
    if ( ( ( device_address % SECTORSIZE_4KB ) != 0 ) || ( ( max_length % SECTORSIZE_4KB ) != 0 ) ) {
        return -1;
    }

    // Get chip size(bytes) if we do not have it already.

    if ( handle->device_id != last_device_id ) {
#ifdef SFLASH_SUPPORT_SST_PARTS
        if ( handle->device_id == SFLASH_ID_SST26VF032B ) {
            chip_size = 0x400000; // 4MB
        }
        else
#endif
        {
            sflash_get_size( handle, &chip_size ); // chip_size is 0 if device_id is unrecognized.
        }
        last_device_id = handle->device_id;
    }

    // The full area to erase should be on the chip if the size is known.

    if ( ( chip_size != 0 ) &&
         ( ( device_address >= chip_size ) || ( device_address + max_length > chip_size ) ) )
    {
        return -1;
    }

    // Make sure max_length is big enough to include the last block that needs to be erased.

#ifdef SFLASH_SUPPORT_MICRON_PARTS
    if ( handle->device_id == SFLASH_ID_M25P32 ) {
       min_blocksize = BLOCKSIZE_64KB;
    }
    else
#endif
    {
        min_blocksize = SECTORSIZE_4KB;
    }

    extra_required = 0;
    required_in_last_block = required_length % min_blocksize;

    if ( required_in_last_block > 0 ) {
        extra_required = min_blocksize - required_in_last_block;
    }

    if ( max_length < required_length + extra_required )
    {
        return -1;
    }

    ///////////// ERASE //////////////////////

    while ( ( device_address < finished_erasing_address ) && ( max_length >= SECTORSIZE_4KB ) ) {

        if ( // Erase 4KB sectors when needed
#ifdef SFLASH_SUPPORT_MICRON_PARTS
             ( handle->device_id != SFLASH_ID_M25P32 ) && // This device cannot erase 4KB sectors.
#endif
             ( ( max_length < BLOCKSIZE_64KB ) // No space to erase 64KB.
               || ( ( device_address % BLOCKSIZE_64KB ) != 0 ) // Not aligned on 64KB boundary.
#ifdef SFLASH_SUPPORT_SST_PARTS
               || ( ( handle->device_id == SFLASH_ID_SST26VF032B ) && // This device has 64KB blocks except for..
                    ( ( device_address < BLOCKSIZE_64KB ) // First 64KB
                      || ( device_address >= chip_size - BLOCKSIZE_64KB ) ) ) // Last 64KB
#endif
            ) )
        {
            sflash_sector_erase ( handle, device_address );
            device_address += SECTORSIZE_4KB;
            max_length -= SECTORSIZE_4KB;
        }
        else {
#ifdef SFLASH_SUPPORT_MICRON_PARTS
            if ( ( handle->device_id == SFLASH_ID_M25P32 ) && ( max_length < BLOCKSIZE_64KB ) ) break;
            // All devices that support 4KB erase already checked to make sure there was at least 64KB.
#endif
            sflash_block_erase ( handle, device_address );
            device_address += BLOCKSIZE_64KB;
            max_length -= BLOCKSIZE_64KB;
        }
    }

    if ( device_address < finished_erasing_address ) {// The while loop exited before erasing was complete.
        return -1; // Input validation should prevent this from ever happening.
    }

    return 0; // Success!
}



