#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_adc.h"
#include "hi_systick.h"
#include "hi_errno.h"
#include "button_hi3861.h"

#define BUTTON_STACK_SIZE   2048        
#define PRESS_INTERVAL      20000       //按键间隔 20000*(1/32000)=0.625s
#define LOOP_INTERVAL       40000       //循环间隔 40000us=40ms
#define LONG_PRESS_INTERVAL 64          
#define LONG_PRESS_END      0xFFFF      //65535(10)
#define MAX_KEY_NUM         BNone       

enum
{
    ADC_USR_MIN = 5,
    ADC_USR_MAX = 228,
    ADC_S1_MIN,
    ADC_S1_MAX  = 512, 
    ADC_S2_MIN,
    ADC_S2_MAX  = 854
};

typedef struct
{
    ButtonName name;
    unsigned int event;     //按键事件:点击(001)，释放(100)，长按(010)
    ButtonCallback callback;        //回调函数
} ButtonInfo;

typedef struct              //按键事件发生时的标记
{
    int pressClicked;       
    int longPressClicked;   
    int longPressInterval;  //记录按键按下后的时间间隔
    int releaseClicked;     
} ButtonClicked;

static volatile int ButtonIsInit =0;     //标记按键是否初始化
static volatile int ButtonIsClick = 0;   //标记按键是否按下

static volatile ButtonInfo BtnInfo[MAX_KEY_NUM] = {0};      
static volatile ButtonClicked BtnClicked[MAX_KEY_NUM] = {0};

static void OnButtonPressed(char *arg)
{
    static volatile hi_u64 preTick=0;
    hi_gpio_idx gpio=(hi_gpio_idx)arg;
    hi_u64 tick=hi_systick_get_cur_tick();

    ButtonIsClick=(tick-preTick)>PRESS_INTERVAL;        //按键消抖 0.625s内的按键按下事件是无效的

    if(ButtonIsClick)
    {
        preTick=tick;
        BtnClicked[gpio].pressClicked=1;        //对应按键按下
        hi_gpio_register_isr_function(gpio,HI_INT_TYPE_EDGE,HI_GPIO_EDGE_RISE_LEVEL_HIGH,OnButtonReleased,arg);
    }
}

static void OnButtonReleased(char* arg)
{
    hi_gpio_idx gpio=(hi_gpio_idx)arg;

    if(ButtonIsClick)
    {
        BtnClicked[gpio].releaseClicked=1;

        hi_gpio_register_isr_function(gpio,HI_INT_TYPE_EDGE,HI_GPIO_EDGE_FALL_LEVEL_LOW,OnButtonPressed,arg);
    }
}

