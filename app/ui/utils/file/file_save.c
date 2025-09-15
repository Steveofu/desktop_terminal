
#include "file_save.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PARAM_SAVE_PATH PROJECT_RES_URL
#define PARAM_SAVE_PATH_MAX_LEN 300

// 从文件中读取参数数据
// name: 参数文件名（不带路径）
// data: 数据存放地址
// len: 数据长度
int file_param_read(const char *name, void *data, int len) {
    int fd = 0;
    int seek_offset = 0;                          // 文件偏移量，默认从文件开头开始
    char file_name[PARAM_SAVE_PATH_MAX_LEN];      // 文件完整路径
    char file_name_copy[PARAM_SAVE_PATH_MAX_LEN]; // 临时文件路径

    // 清空路径缓冲区
    memset(file_name, 0, sizeof(file_name));
    memset(file_name_copy, 0, sizeof(file_name_copy));

    // 拼接文件路径
    sprintf(file_name, "%s%s", PARAM_SAVE_PATH, name);          // 正式参数文件
    sprintf(file_name_copy, "%s%s_copy", PARAM_SAVE_PATH, name); // 临时备份文件

    // 如果正式文件不存在
    if (access(file_name, F_OK) != 0) {
        // 如果临时文件存在，则恢复正式文件
        if (access(file_name_copy, F_OK) == 0) {
            rename(file_name_copy, file_name); // 临时文件改名为正式文件
            unlink(file_name_copy);            // 删除临时文件
        } else {
            printf("open file error\n");      // 正式文件和临时文件都不存在
            return -1;
        }
    }

    // 打开文件，只读方式
    fd = open(file_name, O_RDONLY, 0666);
    if (fd < 0) {
        printf("open file error\n");
        return -1;
    }

    // 文件偏移到开头
    if (lseek(fd, seek_offset, SEEK_SET) == -1) {
        return -1;
    }

    // 读取数据
    if (read(fd, data, len) <= 0) {
        return -1;
    }

    return 0; // 读取成功
}


// 向文件中写入参数数据（带临时文件保护机制）
// name: 参数文件名（不带路径）
// data: 待写入数据地址
// len: 数据长度
int file_param_write(const char *name, void *data, int len) {
    int fd = 0;
    int seek_offset = 0; // 文件偏移量，默认从文件开头
    char file_name[PARAM_SAVE_PATH_MAX_LEN];      // 文件完整路径
    char file_name_copy[PARAM_SAVE_PATH_MAX_LEN]; // 临时文件路径

    // 清空路径缓冲区
    memset(file_name, 0, sizeof(file_name));
    memset(file_name_copy, 0, sizeof(file_name_copy));

    // 拼接文件路径
    sprintf(file_name, "%s%s", PARAM_SAVE_PATH, name);           // 正式参数文件
    sprintf(file_name_copy, "%s%s_copy", PARAM_SAVE_PATH, name); // 临时文件

    printf("path = %s\n", file_name);

    // 打开临时文件，写入模式，文件不存在则创建，存在则清空
    fd = open(file_name_copy, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("open file error\n");
        return -1;
    }

    // 文件偏移到开头
    if (lseek(fd, seek_offset, SEEK_SET) == -1) {
        fsync(fd); // 将数据写入磁盘
        close(fd);
        return -1;
    }

    // 写入数据
    if (write(fd, data, len) != len) {
        fsync(fd);
        close(fd);
        return -1;
    }

    // 写入成功后，将临时文件改名为正式文件，保证原子性
    rename(file_name_copy, file_name);

    // 删除临时文件（如果还存在的话）
    unlink(file_name_copy);

    // 强制将内存缓冲区写入磁盘
    sync();

    return 0; // 写入成功
}


typedef struct 
{
    char name[10];
    int data1;
    char data2;
}file_param_test_data_t;

void file_param_test(){
    file_param_test_data_t wdata;
    file_param_test_data_t rdata;

    int ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

    memcpy(wdata.name,"xiaozhi",strlen("xiaozhi"));
    wdata.data1 = 10;
    wdata.data2 = 20;
    file_param_write("param.cfg",&wdata,sizeof(wdata));
    ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

    memcpy(wdata.name,"xiaohei",strlen("xiaohei"));
    wdata.data1 = 40;
    wdata.data2 = 10;
    file_param_write("param.cfg",&wdata,sizeof(wdata));
    ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

}