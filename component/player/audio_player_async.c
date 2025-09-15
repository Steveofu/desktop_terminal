#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>        // POSIX �߳�
#include <unistd.h>        // POSIX ��׼������
#include <errno.h>         // �����붨��    
#include "systemv_queue.h" // System V ��Ϣ���нӿ�
#include "em_hal_audio.h"   // ��ƵӲ�������ӿڣ�����/ֹͣ��Ƶ��

#define audio_queue "/tmp/audio_queue"  // ��Ϣ�����ļ�·��

typedef enum {
    AUDIO_COMM_ID_START,  // ��ʼ������Ƶ
    AUDIO_COMM_ID_STOP,   // ֹͣ������Ƶ
} AUDIO_COMM_ID;


typedef struct {
    AUDIO_COMM_ID id;        // ���� ID��ָ��ִ�еĲ���
    char file_name[256];     // ��Ƶ�ļ���/·�������� START ����ʱ��Ч��
} audio_obj;

static pthread_t audio_thread;          // ��Ƶ�����߳̾��
static int audio_queue_id = -1;            // ��Ƶ��Ϣ���� ID


/**
 * @brief ��Ƶ�����߳�
 *
 * ���߳�ѭ���ȴ���Ϣ�����е������ִ�ж�Ӧ�Ĳ�����
 * - ֧���߳�ȡ����pthread_cancel��
 * - ��Ϣ��ʱ�ȴ� 100ms
 *
 * @param arg �̲߳���������δʹ��
 * @return void* ��������
 */
static void* audio_thread_fun(void *arg) {
    // �����߳̿ɱ�ȡ����֧�� pthread_cancel��
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // ����ȡ����ʽΪ���ӳ�ȡ�����������ڹؼ�����α�ǿ�ƴ�ϣ���ֹ��Դй¶
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) 
    {
        // 1. ��ʼ����Ϣ�������㣬����ʹ�������ݣ�
        audio_obj obj;
        memset(&obj, 0, sizeof(audio_obj));

        int n = systemv_queue_rcv(audio_queue_id, &obj, sizeof(obj),IPC_NOWAIT);
        if (n > 0) 
        {
            printf("Received cmd: %d\n", obj.id);

            // 3. �������� ID ִ�в���
            if (obj.id == AUDIO_COMM_ID_START) 
            {
                em_stop_play_audio();             // ֹͣ��ǰ����
                em_play_audio(obj.file_name);     // ����ָ���ļ�
            } else if (obj.id == AUDIO_COMM_ID_STOP) 
            {
                em_stop_play_audio();             // ֹͣ����
            }
        }

        // 4. �������� 1ms������ CPU ռ��
        usleep(1000); // 1ms
    }
}



void start_play_audio_async(char *url) {
    // 1. ������Ϣ����
    audio_obj obj = {.id = AUDIO_COMM_ID_START};
    strncpy(obj.file_name, url, sizeof(obj.file_name)-1); // �����ļ�·����ȷ����β�� '\0'

    // 2. ������Ϣ�����У���ʱʱ�� 1000ms��
    if (systemv_queue_send(audio_queue_id, &obj, sizeof(audio_obj)) != 0) {
        printf("Failed to send start cmd\n");
    }
}


/**
 * @brief ��ʼ���첽��Ƶ������
 *
 * ���ܣ�
 *   1. ������Ƶ��Ϣ���У����ڴ�Ŵ����ŵ����
 *   2. ������Ƶ�����̣߳��Ӷ�����ȡ�����ִ�С�
 *
 * @return 0 �ɹ�
 *        -1 ʧ��
 */

int init_async_audio_player(void)
{
    // ��ʼ�� System V ��Ϣ����
    audio_queue_id = systemv_queue_create(audio_queue, M_QUEUE_ID, 50, sizeof(audio_obj));
    if(audio_queue_id < 0)
    {
        printf("Init audio play failed\n");
        return -1;
    }

    if(pthread_create(&audio_thread, NULL, audio_thread_fun, NULL) != 0)
    {
        printf("Init audio play failed\n");
        // �����߳�ʧ�ܣ��ͷ��Ѵ�������Ϣ���У�������Դй©
        systemv_queue_delete(audio_queue_id);
        audio_queue_id = -1;

        return -1;
    }

     pthread_detach(audio_thread);

    return 0;
}


