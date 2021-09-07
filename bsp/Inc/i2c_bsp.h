#ifndef __I2C_H
#define __I2c_H

#include "stm32f1xx.h"
#include "module.h"

#define I2C_WR	0	
#define I2C_RD	1		

typedef struct {
	void (*init) (void *thiz, void *priv);
}i2c_ops_t;

typedef struct {
	uint8_t device_id;
	module_t * gpio_mod;
	int i2c_scl_pin;
	int i2c_sda_pin;
}i2c_private_t;

int Isok_i2c_Device(void);
uint8_t IIC_Write_Buffer(uint16_t len ,uint8_t *data);
uint16_t IIC_Read_Buffer( uint16_t tx_len, uint8_t* txbuf, uint16_t rx_len, uint8_t *rxbuf);
#endif
