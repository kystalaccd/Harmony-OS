#include "cmsis_os2.h"
#include "ohos_init.h"

#include "iic_hi3861.h"
#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_time.h"

#define IIC_SCL(value) iic_scl(value)
#define IIC_SDA(value) iic_sda(value)
#define READ_SDA read_sda()
#define SDA_OUT() hi_gpio_set_dir(PIN_SDA,HI_GPIO_DIR_OUT)
#define SDA_IN() hi_gpio_set_dir(PIN_SDA,HI_GPIO_DIR_IN)

static void iic_scl(hi_gpio_value value)
{
    hi_gpio_value val = (value == 0) ? 0 : 1;
    hi_gpio_set_ouput_val(PIN_SCL, val);
}

static void iic_sda(hi_gpio_value value)
{
    hi_gpio_value val = (value == 0) ? 0 : 1;
    hi_gpio_set_ouput_val(PIN_SDA, val);
}

static hi_gpio_value read_sda(void)
{
    hi_gpio_value val = 0;
    hi_gpio_get_input_val(PIN_SDA, &val);

    return val;
}

void IIC_Init(void)
{
    hi_gpio_init();
    //SCL初始化
    hi_io_set_func(PIN_SCL,HI_IO_FUNC_GPIO_10_GPIO);
    hi_gpio_set_dir(PIN_SCL,HI_GPIO_DIR_OUT);
    //SDA初始化
    hi_io_set_func(PIN_SDA,HI_IO_FUNC_GPIO_11_GPIO);
    hi_gpio_set_dir(PIN_SDA,HI_GPIO_DIR_OUT);
    //SCL，SDA拉高，总线处于空闲状态
    IIC_SCL(1);
    IIC_SDA(1);
}

void IIC_Start(void)
{
    SDA_OUT();
    IIC_SCL(1);
    IIC_SDA(1);
    hi_udelay(2);
    IIC_SDA(0);
    hi_udelay(2);
    IIC_SCL(0);
    // hi_udelay(2);
}

void IIC_Stop(void)
{
    SDA_OUT();
    IIC_SCL(0);
    IIC_SDA(0);
    hi_udelay(2);
    IIC_SCL(1);
    hi_udelay(2);
    IIC_SDA(1);
    hi_udelay(2);
}

void IIC_Send_Byte(unsigned char data)
{
    unsigned char temp;
    SDA_OUT();
    IIC_SCL(0);
    for(temp=0x80;temp!=0;temp>>=1)
    {
        IIC_SDA(temp&data);
        hi_udelay(2);
        IIC_SCL(1);
        hi_udelay(2);
        IIC_SCL(0);
        hi_udelay(2);
    }
}

unsigned char IIC_Recv_Ack(void)
{
    unsigned char ack;
    SDA_OUT();
    IIC_SDA(1);
    SDA_IN();
    hi_udelay(2);
    IIC_SCL(1);
    hi_udelay(2);
    ack = READ_SDA;
    IIC_SCL(0);

    return ack;
}


unsigned char IIC_Read_Byte(void)
{
    unsigned char data=0;
    unsigned char temp;
    SDA_OUT();
    IIC_SDA(1);
    SDA_IN();
    for(temp=0x80;temp!=0;temp>>=1) //0x1000 0000
    {
        IIC_SCL(0);
        hi_udelay(2);
        IIC_SCL(1);
        if(READ_SDA)
            data |= temp;
        else
            data &= (~temp);
        hi_udelay(2);
    }
    return data;
}


void IIC_Ack(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(0);
    hi_udelay(2);
    IIC_SCL(1);
    hi_udelay(2);
    IIC_SCL(0);
}

void IIC_NAck(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(1);
    hi_udelay(2);
    IIC_SCL(1);
    hi_udelay(2);
    IIC_SCL(0);
}

unsigned char IIC_Wait_Ack(void)
{
    unsigned char ErrTime=0;
    SDA_OUT();
    IIC_SDA(1);
    hi_udelay(2);
    IIC_SCL(1);
    hi_udelay(2);
    SDA_IN();
    while(READ_SDA) //未应答时为1，应答后为0
    {
        ErrTime++;
        if(ErrTime>250)
        {
            IIC_Stop();
            return 1;       //等待超时，返回1
        }
    }
    IIC_SCL(0);
    return 0;       //正常应答，返回0
}
