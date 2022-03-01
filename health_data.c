#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "health_data.h"
#include "iic_hi3861.h"
#include "max30102.h"



static void health_data_translation(FIFO_DATA red_ir_data[],HD_len RED_data[],HD_len IR_data[],unsigned int data_start,unsigned int data_end)
{
    unsigned int i;
    HD_len temp;
    for(i=data_start;i<data_end+1;++i)  //每次从数组下标data_start写到data_end（包含data_end）
    {
        /*
        //ADC分辨率为16bit时
        //RED LED数据转换
        temp=red_ir_data[i-data_start].RED[2]&0xFC;
        temp>>=2;
        RED_data[i]+=temp;      //LSB, bit0~bit5  
        temp=red_ir_data[i-data_start].RED[1];
        temp<<=6;
        RED_data[i]+=temp;      //bit6~bit13
        temp=red_ir_data[i-data_start].RED[0]&0x03;
        temp<<=14;
        RED_data[i]+=temp;      //MSB, bit14~bit15

        //IR LED数据转换
        temp=red_ir_data[i-data_start].IR[2]&0xFC;
        temp>>=2;
        IR_data[i]+=temp;      //LSB, bit0~bit5
        temp=red_ir_data[i-data_start].IR[1];
        temp<<=6;
        IR_data[i]+=temp;       //bit6~bit13
        temp=red_ir_data[i-data_start].IR[0]&0x03;
        temp<<=14;
        IR_data[i]+=temp;       //MSB, bit14~bit15
        */


        //ADC分辨率为18bits时
        //RED LED数据转换
        RED_data[i]=red_ir_data[i-data_start].RED[2];      //LSB, bit0~bit7  
        temp=red_ir_data[i-data_start].RED[1];
        temp<<=8;
        RED_data[i]+=temp;      //bit8~bit15
        temp=red_ir_data[i-data_start].RED[0]&0x03;
        temp<<=16;
        RED_data[i]+=temp;      //MSB, bit16~bit17

        //IR LED数据转换
        IR_data[i]=red_ir_data[i-data_start].IR[2];      //LSB, bit0~bit7
        temp=red_ir_data[i-data_start].IR[1];
        temp<<=8;
        IR_data[i]+=temp;       //bit8~bit15
        temp=red_ir_data[i-data_start].IR[0]&0x03;
        temp<<=16;
        IR_data[i]+=temp;       //MSB, bit16~bit17
    }
}

void health_data_update(void)
{
    unsigned int i;
    FIFO_DATA sample_data[SAMPLES_PER_TIME];
    HD_len RED_data[HEALTH_DATA_PER_TIME];
    HD_len IR_data[HEALTH_DATA_PER_TIME];

    for(i=0;i<HEALTH_DATA_PER_TIME;)
    {
        while(MAX30102_INT==0)    //当中断触发时,MAX30102_INT=0;
        {
            max30102_read_FIFO_DATA(sample_data,SAMPLES_PER_TIME);  //每次读取了17个样本
            health_data_translation(sample_data,RED_data,IR_data,i,i+SAMPLES_PER_TIME-1);
            i+=17;
        }
    }

    //******************************test start************************************
    for(i=0;i<HEALTH_DATA_PER_TIME;++i)
    {
        printf("No.%u Red LED ADC data is %u\n",i+1,RED_data[i]);
    }
    printf("\n\n");
    for(i=0;i<HEALTH_DATA_PER_TIME;++i)
    {
        printf("No.%u IR LED ADC data is %u\n",i+1,IR_data[i]);
    }
    //********************************test end**************************************
}