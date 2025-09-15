/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 18:24:33 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 01:14:51
 */
#ifndef _DEVICE_DATA_H_
#define _DEVICE_DATA_H_

#include <stdio.h>
#include "lvgl.h"
#include "wpa_manager.h"

#define WEATHER_CITY "guangzhou"
#define WEATHER_KEY "SmazqPcltzTft-X3v"

typedef enum{
    OFF = 0,
    ON,
}SWITCH_STATE_E;

typedef enum{
    DISCONNECT = 0,
    CONNECTING,
    CONNECT,
}CONNECT_STATE_E;

typedef enum{
    TIME_TYPE_1 = 0,
    TIME_TYPE_2,
    TIME_TYPE_3,
}TIME_SHOW_TYPE_E;

typedef enum{
    NOTICE_TYPE_ONLY_WIN = 0,
    NOTICE_TYPE_WIN_AND_AUDIO,
}NOTICE_TYPE_E;

// 定义设备状态结构体，包含各模块连接状态、传感器数据、LED状态、时间、电量、WiFi、音量等
typedef struct {
    CONNECT_STATE_E ble_mesh_state; // BLE Mesh 模组连接状态
    CONNECT_STATE_E led_connect_state; // LED 模组连接状态
    CONNECT_STATE_E switch_connect_state; // 开关模块连接状态
    CONNECT_STATE_E body_sensor_connect_state; // 人体传感器连接状态
    CONNECT_STATE_E smart_coaster_connect_state; // 智能杯垫连接状态
    CONNECT_STATE_E flame_sensor_connect_state; // 火焰传感器连接状态

    uint8_t rgbled_battery_level; // RGB LED 电量
    uint8_t switch_battery_level; // 开关模块电量
    uint8_t body_sensor_battery_level; // 人体传感器电量
    uint8_t smart_coaster_battery_level; // 智能杯垫电量
    uint8_t flame_sensor_battery_level; // 火焰传感器电量

    SWITCH_STATE_E led_r; // 红色 LED 开关状态
    SWITCH_STATE_E led_g; // 绿色 LED 开关状态
    SWITCH_STATE_E led_b; // 蓝色 LED 开关状态

    SWITCH_STATE_E mesh_switch_state; // Mesh 开关模块状态

    SWITCH_STATE_E body_sensor_state; // 人体传感器状态
    uint8_t body_sensor_data; // 人体传感器数据
    NOTICE_TYPE_E body_sensor_notice_type; // 人体传感器提示类型

    SWITCH_STATE_E smart_coaster_state; // 智能杯垫状态
    uint8_t smart_coaster_data; // 智能杯垫数据
    uint32_t smart_coaster_time; // 智能杯垫计时总时间（单位：分钟）
    uint32_t smart_coaster_remaining_time; // 智能杯垫剩余时间

    SWITCH_STATE_E flame_sensor_state; // 火焰传感器状态
    uint8_t flame_sensor_data; // 火焰传感器数据
    uint32_t flame_sensor_time; // 火焰传感器计时总时间（单位：分钟）
    uint32_t flame_sensor_remaining_time; // 火焰传感器剩余时间

    SWITCH_STATE_E tomato_time_state; // 番茄钟状态
    uint32_t tomato_learn_time; // 学习时间总量
    uint32_t tomato_learn_remaining_time; // 学习剩余时间

    uint32_t tomato_rest_time; // 休息时间总量
    uint32_t tomato_rest_remaining_time; // 休息剩余时间

    char weather_info[50]; // 天气信息
    char weather_city[20]; // 城市名称

    TIME_SHOW_TYPE_E time_type; // 时间显示类型
    int clock_type; // 时钟类型

    WPA_WIFI_STATUS_E wifi_state; // WiFi 开启状态
    WPA_WIFI_CONNECT_STATUS_E wifi_connect_state; // WiFi 连接状态

    int brightness_value; // 屏幕亮度
    int volume_value; // 音量

    bool is_open_type1; // 开关类型 1 是否打开
    bool is_open_type2; // 开关类型 2 是否打开
    int alarm_time1; // 闹钟 1
    int alarm_time2; // 闹钟 2

    bool is_disp_orientation; // 是否显示屏幕方向
} device_state_t;


void init_device_state(void);

device_state_t* get_device_state(void);

void device_timer_init();

void device_param_write(void);

void device_param_read(void);

void update_wpa_manager_callback_fun(void);

#endif
