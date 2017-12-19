#
# Copyright 2017, Eric Thelin
#

NAME := Lib_SPI_Flash_Fast_Erase_Library_$(PLATFORM)

$(NAME)_SOURCES := spi_flash_fast_erase.c

$(NAME)_DEFINES += SFLASH_SUPPORT_SST_PARTS \
                   SFLASH_SUPPORT_MACRONIX_PARTS \
                   SFLASH_SUPPORT_EON_PARTS \
                   SFLASH_SUPPORT_MICRON_PARTS

GLOBAL_INCLUDES := .

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)