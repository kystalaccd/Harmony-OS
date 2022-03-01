#ifndef MAX30102_H
#define MAX30102_H

#include "hi_io.h"
#include "hi_gpio.h"

//FIFO数据
typedef struct
{
    unsigned char RED[3];
    unsigned char IR[3];
}FIFO_DATA;

//每次FIFO_FULL中断触发时的未读样本数
#define SAMPLES_PER_TIME 17

//max30102的设备ID
#define MAX30102_ADDR 0xAE

//max30102的各寄存器地址
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TINT 0x1F
#define REG_TFRAC 0x20
#define REG_TEMP_EN 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

//max30102的各寄存器配置参数
#define CONFIG_INTR_ENABLE_1 0x80   //A_FULL_EN=1,启用A_FULL中断；其他中断禁用
#define CONFIG_INTR_ENABLE_2 0x00   //禁用内部温度就绪中断
#define CONFIG_FIFO 0x0f    //0b0000 1111，SMP_AVE[2:0]=000，不进行样本平均；FIFO_ROLLOVER_EN=0，不启用FIFO满时回滚功能；FIFO_A_FULL[3:0]=0xfh，当FIFO中有17个未读样本时触发中断。
#define CONFIG_MODE 0x03    //0b0000 0011，启用SpO2模式。
#define CONFIG_SPO2 0x27    //0b0010 0111， ADC满标度为4096nA, 每秒钟采集100个样本, ADC分辨率为18bits
#define CONFIG_LED1_PA 0x32     //LED1脉冲幅度置为10mA
#define CONFIG_LED2_PA 0x32     //LED2脉冲幅度置为10mA
#define CONFIG_TEMP_EN 0x01     //启用测温功能




//GPIO8 引脚监控max30102中断引脚
#define PIN_INT HI_GPIO_IDX_8

#define MAX30102_INT max30102_interrupt_pin()

/*
    Description:
        MAX30102 is initialized to SpO2 mode;
        Only A_FULL interrupt is enabled, and 32 unread data sample is stored in FIFO when intrerupt is issued;
        ADC range = 4096nA, SpO2 sample rate = 100Hz, ADC resolution = 18 bits;
    Parameter:
        None
    Return Value:
        None
*/
void max30102_init(void);

/*
    Description:
        reset MAX30102.
    Parameter:
        None
    Return Value:
        None
*/
void max30102_reset(void);

/*
    Description:
        Write data to MAX30102 register.
    Parameter:
        reg_addr    --register address
        wr_data     --data ready to write
    Return Value:
           0    --false
           1    --true
*/
unsigned char max30102_write_reg(unsigned char reg_addr,unsigned char wr_data);

/*
    Description:
        Read data from max31002 register.
    Parameter:
        reg_addr    --register address
        rd_data     --address of data to be read
    Return Value:
        0   --false
        1   --true
*/
unsigned char max30102_read_reg(unsigned char reg_addr,unsigned char *rd_data);

/*
    Description:
        Read multiple data from max31002 register.
    Parameter:
        reg_addr        --register address
        data            --address of data to be read
        data_length     --length of data to be read
    Return Value:
        0   --false
        1   --true
*/
unsigned char max30102_read_multi_reg(unsigned char reg_addr,unsigned char *data,unsigned char data_length);

/*
    Description:
        Read data from max30102 FIFO_DATA register.
    Parameter:
        sample_data             --data of the sample to be read
        num_samples_to_read     --number of samples ready to read
    Return Value:
        0   --false
        1   --true
*/
unsigned char max30102_read_FIFO_DATA(FIFO_DATA sample_data[],unsigned char num_samples_to_read);

/*
    Description:
        Read data from max30102 FIFO_RD_PTR register.
    Parameter:
        fifo_rd_ptr     --Size of FIFO read pointer
    Return Value:
        0   --false
        1   --true
*/
unsigned char max30102_read_FIFO_RD_PTR(unsigned char *fifo_rd_ptr);

/*
    Description:
        Read data from max30102 FIFO_WR_PTR register.
    Parameter:
        fifo_wr_ptr     --Size of FIFO write pointer
    Return Value:
        0   --false
        1   --true
*/
unsigned char max30102_read_FIFO_WR_PTR(unsigned char *fifo_wr_ptr);

/*
    Description:
        Monitor MAX30102 interrupt pin level value.
    Parameter:
        None
    Return Value:
        0   --interrupt is triggered
        1   --Interrupt is not triggered
*/
hi_gpio_value max30102_interrupt_pin(void);

#endif