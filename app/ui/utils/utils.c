/*
 * @Author: xiaozhi 
 * @Date: 2024-09-25 00:07:46 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:56:32
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "lvgl.h"
#include "utils.h"

static char time[20];

/**
 * 杩斿洖鍒嗛挓杞?鎹㈢殑灏忔椂銆佸垎閽熷瓧绗︿覆 鍗曚綅鏁拌ˉ0
 */
char *get_time_str(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d:%d",time_h,time_m);
        }else{
            sprintf(time,"%d:0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d:%d",time_h,time_m);
        }else{
            sprintf(time,"0%d:0%d",time_h,time_m);
        }
    }
    return time;
}   

/**
 * 杩斿洖鍒嗛挓杞?鎹㈢殑灏忔椂銆佸垎閽熷瓧绗︿覆 鍗曚綅鏁拌ˉ0
 */
char *get_time_str_nosymbol(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"%d#000000 :#0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"0%d#000000 :#0%d",time_h,time_m);
        }
    }
    return time;
}


/**
 * @brief 信号回调函数
 * 
 * 当进程收到指定的信号时，会自动调用该函数。
 * 
 * @param sig_no 捕获到的信号编号
 */
static void signal_callback_func(int sig_no) {
    // 打印接收到的信号编号
    printf("signal %d, exiting ...\n", sig_no);

    // 直接退出进程，不进行标准的清理操作（如关闭文件、刷新缓冲区等）
    // 使用 _exit 而不是 exit，避免信号处理函数中再次触发异常。
    _exit(1);

    // 这一行不会被执行，因为 _exit() 调用后进程立即终止
    printf("retry _exit ...\n");
}

/**
 * @brief 初始化系统信号处理函数
 * 
 * 通过 signal() 函数，将多个常见的系统信号与 signal_callback_func 绑定。
 * 当程序收到这些信号时，会调用回调函数进行处理（打印信息并退出）。
 */
void system_signal_init() {
    // 终端中断 (Ctrl + C)
    signal(SIGINT, signal_callback_func);

    // 终端退出 (Ctrl + \)
    signal(SIGQUIT, signal_callback_func);

    // 进程被 kill 等方式终止
    signal(SIGTERM, signal_callback_func);

    // 段错误（无效的内存访问）
    signal(SIGSEGV, signal_callback_func);

    // 异常终止（调用 abort()）
    signal(SIGABRT, signal_callback_func);

    // 总线错误（内存对齐等硬件错误）
    signal(SIGBUS, signal_callback_func);

    // 浮点异常（除以零、浮点溢出等）
    signal(SIGFPE, signal_callback_func);

    // 非法指令（CPU 无法识别的指令）
    signal(SIGILL, signal_callback_func);

    // 终端挂起 (Ctrl + Z)
    signal(SIGTSTP, signal_callback_func);
}


// typedef enum {
//     WIFI_DISCONNECTED=0,
//     WIFI_CONNECTED,
//     INVAILD_STATUS,
// }WIFI_STATUS_E;

// #define READ_WIFI_STATUS_CMD  "cat /sys/class/net/%s/carrier"
// #define WIFI_INTERFACE  "wlan0"

// WIFI_STATUS_E  get_wifi_status(void)
// {
//     int status = 0;
//     int bytes = 0;
//     char cmd[100];
//     char buf[500];
//     char *p =NULL;
//     memset(cmd,0,sizeof(cmd));
//     memset(buf,0,sizeof(buf));
//     sprintf(cmd,READ_WIFI_STATUS_CMD,WIFI_INTERFACE);
//     if(bytes > 0){
//         status = atoi(buf);
//         if(status == 1){
//             return WIFI_CONNECTED;
//         }
//         else if(status == 0){
//              return WIFI_DISCONNECTED;
//         }
//         else
//             return INVAILD_STATUS;
//     }
// }