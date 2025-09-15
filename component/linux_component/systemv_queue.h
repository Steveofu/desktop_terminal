#ifndef __SYSTEMV_QUEUE_H__
#define __SYSTEMV_QUEUE_H__

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define M_QUEUE_ID 1

/* 消息结构定义 */
typedef struct {
    long mtype;       // 消息类型，必须 >0
    char mtext[1024]; // 消息正文（可调整大小）
} sysv_msg_t;


int systemv_queue_create(const char *path, int proj_id,int msg_size,int queue_size);

int systemv_queue_send(int g_msgid,const void *msg, uint32_t len);

//0 → 默认阻塞，队列没消息就挂起等待。
// IPC_NOWAIT → 非阻塞，
int systemv_queue_rcv(int g_msgid, void *msg, uint32_t len,int msg_flag);

int systemv_queue_delete(int g_msgid);

#endif
