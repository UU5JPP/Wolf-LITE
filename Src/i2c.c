#include "stm32f4xx_hal.h"
#include "main.h"
#include "functions.h"
#include "i2c.h"

/* low level conventions:
 * - SDA/SCL idle high (expected high)
 * - always start with i2c_delay rather than end
 */

I2C_DEVICE I2C_WM8731 = {
	.SDA_PORT = WM8731_SDA_GPIO_Port,
	.SDA_PIN = WM8731_SDA_Pin,
	.SCK_PORT = WM8731_SCK_GPIO_Port,
	.SCK_PIN = WM8731_SCK_Pin,
	.i2c_tx_addr = 0,
	.i2c_tx_buf = {0},
	.i2c_tx_buf_idx = 0,
	.i2c_tx_buf_overflow = false,
};

#ifdef HAS_TOUCHPAD
I2C_DEVICE I2C_TOUCHPAD = {
	.SDA_PORT = ENC2_CLK_GPIO_Port,
	.SDA_PIN = ENC2_CLK_Pin,
	.SCK_PORT = ENC2_DT_GPIO_Port,
	.SCK_PIN = ENC2_DT_Pin,
	.i2c_tx_addr = 0,
	.i2c_tx_buf = {0},
	.i2c_tx_buf_idx = 0,
	.i2c_tx_buf_overflow = false,
};
#endif

static uint8_t i2c_writeOneByte(I2C_DEVICE *dev, uint8_t);

static void SDA_OUT(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);
}

static void SDA_IN(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(dev->SDA_PORT, &GPIO_InitStruct);
}

static void SCK_OUT(I2C_DEVICE *dev)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = dev->SCK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(dev->SCK_PORT, &GPIO_InitStruct);
}

void i2c_start(I2C_DEVICE *dev)
{
	SDA_SET;
	SCK_SET;
	I2C_DELAY
	SDA_CLR;
	I2C_DELAY
	SCK_CLR;
}

void i2c_stop(I2C_DEVICE *dev)
{
	SDA_OUT(dev);
	SDA_CLR;
	SCK_SET;
	I2C_DELAY
	I2C_DELAY
	I2C_DELAY
	I2C_DELAY
	SDA_SET;
	I2C_DELAY
}

bool i2c_get_ack(I2C_DEVICE *dev)
{
	uint32_t time = 0;
	
	SDA_IN(dev);

	I2C_DELAY
	I2C_DELAY
	SCK_SET;
	I2C_DELAY
	I2C_DELAY

	while (HAL_GPIO_ReadPin(dev->SDA_PORT, dev->SDA_PIN))
	{
		time++;
		if (time > 50)
		{
			i2c_stop(dev);
			return false;
		}
		I2C_DELAY
		I2C_DELAY
	}

	SCK_CLR;

	return true;
}

void i2c_shift_out(I2C_DEVICE *dev, uint8_t val)
{
	SDA_OUT(dev);
	int i;
	for (i = 0; i < 8; i++)
	{

		I2C_DELAY
		HAL_GPIO_WritePin(dev->SDA_PORT, dev->SDA_PIN, (GPIO_PinState) !!(val & (1 << (7 - i))));
		SDA_OUT(dev);

		I2C_DELAY
		SCK_SET;

		I2C_DELAY
		I2C_DELAY
		SCK_CLR;
	}
}

/*
 * Joins I2C bus as master on given SDA and SCL pins.
 */
void i2c_begin(I2C_DEVICE *dev)
{
	SCK_SET;
	SDA_SET;

	SCK_OUT(dev);
	SDA_OUT(dev);
}

void i2c_beginTransmission_u8(I2C_DEVICE *dev, uint8_t slave_address)
{
	i2c_begin(dev);
	dev->i2c_tx_addr = slave_address;
	dev->i2c_tx_buf_idx = 0;
	dev->i2c_tx_buf_overflow = false;
}

bool i2c_beginReceive_u8(I2C_DEVICE *dev, uint8_t slave_address)
{
	i2c_begin(dev);
	dev->i2c_tx_addr = slave_address;
	dev->i2c_tx_buf_idx = 0;
	dev->i2c_tx_buf_overflow = false;
	i2c_start(dev);
	i2c_shift_out(dev, (uint8_t)((dev->i2c_tx_addr << 1) | I2C_READ));
	if (!i2c_get_ack(dev))
	{
		//sendToDebug_strln("no ack");
		return false;
	}
	return true;
}

uint8_t i2c_endTransmission(I2C_DEVICE *dev)
{
	if (dev->i2c_tx_buf_overflow)
		return EDATA;
	i2c_start(dev);

	//I2C_DELAY;
	i2c_shift_out(dev, (uint8_t)((dev->i2c_tx_addr << 1) | I2C_WRITE));
	if (!i2c_get_ack(dev))
		return ENACKADDR;

	// shift out the address we're transmitting to
	for (uint8_t i = 0; i < dev->i2c_tx_buf_idx; i++)
	{
		uint8_t ret = i2c_writeOneByte(dev, dev->i2c_tx_buf[i]);
		if (ret)
			return ret; // SUCCESS is 0
	}
	I2C_DELAY
	I2C_DELAY
	i2c_stop(dev);

	dev->i2c_tx_buf_idx = 0;
	dev->i2c_tx_buf_overflow = false;
	return SUCCESS;
}

void i2c_write_u8(I2C_DEVICE *dev, uint8_t value)
{
	if (dev->i2c_tx_buf_idx == WIRE_BUFSIZ)
	{
		dev->i2c_tx_buf_overflow = true;
		return;
	}

	dev->i2c_tx_buf[dev->i2c_tx_buf_idx++] = value;
}

// private methods
static uint8_t i2c_writeOneByte(I2C_DEVICE *dev, uint8_t byte)
{
	i2c_shift_out(dev, byte);
	if (!i2c_get_ack(dev))
		return ENACKTRNS;
	return SUCCESS;
}

uint8_t i2c_Read_Byte(I2C_DEVICE *dev, uint8_t ack)
{
	unsigned char i,receive=0;
	SDA_IN(dev);
	I2C_DELAY
	I2C_DELAY
	
  for(i=0;i<8;i++ )
	{
    SCK_CLR;
		I2C_DELAY
		I2C_DELAY
		SCK_SET;
    receive<<=1;
		I2C_DELAY
		I2C_DELAY
    if(HAL_GPIO_ReadPin(dev->SDA_PORT,dev->SDA_PIN))
			receive++;   
	}
	if (!ack)
	{
		//NAck
		SCK_CLR;
		SDA_OUT(dev);
		SDA_SET;
		I2C_DELAY
		I2C_DELAY
		SCK_SET;
		I2C_DELAY
		I2C_DELAY
		SCK_CLR;
		SDA_IN(dev);
	}
	else 
	{
		//Ack
		SCK_CLR;
		SDA_OUT(dev);
		SDA_CLR;
		I2C_DELAY
		I2C_DELAY
		SCK_SET;
		I2C_DELAY
		I2C_DELAY
		SCK_CLR;
		SDA_IN(dev);
	}
 	return receive;
}
