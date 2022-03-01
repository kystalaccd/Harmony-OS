#ifndef HEALTH_DATA_H
#define HEALTH_DATA_H

#include <stdio.h>

#define HEALTH_DATA_PER_TIME 340    //每次更新健康数据的数目，注意避免数目太大导致内存不足




typedef unsigned int HD_len;    //健康数据的字节长度





void health_data_update(void);

#endif