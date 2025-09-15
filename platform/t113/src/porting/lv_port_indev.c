#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include "lv_port_indev.h"
#include "lvgl.h"
#include "port_conf.h"

// evdev 文件描述符
int evdev_fd = -1;

// 当前触摸点坐标
int lv_indev_x;
int lv_indev_y;

// 当前触摸状态（按下/释放）
int lv_indev_state;

// LVGL 输入设备驱动结构体
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev;

/**
 * 初始化 evdev（Linux 输入设备接口）
 */
int evdev_init(void)
{
    // 打开 evdev 设备文件（例如 /dev/input/event0）
    evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
    if(evdev_fd == -1) 
    {
        perror("unable to open evdev interface:"); // 打开失败
        return -1;
    }

    // 设置文件描述符为异步非阻塞
    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
     
    // 初始化触摸状态
    lv_indev_x = 0;
    lv_indev_y = 0;
    lv_indev_state = LV_INDEV_STATE_REL; // 默认释放状态
    return 0;
}

/**
 * 获取当前触摸屏位置和状态（回调函数）
 * @param drv   指向 LVGL 驱动结构体
 * @param data  输出触摸数据
 */
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct input_event in;

    // 循环读取所有输入事件
    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0) 
    {
        if(in.type == EV_ABS) // 绝对坐标事件
        {
            if(in.code == ABS_MT_POSITION_X) // X 坐标
            {
                lv_indev_x = in.value;
            }
            else if(in.code == ABS_MT_POSITION_Y) // Y 坐标
            {
                lv_indev_y = in.value;
            }
        }
        else if(in.type == EV_KEY) // 按键事件
        {
            if(in.code == BTN_TOUCH) // 触摸按下/释放
            {
                if(in.value == 0)
                    lv_indev_state = LV_INDEV_STATE_REL; // 释放
                else if(in.value == 1)
                    lv_indev_state = LV_INDEV_STATE_PR;  // 按下
            }
        }
    }
    
    // 将当前触摸状态传给 LVGL
    data->point.x = lv_indev_x; 
    data->point.y = lv_indev_y;
    data->state = lv_indev_state;

    // 调试输出
    // printf("lv_indev_x=%d x,y=%d,%d  state=%d\n", lv_indev_x, data->point.x, data->point.y, data->state);
}

/**
 * 初始化 LVGL 输入设备（触摸屏）
 */
void lv_port_indev_init(void)
{
    // 初始化 evdev 设备
    evdev_init();

    // 初始化 LVGL 输入设备驱动
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;  // 指针类型（触摸/鼠标）
    indev_drv.read_cb = evdev_read;          // 设置读取回调函数

    // 注册输入设备到 LVGL
    indev = lv_indev_drv_register(&indev_drv);
}

/**
 * 反初始化 LVGL 输入设备
 */
void lv_port_indev_deinit(void)
{
    // 删除 LVGL 输入设备，释放资源
    lv_indev_delete(indev);
}
