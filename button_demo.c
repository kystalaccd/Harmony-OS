#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "button_hi3861.h"

static void USR_callback(ButtonEvent event)
{
    if(event == Pressed){}
    else if(event == LongPressed){}
}

static void S1_callback(ButtonEvent event)
{
    ;
}

static void S2_callback(ButtonEvent event)
{
    ;
}

static void button_demo_task (void *arg)
{
    int ret=0;

    ret +=Button_Init();        //按键模块初始化
    //使能指定按键的指定功能
    ret +=Button_Enable(USR,USR_callback,Pressed | LongPressed);       //处理点击和长按   0101
    ret +=Button_Enable(S1,S1_callback,Released);       //处理按键释放
    ret +=Button_Enable(S2,S2_callback,Pressed);        //处理按键点击
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

    if (osThreadNew((osThreadFunc_t)button_demo_task, NULL, &attrOne) == NULL)
    {
    printf("Failed to create taskOne!");
    }

}

SYS_RUN(start);