/*
 * i2c.h
 *
 * Created: 13.01.2024 16:57:17
 *  Author: zalto
 */ 
#ifndef I2C_H_
#define I2C_H_
void    I2C_0_Init(void);
uint8_t I2C_0_SendData(uint8_t address, uint8_t *pData, uint8_t len); // returns how many bytes have been sent, -1 means NACK at address
uint8_t I2C_0_GetData(uint8_t address, uint8_t *pData, uint8_t len); // returns how many bytes have been received, -1 means NACK at address
void    I2C_0_EndSession(void);
void I2C_SlaveAddress(uint8_t);
void I2C_SendByte(uint8_t);
#endif