static int GetButtonAdc(void)
{
    unsigned short data = 0;
    int ret = None;

    if(hi_adc_read(HI_ADC_CHANNEL_2, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == 0)
    {
        if( (ADC_USR_MIN <= data) && (data <= ADC_USR_MAX) )  ret = USR;
        if( (ADC_S1_MIN  <= data) && (data <= ADC_S1_MAX ) )  ret = S1;
        if( (ADC_S2_MIN  <= data) && (data <= ADC_S2_MAX ) )  ret = S2;
    }
    return ret;
}

static void AdcEventTrigger(void)
{
    static hi_u64 preTick =0;
    static int preKey = None;

    int curKey = GetButtonAdc();

    if((preKey==None)&&(curKey != None))
    {
        hi_u64 tick=hi_systick_get_cur_tick();
        int IsClick = (tick -preTick)>PRESS_INTERVAL;

        if(IsClick)
        {
            BtnClicked[curKey].pressClicked = 1;
            preKey=curKey;
            preTick=tick;
        }
    }
    else if((preKey!=None)&&(curKey==None))
    {
        BtnClicked[preKey].releaseClicked=1;
        preKey=curKey;
    }
}

static void FinalEventHandler(void)
{
    for(size_t i=0;i<MAX_KEY_NUM;++i)
    {
        if(BtnClicked[i].pressClicked)
        {
            if(BtnInfo[i].event & Pressed)  //event与Pressed(0001)做位或操作,若event包含Pressed操作则结果为真，否则为假
                BtnInfo[i].callback(Pressed);
            BtnClicked[i].pressClicked=0;
            BtnClicked[i].longPressInterval=0;
        }

        if(BtnClicked[i].longPressInterval<LONG_PRESS_END)  //65535    65535/25=2621.4s   这个数可以是比2.56s大的任何数
        {
            BtnClicked[i].longPressInterval++;              //长按按键时，每大约40ms计数一次，1/0.04=25次/s
        }

        if(BtnClicked[i].longPressInterval == LONG_PRESS_INTERVAL)  //64/25=2.56s,长按按键2.5s左右就会触发一次长按按键事件
        {
            BtnClicked[i].longPressClicked = 1; //长按按键事件标记
        }

        if(BtnClicked[i].longPressClicked)
        {
            if(BtnInfo[i].event & LongPressed)//0110 & 0100
                BtnInfo[i].callback(LongPressed);
            
            BtnClicked[i].longPressClicked = 0;
            BtnClicked[i].longPressInterval = LONG_PRESS_END;
        }

        if(BtnClicked[i].releaseClicked)
        {
            if(BtnInfo[i].event & Released)
                BtnInfo[i].callback(Released);

            BtnClicked[i].releaseClicked = 0;
            BtnClicked[i].longPressInterval= LONG_PRESS_END;
        }
    }
}

static void *Button_Task(void* arg)
{
    while(ButtonIsInit)
    {
        AdcEventTrigger();
        FinalEventHandler();
        usleep(LOOP_INTERVAL);      //等待40ms
    }
    return arg;
}

int Button_Init(void)   //创建按键线程
{
    int ret = hi_gpio_init();     //检查初始化gpio功能，Success(0)

    if(ret == (int)HI_ERR_GPIO_REPEAT_INIT)   //处理gpio重复初始化
    {
        ret = 0;
    }

    if(!ret && !ButtonIsInit)     //如果gpio成功初始化且按键功能没有初始化过
    {
        osThreadAttr_t attr;      //创建按键线程

        for(size_t i=0;i<MAX_KEY_NUM;++i)
        {
            BtnClicked[i].longPressInterval=LONG_PRESS_END;
        }

        attr.name = "ButtonTask";
        attr.attr_bits = 0U;
        attr.cb_mem = NULL;
        attr.cb_size = 0U;
        attr.stack_mem = NULL;
        attr.stack_size = BUTTON_STACK_SIZE;
        attr.priority = osPriorityNormal;

        ret += (osThreadNew((osThreadFunc_t)Button_Task, NULL, &attr) == NULL);     //线程创建成功则ret为0，否则为1
        ButtonIsInit = (ret==0);    //按键线程创建成功,ButtonIsInit标记为1,否则为0
    }
    return ret;     //Success(0)
}

void Button_Deinit(void) //按键功能解初始化
{
    ButtonIsInit = 0;
}

int Button_Enable(ButtonName name,ButtonCallback callback,unsigned int event)     //使能指定按键及其功能
{
    int ret =-1;

    if(callback)
    {
        switch (name)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            ret =  hi_io_set_func((hi_io_name)name,0);
            ret += hi_io_set_pull((hi_io_name)name,HI_IO_PULL_UP);   
            ret += hi_gpio_set_dir((hi_gpio_idx)name,HI_GPIO_DIR_IN);
            ret += hi_gpio_register_isr_function((hi_gpio_idx)name,HI_INT_TYPE_EDGE,HI_GPIO_EDGE_FALL_LEVEL_LOW,OnButtonPressed,(char*)name);
            break;
        case 13:
        case 14:
            ret =  hi_io_set_func((hi_io_name)name,4);
            ret += hi_io_set_pull((hi_io_name)name,HI_IO_PULL_UP);   
            ret += hi_gpio_set_dir((hi_gpio_idx)name,HI_GPIO_DIR_IN);
            ret += hi_gpio_register_isr_function((hi_gpio_idx)name,HI_INT_TYPE_EDGE,HI_GPIO_EDGE_FALL_LEVEL_LOW,OnButtonPressed,(char*)name);
            break;
        case 15:
        case 16:
        case 17:
            ret =0;
            break;
        default:
            break;
        }
        if(ret == 0)
        {
            BtnInfo[name].name=name;
            BtnInfo[name].event=event;
            BtnInfo[name].callback=callback;
        }
    }
    return ret;     //Success(0)
}

void Button_Disable(ButtonName name)   //去使能指定按键
{
    BtnInfo[name].name=BNone;
    BtnInfo[name].event=0;
    BtnInfo[name].callback=0;
}