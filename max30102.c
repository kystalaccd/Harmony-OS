#include <stdio.h>
#include <unistd.h>
#include "max30102.h"
#include "iic_hi3861.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_gpio.h"
#include "hi_io.h"

//选择对max30102写入或读取
#define WR 0x00
#define RD 0x01

unsigned char max30102_write_reg(unsigned char reg_addr,unsigned char data)
{
    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);  //选择向MAX30102中写入数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_write_reg: Slave not responding!\n");
        IIC_Stop();
        return 0;
    }
    IIC_Send_Byte(reg_addr);
    if(IIC_Wait_Ack())
    {
        printf("max30102_write_reg: Failed to write register address!\n");
        IIC_Stop();
        return 0;
    }
    IIC_Send_Byte(data);
    if(IIC_Wait_Ack())
    {
        printf("max30102_write_reg: Data writing failed!\n");
        IIC_Stop();
        return 0;
    }
    IIC_Stop();
    return 1;
}

void max30102_init(void)
{
    //配置中断寄存器
    max30102_write_reg(REG_INTR_ENABLE_1,CONFIG_INTR_ENABLE_1);
    max30102_write_reg(REG_INTR_ENABLE_2,CONFIG_INTR_ENABLE_2);
    //将FIFO_WR_PTR、OVF_COUNTER、FIFO_RD_PTR寄存器全部清除为0
    max30102_write_reg(REG_FIFO_WR_PTR,0x00);
    max30102_write_reg(REG_OVF_COUNTER,0x00);
    max30102_write_reg(REG_FIFO_RD_PTR,0x00);
    //FIFO配置
    max30102_write_reg(REG_FIFO_CONFIG,CONFIG_FIFO);
    //Mode配置
    max30102_write_reg(REG_MODE_CONFIG,CONFIG_MODE);
    //SpO2配置
    max30102_write_reg(REG_SPO2_CONFIG,CONFIG_SPO2);
    //LED脉冲幅度
    max30102_write_reg(REG_LED1_PA,CONFIG_LED1_PA);
    max30102_write_reg(REG_LED2_PA,CONFIG_LED2_PA);
    //温度模块使能
    max30102_write_reg(REG_TEMP_EN,CONFIG_TEMP_EN);
    //GPIO模拟中断初始化
    hi_io_set_func(PIN_INT,HI_IO_FUNC_GPIO_8_GPIO);
    hi_gpio_set_dir(PIN_INT,HI_GPIO_DIR_IN);
    hi_io_set_pull(PIN_INT,HI_IO_PULL_UP);
}

void max30102_reset(void)
{
    max30102_write_reg(REG_MODE_CONFIG,0x40);
}

unsigned char max30102_read_reg(unsigned char reg_addr,unsigned char *data)
{
    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_reg: Slave not responding for writing!\n");
        IIC_Stop();
        return 0;
    }
    IIC_Send_Byte(reg_addr);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_reg: Failed to write register address!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | RD);  //选择从MAX30102中读取数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_reg: Slave not responding for reading!\n");
        IIC_Stop();
        return 0;
    }
    *data=IIC_Read_Byte();
    IIC_NAck();
    IIC_Stop();
    return 1;
}

unsigned char max30102_read_multi_reg(unsigned char reg_addr,unsigned char *data,unsigned char data_length)
{
    unsigned char i;
    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_multi_reg: Slave not responding for writing!\n");
        IIC_Stop();
        return 0;
    }
    IIC_Send_Byte(reg_addr);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_multi_reg: Failed to write register address!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | RD);  //选择从MAX30102中读取数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_multi_reg: Slave not responding for reading!\n");
        IIC_Stop();
        return 0;
    }
    for(i=0;i<data_length-1;++i)    //读取前data_length-1个数据时，主机需要发送应答信号。
    {
        data[i]=IIC_Read_Byte();
        IIC_Ack();   
    }
    data[i]=IIC_Read_Byte();
    IIC_NAck();     //最后一个数据主机需要发送非应答信号。
    IIC_Stop();
    return 1;
}

unsigned char max30102_read_FIFO_DATA(FIFO_DATA sample_data[],unsigned char num_samples_to_read)
{
    unsigned char i,j;

    //清除中断
    max30102_read_reg(REG_INTR_STATUS_1,&i);
    max30102_read_reg(REG_INTR_STATUS_2,&j);

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_DATA: Slave not responding for writing!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Send_Byte(REG_FIFO_DATA);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_DATA: Failed to write FIFO_DATA register address!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | RD);  //选择从MAX30102中读取数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_DATA: Slave not responding for reading!\n");
        IIC_Stop();
        return 0;
    }

    for(i=0;i<num_samples_to_read;++i)
    {
        for(j=0;j<3;++j)
        {
            sample_data[i].RED[j]=IIC_Read_Byte();
            IIC_Ack();
        }    
        for(j=0;j<3;++j)
        {   
            if(i==num_samples_to_read-1 && j==2)    //读取到最后一个字节时，主机需要发送非应答信号
            {
                sample_data[i].IR[j]=IIC_Read_Byte();
                IIC_NAck();
            }
            else
            {
                sample_data[i].IR[j]=IIC_Read_Byte();
                IIC_Ack();
            }
        } 
    }
    IIC_Stop();
    return 1;
}

unsigned char max30102_read_FIFO_WR_PTR(unsigned char *fifo_wr_ptr)
{
    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_WR_PTR: Slave not responding for writing!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Send_Byte(REG_FIFO_WR_PTR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_WR_PTR: Failed to write FIFO_WR_PTR register address!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | RD);  //选择从MAX30102中读取数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_WR_PTR: Slave not responding for reading!\n");
        IIC_Stop();
        return 0;
    }

    *fifo_wr_ptr = IIC_Read_Byte();
    IIC_NAck();
    IIC_Stop();

    return 1;
}

unsigned char max30102_read_FIFO_RD_PTR(unsigned char *fifo_rd_ptr)
{
    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | WR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_RD_PTR: Slave not responding for writing!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Send_Byte(REG_FIFO_RD_PTR);
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_RD_PTR: Failed to write FIFO_RD_PTR register address!\n");
        IIC_Stop();
        return 0;
    }

    IIC_Start();
    IIC_Send_Byte(MAX30102_ADDR | RD);  //选择从MAX30102中读取数据
    if(IIC_Wait_Ack())
    {
        printf("max30102_read_FIFO_RD_PTR: Slave not responding for reading!\n");
        IIC_Stop();
        return 0;
    }

    *fifo_rd_ptr = IIC_Read_Byte();
    IIC_NAck();
    IIC_Stop();

    return 1;
}

hi_gpio_value max30102_interrupt_pin(void)
{
    hi_gpio_value val = 0;
    
    hi_gpio_get_input_val(PIN_INT,&val);

    return val;
}