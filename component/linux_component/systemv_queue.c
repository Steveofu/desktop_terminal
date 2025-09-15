#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "systemv_queue.h"



int systemv_queue_create(const char *path, int proj_id,int queue_size,int msg_size) 
{
    struct msqid_ds buf;

    if(path == NULL || proj_id <= 0 || proj_id > 255) {
        fprintf(stderr, "Invalid path or proj_id\n");
        return -1;
    }

    // 浣跨敤 ftok 鏍规嵁鏂囦欢璺�寰� + 椤圭洰 ID 鐢熸垚鍞�涓€ key
    // key 浼氳��鐢ㄦ潵鏍囪瘑 System V IPC 瀵硅薄锛堟秷鎭�闃熷垪銆佷俊鍙烽噺銆佸叡浜�鍐呭瓨锛�
    key_t key = ftok(path, proj_id);
    if (key == -1) {
        perror("ftok");   // 鎵撳嵃閿欒��鍘熷洜锛堜緥濡� path 涓嶅瓨鍦�锛�
        return -1;
    }

    // 鏍规嵁 key 鍒涘缓鎴栬幏鍙栨秷鎭�闃熷垪
    // IPC_CREAT 琛ㄧず濡傛灉闃熷垪涓嶅瓨鍦ㄥ垯鏂板缓
    // 鏉冮檺 0666 琛ㄧず鎵€鏈夌敤鎴峰彲璇诲彲鍐�
    int g_msgid = msgget(key, IPC_CREAT | 0666);
    if (g_msgid == -1) {
        perror("msgget"); // 鎵撳嵃閿欒��鍘熷洜锛堜緥濡傛潈闄愪笉瓒筹級
        return -1;
    }

    // 璇诲彇褰撳墠闃熷垪灞炴€�
    if (msgctl(g_msgid, IPC_STAT, &buf) == -1) {
        perror("msgctl IPC_STAT");
        return 1;
    }

    // 淇�鏀归槦鍒楀ぇ灏忥紙鏈€澶у瓧鑺傛暟锛�
    buf.msg_qbytes = queue_size * msg_size; 

    // 璁剧疆鏂扮殑灞炴€�
    if (msgctl(g_msgid, IPC_SET, &buf) == -1) {
        perror("msgctl IPC_SET");
        return 1;
    }

    printf("Queue size set to %lu bytes\n", buf.msg_qbytes);

    // 杩斿洖娑堟伅闃熷垪 ID锛岀敤浜庡悗缁� msgsnd/msgrcv/msgctl
    return g_msgid;
}


int systemv_queue_send(int g_msgid,const void *msg, uint32_t len)
{
    if(g_msgid == -1) {
        fprintf(stderr, "Queue not initialized!\n");
        return -1;
    }
    if(msg == NULL || len == 0) {
        fprintf(stderr, "Invalid message or length\n");
        return -1;
    }

    sysv_msg_t sbuf;

    if (g_msgid == -1) {
        fprintf(stderr, "Queue not initialized!\n");
        return -1;
    }

    if(len>sizeof(sbuf.mtext)) {
        fprintf(stderr, "Message too long\n");
        return -1;
    }

    sbuf.mtype = 1; // 娑堟伅绫诲瀷锛屽彲浠ユ牴鎹�闇€瑕佽�剧疆
    memcpy(sbuf.mtext,msg,len);

    // 0=闃诲�烇紝IPC_NOWAIT=闈為樆濉烇級
    if(msgsnd(g_msgid,&sbuf,len,0) == -1) 
    {
        perror("msgsnd");
        return -1;
    }
    return 0;
}


int systemv_queue_rcv(int g_msgid, void *msg, uint32_t len,int msg_flag)
{
    if(g_msgid == -1) {
        fprintf(stderr, "Queue not initialized!\n");
        return -1;
    }
    if(msg == NULL || len == 0) {
        fprintf(stderr, "Invalid message or length\n");
        return -1;
    }

    sysv_msg_t rsv_buf;
    int ret = 0;

    // 浠庨槦鍒楁帴鏀舵秷鎭�锛坢sgtyp=0 琛ㄧず鎺ユ敹浠绘剰绫诲瀷锛�
    if((ret=msgrcv(g_msgid,&rsv_buf,len,0,msg_flag)) == -1) 
    {
        perror("msgrcv");
        return -1;
    }
    if(ret > len) {
        fprintf(stderr, "Received message too long\n");
        return -1;
    }

    memcpy(msg,rsv_buf.mtext,ret);

    return ret; // 杩斿洖瀹為檯鎺ユ敹鐨勫瓧鑺傛暟
}


int systemv_queue_delete(int g_msgid)
{
    if (g_msgid < 0) {
        fprintf(stderr, "Invalid queue id\n");
        return -1;
    }

    if (msgctl(g_msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
        return -1;
    }

    printf("Message queue (id=%d) deleted successfully\n", g_msgid);
    return 0;
}