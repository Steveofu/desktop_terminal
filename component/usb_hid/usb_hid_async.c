#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include "usb_hid_utils.h"
#include "systemv_queue.h"


#define hid_queue_path "/tmp/hid_queue"

typedef enum
{
    HDI_TYPE_ID_CRTL_ALT = 0,
    HDI_TYPE_ID_CRTL,
}HDI_TYPE_ID_E;

typedef struct
{
    HDI_TYPE_ID_E id;
    char c;
}hid_obj;

static int hid_queue_id = -1;
static pthread_t hid_thread = 0;


static void handle_hid(int hid_queue_id)
{
    int ret = -1;
    
    hid_obj obj;
    memset(&obj, 0, sizeof(hid_obj));

    ret = systemv_queue_rcv(hid_queue_id, &obj,sizeof(obj), IPC_NOWAIT);
    if (ret == 0)
    {
        HDI_TYPE_ID_E id = obj.id;
        switch(id)
        {
            case HDI_TYPE_ID_CRTL_ALT:
                send_ctrl_alt_combination(obj.c);
                break;
            case HDI_TYPE_ID_CRTL:
                send_ctrl_combination(obj.c);
                break;
            default:
                break;
        }
    }
}

static void* hid_thread_fun(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    while(1)
    {
        handle_hid(hid_queue_id);
        usleep(1000);
    }
}


// 模拟按下 CTRL + ALT + 字符 的组合键
void send_ctrl_alt_combination_async(char c){
    hid_obj obj;    
    memset(&obj, 0, sizeof(hid_obj));
    obj.id = HDI_TYPE_ID_CRTL_ALT;
    obj.c = c;
    int ret = systemv_queue_send(hid_queue_id, &obj, sizeof(hid_obj));
    if(ret == -1)
    {
        printf("queue send error");
    }
}

// 模拟按下 CTRL + 字符 的组合键
void send_ctrl_combination_async(char c){
    hid_obj obj;    
    memset(&obj, 0, sizeof(hid_obj));
    obj.id = HDI_TYPE_ID_CRTL;
    obj.c = c;
    int ret = systemv_queue_send(hid_queue_id, &obj, sizeof(hid_obj));
    if(ret == -1)
    {
        printf("queue send error");
    }
}

int init_async_usb_hid(void)
{
    static int ret = -1;
    if(ret == -1)
        return 0;

    ret = systemv_queue_create(hid_queue_path,hid_queue_id,sizeof(hid_obj),50);
    if(ret == -1)
    {
        printf("create queue error");
        return -1;
    }   

    ret = pthread_create(&hid_thread,NULL,hid_thread_fun, NULL);
    if(ret == -1)
    {
        printf("create thread error");
        return -1;
    }

    hid_keyboard_init();
    ret = 0;

    return 0;
}