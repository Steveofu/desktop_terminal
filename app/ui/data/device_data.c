#include "device_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "file_save.h"
#include "http_manager.h"
#include "em_hal_audio.h"
#include "em_hal_brightness.h"
#include "page_conf.h"


// #define USE_SYSTEM_TIMER 1
#ifndef USE_SYSTEM_TIMER
    #define USE_LVGL_TIMER 1
#endif

device_state_t device_state;

static int time_s = 0;
static int time_m = 0;
static int time_h = 0;

static time_t timep;
static struct tm time_temp;

static void warn_time_count_handle(){
    device_state_t* device_state = get_device_state();
    if(device_state->body_sensor_connect_state == CONNECT && device_state->body_sensor_state == ON &&  device_state->body_sensor_data == 1)
    {
        init_page_warn(WARN_BODY_SENSOR_TRIGGER);
    }
    if(device_state->smart_coaster_connect_state == CONNECT && device_state->smart_coaster_state == ON &&  device_state->smart_coaster_remaining_time == 0){
        init_page_warn(WARN_SMART_COASTER_TRIGGER);
        device_state->smart_coaster_remaining_time = device_state->smart_coaster_time;
    }
    if(device_state->flame_sensor_connect_state == CONNECT && device_state->flame_sensor_state == ON &&  device_state->flame_sensor_remaining_time == 0){
        init_page_warn(WARN_FLAME_SENSOR_TRIGGER);
        device_state->flame_sensor_remaining_time = device_state->flame_sensor_time;
    }
}

static void alarm_time_count_min(){
    if(device_state.is_open_type1){
        if(device_state.alarm_time1 > 0){
            device_state.alarm_time1 --;
        }
        if(device_state.alarm_time1 == 0){
            //TODO alarm
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
            device_state.is_open_type1 = false;
        }
    }
    if(device_state.is_open_type2){
        time(&timep);
        memcpy(&time_temp, localtime(&timep), sizeof(struct tm));
        if((time_temp.tm_hour*60+time_temp.tm_min) == device_state.alarm_time2){
            //TODO alarm
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
            device_state.is_open_type2 = false;
        }
    }
}

static void timer_count(){
    time_s ++;
    warn_time_count_handle();
    //min
    if(time_s >= 60){
        time_m ++;
        time_s = 0;
        alarm_time_count_min();
        if(device_state.flame_sensor_state == ON){
            if(device_state.flame_sensor_remaining_time > 0){
                device_state.flame_sensor_remaining_time --;
            }
        }
        if(device_state.smart_coaster_state == ON){
            if(device_state.smart_coaster_remaining_time > 0){
                device_state.smart_coaster_remaining_time --;
            }
        }
        if(device_state.tomato_time_state == ON){
            if(device_state.tomato_learn_remaining_time > 0){
                device_state.tomato_learn_remaining_time --;
            }
            if(device_state.tomato_learn_remaining_time == 0){
                if(device_state.tomato_rest_remaining_time > 0)
                    device_state.tomato_rest_remaining_time --;
            }
        }
    }
    //hours
    if(time_m >= 60){
        time_h ++;
        time_m = 0;
    }
    //day
    if(time_h >= 24){
        time_h = 0;
    }
}

#ifdef USE_LVGL_TIMER
static void lv_timer_cb_func(lv_timer_t * timer)
{
    // printf("lv_timer_cb_func timer run ...\n");
    timer_count();
}
#elif USE_SYSTEM_TIMER
static void timer_handler(int signum) {
    static int count = 0;
    printf("Timer tick %d\n", ++count);
    timer_count();
}
#endif


/**
 * @brief 初始化设备定时器
 *
 * 根据编译选项选择不同的定时器实现方式：
 * - USE_LVGL_TIMER   使用 LVGL 内部定时器（lv_timer）
 * - USE_SYSTEM_TIMER 使用系统级定时器（setitimer + SIGALRM）
 */
