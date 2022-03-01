#ifndef BUTTON_HI3861_H
#define BUTTON_HI3861_H

/*
    Description: 
        Button name
*/
typedef enum
{
    GPIO_0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    USR,
    S1,
    S2,
    BNone
}ButtonName;

/*
    Description: 
        Button Event ID
*/
typedef enum
{
    None=0,//0000
    Pressed=1,//0001
    LongPressed=2,//0010
    Released=4//0100
}ButtonEvent;

/*
    Description: 
        Button event callback function pointer type.
    Parameter:
        event  -- event ID which trigger the function call   
*/
typedef void (*ButtonCallback)(ButtonEvent arg);

/*
    Description: 
        To initialize button function
    Parameter:
        None
    Return Value:
        0     -- Success
        other -- Failure
*/
int Button_Init(void);

/*
    Description:
        To deinitialize button function
    Parameter:
        None
    Return Value:
        None
*/
void Button_Deinit(void);

/*
    Description:
        To register callback functions for a GPIO button.
    Parameter:
        name     -- target GPIO port name for a phisical button
        callback -- callback function for button event
        event    -- the target button event to trigger callback
    Return Value:
        0     -- Success
        other -- Failure
*/
int Button_Enable(ButtonName name,ButtonCallback callback,unsigned int event);

/*
    Description:
        To unregister callback functions for a GPIO button.
    Parameter:
        name -- target GPIO port name for a phisical button
    Return Value:
        None
*/
void Button_Disable(ButtonName name);

#endif