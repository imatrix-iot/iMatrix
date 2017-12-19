/*
 * spi_flash_fast_erase.h
 *
 *  Created on: Dec 15, 2017
 *      Author: Eric Thelin
 */

#ifndef LIBRARIES_DRIVERS_SPI_FLASH_FAST_ERASE_SPI_FLASH_FAST_ERASE_H_
#define LIBRARIES_DRIVERS_SPI_FLASH_FAST_ERASE_SPI_FLASH_FAST_ERASE_H_

int sflash_erase_area( const sflash_handle_t* const handle, unsigned long device_address,
        unsigned long required_length, unsigned long max_length );


#endif /* LIBRARIES_DRIVERS_SPI_FLASH_FAST_ERASE_SPI_FLASH_FAST_ERASE_H_ */
