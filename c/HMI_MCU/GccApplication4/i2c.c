/*
 * i2c.c
 *
*/

#include <util/delay.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include "config.h"
#include "i2c.h"

#define CLK_PER                                         16000000     // 24MHz clock
#define TWI0_BAUD(F_SCL, T_RISE)                        ((((((float)CLK_PER / (float)F_SCL)) - 10 - ((float)CLK_PER * T_RISE))) / 2)

#define I2C_SCL_FREQ                                    100000
#define I2C_SLAVE_ADDRESS                               0x27

#define I2C_DIRECTION_BIT_WRITE                         0
#define I2C_DIRECTION_BIT_READ                          1
#define I2C_SET_ADDR_POSITION(address)                  (address << 1)
#define I2C_SLAVE_RESPONSE_ACKED                        (!(TWI_RXACK_bm & TWI0.MSTATUS))
#define I2C_DATA_RECEIVED                               (TWI_RIF_bm & TWI0.MSTATUS)

enum {
	I2C_INIT = 0,
	I2C_ACKED,
	I2C_NACKED,
	I2C_READY,
	I2C_ERROR
};



void I2C_0_init(void)
{
	TWI0.MBAUD = (uint8_t)TWI0_BAUD(I2C_SCL_FREQ, 0);	//Setting baudrate
	PORTMUX.TWIROUTEA = 0x02;		//Select pins to PC2/PC3
	TWI0.MCTRLA = TWI_ENABLE_bm;	//Enable I2C
	TWI0.MADDR = 0x00;				//Setting master address
	TWI0.MDATA = 0x00;				//Initialize register
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;	//Set bus state to idle	
	#ifdef debugmessages
	USART3_transmitt_char_array(0, "I2C initialized\n", strlen("I2C initialized\n"));
	#endif
}

static uint8_t i2c_0_WaitW(void)
{
	uint8_t state = I2C_INIT;
	do
	{
		if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			if(!(TWI0.MSTATUS & TWI_RXACK_bm))
			{
				/* client responded with ack - TWI goes to M1 state */
				state = I2C_ACKED;
			}
			else
			{
				/* address sent but no ack received - TWI goes to M3 state */
				state = I2C_NACKED;
			}
		}
		else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

static uint8_t i2c_0_WaitR(void)
{
	uint8_t state = I2C_INIT;
	
	do
	{
		if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			state = I2C_READY;
		}
		else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

 /* Returns how many bytes have been sent, -1 means NACK at address, 0 means client ACKed to client address */
uint8_t I2C_0_SendData(uint8_t address, uint8_t *pData, uint8_t len)
{
	uint8_t retVal = (uint8_t) - 1;
	
	/* start transmitting the client address */
	uint8_t shiftedAddress = address << 1;
	//uint8_t addressWithLSB = shiftedAddress | 0x00;

	TWI0.MADDR = shiftedAddress;// & ~0x01;
	if(i2c_0_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData != NULL))
	{
		while(len--)
		{
			TWI0.MDATA = *pData;
			if(i2c_0_WaitW() == I2C_ACKED)
			{
				retVal++;
				pData++;
				continue;
			}
			else // did not get ACK after client address
			{
				break;
			}
		}
	}
	
	return retVal;
}

void I2C_SendByte(uint8_t data){
	uint8_t retVal = (uint8_t) - 1;

	// Sending only one byte of data directly
	TWI0.MDATA = data;
	if(i2c_0_WaitW() == I2C_ACKED)
	{
		retVal = 1; // Successfully sent one byte
	}

return retVal;
}

void I2C_SlaveAddress(uint8_t addr){
	int8_t retVal = (uint8_t) - 1;
	uint8_t shiftedAddress = addr << 1;
	TWI0.MADDR = shiftedAddress;
   // Wait for ACK from the slave device
   if(i2c_0_WaitW() == I2C_ACKED)
   {
	   retVal = 0; // ACK received, communication successful
   }

   return retVal;
}

/* Returns how many bytes have been received, -1 means NACK at address */
uint8_t I2C_0_GetData(uint8_t address, uint8_t *pData, uint8_t len)
{
	uint8_t retVal = (uint8_t) - 1;
	
	/* start transmitting the client address */
	TWI0.MADDR = address | 0x01;
	if(i2c_0_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData !=NULL ))
	{
		while(len--)
		{
			if(i2c_0_WaitR() == I2C_READY)
			{
				*pData = TWI0.MDATA;
				TWI0.MCTRLB = (len == 0)? TWI_ACKACT_bm | TWI_MCMD_STOP_gc : TWI_MCMD_RECVTRANS_gc;
				retVal++;
				pData++;
				continue;
			}
			else
			break;
		}
	}
	
	return retVal;
}