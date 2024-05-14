/*
 * spi.h
 *
 * Created: 11.03.2024 17:30:58
 *  Author: zalto
 */ 


#ifndef SPI_H_
#define SPI_H_
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

/** Function pointer to callback function called by SPI IRQ.
    NULL=default value: No callback function is to be used.
*/
typedef void (*spi_transfer_done_cb_t)(void);

/** Specify whether the SPI transfer is to be uni- or bidirectional.
    A bidirectional transfer (=SPI_EXCHANGE) causes the received data
    to overwrite the buffer with the data to transmit.
*/
typedef enum spi_transfer_type {
	SPI_EXCHANGE, ///< SPI transfer is bidirectional
	SPI_READ,     ///< SPI transfer reads, writes only 0s
	SPI_WRITE     ///< SPI transfer writes, discards read data
} spi_transfer_type_t;

/** Status of the SPI hardware and SPI bus.*/
typedef enum spi_transfer_status {
	SPI_FREE, ///< SPI hardware is not open, bus is free.
	SPI_IDLE, ///< SPI hardware has been opened, no transfer ongoing.
	SPI_BUSY, ///< SPI hardware has been opened, transfer ongoing.
	SPI_DONE  ///< SPI hardware has been opened, transfer complete.
} spi_transfer_status_t;

void SPI_0_init(void);

void SPI_0_enable();

void SPI_0_disable();

uint8_t SPI_0_exchange_byte(uint8_t data);

void SPI_0_exchange_block(void *block, uint8_t size);

void SPI_0_write_block(void *block, uint8_t size);

void SPI_0_read_block(void *block, uint8_t size);

void SPI_0_register_callback(spi_transfer_done_cb_t f);

bool SPI_0_status_free(void);

bool SPI_0_status_idle(void);

bool SPI_0_status_busy(void);

bool SPI_0_status_done(void);



#endif /* SPI_H_ */