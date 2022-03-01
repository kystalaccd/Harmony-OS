#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iic_hi3861.h"
#include "max30102.h"
#include "hi_time.h"
#include "hi_gpio.h"
#include "hi_io.h"


static void  max30102Task(void *arg)
{
    unsigned char data1=0,data2=0;
    unsigned char fifo_wr=0,fifo_rd=0;
    FIFO_DATA red_ir[32]={0};

    while(MAX30102_INT)
        printf("loop\n");
    
    // max30102_read_reg(REG_PART_ID,&data1);
    // printf("part id is %d\n",data1);

    

    max30102_read_FIFO_WR_PTR(&fifo_wr);
    max30102_read_FIFO_RD_PTR(&fifo_rd);
    printf("sample number is %d\n",fifo_wr-fifo_rd);
    // printf("*****************************************************************************\n");
    // for(unsigned char i=0;i<17;++i)
    // {
    //     max30102_read_FIFO_DATA(red_ir,17);
    //     printf("red_data%d is %x",i+1,red_ir[i].RED[0]);
    //     printf("%x",red_ir[i].RED[1]);
    //     printf("%x\n",red_ir[i].RED[2]);
    // }



    max30102_read_reg(REG_TINT,&data1);
    max30102_read_reg(REG_TFRAC,&data2);
    printf("temperature is %f\n",data1+(float)data2*0.0625);
}

static void start(void)
{
    osThreadAttr_t attrOne;
    attrOne.name = "threadOne";
    attrOne.attr_bits = 0U;
    attrOne.cb_mem = NULL;
    attrOne.cb_size = 0U;
    attrOne.stack_mem = NULL;
    attrOne.stack_size = 1024 * 4;
    attrOne.priority = 25;

    IIC_Init();
    max30102_reset();
    max30102_init();

    if (osThreadNew((osThreadFunc_t)max30102Task, NULL, &attrOne) == NULL)
    {
    printf("Failed to create taskOne!");
    }

}

SYS_RUN(start);