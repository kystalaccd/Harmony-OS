#ifndef IIC_HI3861_H
#define IIC_HI3861_H

#include "hi_gpio.h"

//GPIO模拟IIC通信
#define PIN_SCL HI_GPIO_IDX_10
#define PIN_SDA HI_GPIO_IDX_11


/*

*/
void IIC_Init(void);

void IIC_Start(void);

void IIC_Stop(void);

void IIC_Send_Byte(unsigned char data);

unsigned char IIC_Read_Byte(void);
/*

*/
unsigned char IIC_Wait_Ack(void);

unsigned char IIC_Recv_Ack(void);

void IIC_Ack(void);

void IIC_NAck(void);

#endif