void device_timer_init() {
    printf("device_timer_init\n");

#ifdef USE_LVGL_TIMER
    /**
     * 使用 LVGL 定时器：
     * 参数：
     *   lv_timer_cb_func : 定时器回调函数
     *   1000             : 定时周期，单位 ms（1秒）
     *   NULL             : 用户数据（传递给回调）
     */
    lv_timer_t *device_timer = lv_timer_create(lv_timer_cb_func, 1000, NULL);

#elif USE_SYSTEM_TIMER
    struct itimerval system_timer;

    // 1. 注册 SIGALRM 信号的处理函数
    signal(SIGALRM, timer_handler);

    // 2. 配置定时器初始值和周期
    system_timer.it_value.tv_sec     = 1; // 首次触发时间：1 秒
    system_timer.it_value.tv_usec    = 0; // 微秒部分
    system_timer.it_interval.tv_sec  = 1; // 周期触发时间：1 秒
    system_timer.it_interval.tv_usec = 0; // 微秒部分

    // 3. 启动定时器
    if (setitimer(ITIMER_REAL, &system_timer, NULL) == -1) {
        perror("setitimer error");
    }
#endif
}

void device_param_write(){
    int ret = file_param_write("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_write success\n");
}

void device_param_read(){
    int ret = file_param_read("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_read success ble_mesh_state = %d \n",device_state.ble_mesh_state);
}

static void weather_callback_func(char *data){
    printf("---->%s\n",data);
    strcpy(device_state.weather_info,data);
}

static void connect_status_callback_func(WPA_WIFI_CONNECT_STATUS_E status){
    printf("davicedata----->connect_status_callback_func: %d\n", status);
    device_state.wifi_connect_state = status;
}

static void wifi_status_callback_func(WPA_WIFI_STATUS_E status){
    printf("----->wifi_status_callback_func: %d\n", status);
    device_state.wifi_state = status;
}

void update_wpa_manager_callback_fun(){
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

void init_device_state(){

    memset(&device_state,0,sizeof(device_state_t));

    device_state.time_type = TIME_TYPE_3;
    device_state.clock_type = 2;
    device_state.is_disp_orientation = false;

    device_param_read();

    device_state.led_r = 0;
    device_state.led_g = 0;
    device_state.led_b = 0;

    device_state.led_connect_state = DISCONNECT;
    device_state.switch_connect_state = DISCONNECT;
    device_state.mesh_switch_state = OFF;
    device_state.body_sensor_state = OFF;

    device_state.smart_coaster_connect_state = DISCONNECT;
    device_state.smart_coaster_state = OFF;
    if(device_state.smart_coaster_time == 0){
        device_state.smart_coaster_time = 20;
        device_state.smart_coaster_remaining_time = 20;
    }

    device_state.body_sensor_connect_state = DISCONNECT;
    device_state.body_sensor_state = OFF;
    device_state.body_sensor_notice_type = NOTICE_TYPE_WIN_AND_AUDIO;
    
    device_state.flame_sensor_connect_state = DISCONNECT;
    device_state.flame_sensor_state = OFF;
    device_state.flame_sensor_data = 0;
    if(device_state.flame_sensor_time == 0){
        device_state.flame_sensor_time = 20;
        device_state.flame_sensor_remaining_time = 20;
    }

    device_state.switch_battery_level = 0;
    device_state.body_sensor_battery_level = 0;
    device_state.flame_sensor_battery_level = 0;
    device_state.smart_coaster_battery_level = 0;

    device_state.wifi_connect_state = WPA_WIFI_INACTIVE;
    device_state.ble_mesh_state = DISCONNECT;

    device_state.is_open_type1 = false;
    device_state.is_open_type2 = false;

    device_state.brightness_value = em_hal_brightness_get_value();
    if(device_state.brightness_value == 0){
        //閬垮厤榛戝睆鎯呭喌
        em_hal_brightness_set_value(10);
    }
    if(device_state.volume_value != 0){
        em_set_audio_vol(device_state.volume_value);
    }else
        device_state.volume_value = em_get_audio_vol();

    if(strlen(device_state.weather_city) == 0)
        strcpy(device_state.weather_city,"guangzhou");
    if(strlen(device_state.weather_info) == 0)
        strcpy(device_state.weather_info,"骞垮窞 澶氫簯 26掳C");

    

    http_set_weather_callback(weather_callback_func);
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

device_state_t* get_device_state(){
    return &device_state;
}