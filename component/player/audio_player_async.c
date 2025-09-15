#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>        // POSIX 线程
#include <unistd.h>        // POSIX 标准函数库
#include <errno.h>         // 错误码定义    
#include "systemv_queue.h" // System V 消息队列接口
#include "em_hal_audio.h"   // 音频硬件抽象层接口（播放/停止音频）

#define audio_queue "/tmp/audio_queue"  // 消息队列文件路径

typedef enum {
    AUDIO_COMM_ID_START,  // 开始播放音频
    AUDIO_COMM_ID_STOP,   // 停止播放音频
} AUDIO_COMM_ID;


typedef struct {
    AUDIO_COMM_ID id;        // 命令 ID，指定执行的操作
    char file_name[256];     // 音频文件名/路径（仅在 START 命令时有效）
} audio_obj;

static pthread_t audio_thread;          // 音频处理线程句柄
static int audio_queue_id = -1;            // 音频消息队列 ID


/**
 * @brief 音频处理线程
 *
 * 该线程循环等待消息队列中的命令，并执行对应的操作。
 * - 支持线程取消（pthread_cancel）
 * - 消息超时等待 100ms
 *
 * @param arg 线程参数，这里未使用
 * @return void* 永不返回
 */
static void* audio_thread_fun(void *arg) {
    // 设置线程可被取消（支持 pthread_cancel）
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // 设置取消方式为“延迟取消”，避免在关键代码段被强制打断，防止资源泄露
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) 
    {
        // 1. 初始化消息对象（清零，避免使用脏数据）
        audio_obj obj;
        memset(&obj, 0, sizeof(audio_obj));

        int n = systemv_queue_rcv(audio_queue_id, &obj, sizeof(obj),IPC_NOWAIT);
        if (n > 0) 
        {
            printf("Received cmd: %d\n", obj.id);

            // 3. 根据命令 ID 执行操作
            if (obj.id == AUDIO_COMM_ID_START) 
            {
                em_stop_play_audio();             // 停止当前播放
                em_play_audio(obj.file_name);     // 播放指定文件
            } else if (obj.id == AUDIO_COMM_ID_STOP) 
            {
                em_stop_play_audio();             // 停止播放
            }
        }

        // 4. 主动休眠 1ms，降低 CPU 占用
        usleep(1000); // 1ms
    }
}



void start_play_audio_async(char *url) {
    // 1. 构造消息对象
    audio_obj obj = {.id = AUDIO_COMM_ID_START};
    strncpy(obj.file_name, url, sizeof(obj.file_name)-1); // 拷贝文件路径，确保结尾有 '\0'

    // 2. 发送消息到队列（超时时间 1000ms）
    if (systemv_queue_send(audio_queue_id, &obj, sizeof(audio_obj)) != 0) {
        printf("Failed to send start cmd\n");
    }
}


/**
 * @brief 初始化异步音频播放器
 *
 * 功能：
 *   1. 创建音频消息队列，用于存放待播放的命令。
 *   2. 创建音频处理线程，从队列中取出命令并执行。
 *
 * @return 0 成功
 *        -1 失败
 */

int init_async_audio_player(void)
{
    // 初始化 System V 消息队列
    audio_queue_id = systemv_queue_create(audio_queue, M_QUEUE_ID, 50, sizeof(audio_obj));
    if(audio_queue_id < 0)
    {
        printf("Init audio play failed\n");
        return -1;
    }

    if(pthread_create(&audio_thread, NULL, audio_thread_fun, NULL) != 0)
    {
        printf("Init audio play failed\n");
        // 创建线程失败，释放已创建的消息队列，避免资源泄漏
        systemv_queue_delete(audio_queue_id);
        audio_queue_id = -1;

        return -1;
    }

     pthread_detach(audio_thread);

    return 0;
}


