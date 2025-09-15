#include <stdlib.h>
#include <stdio.h>

#include "lv_port_disp.h"
#include "lvgl.h"
#include "sunxifb.h"
#include "port_conf.h"


static lv_color_t *draw_buf;
static lv_color_t *draw_buf_1;

// 初始化显示屏，并注册到 LVGL
// is_disp_orientation: 是否旋转屏幕（true -> 270度，false -> 90度）
void lv_port_disp_init(bool is_disp_orientation)
{
    /*-------------------------
     * 初始化显示方向
     * -----------------------*/
	uint32_t rotated = DISP_ORIENTATION;  // 默认显示方向

    // 根据传入参数设置旋转角度
    if(is_disp_orientation == true){
        rotated = LV_DISP_ROT_270;   // 旋转 270 度
    } else {
        rotated = LV_DISP_ROT_90;    // 旋转 90 度
    }
	 
    // 初始化 Sunxi framebuffer，设置旋转角度
    sunxifb_init(rotated);

    /*-------------------------
     * 获取屏幕尺寸
     * -----------------------*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height); // 从硬件获取宽高
    width = 280;                         // 固定宽度为 280（覆盖硬件获取值？）

    // 计算绘图缓冲区大小
	int draw_buf_size = width * height * sizeof(lv_color_t);
	
    // 分配绘图缓冲区 draw_buf
    draw_buf = (lv_color_t*) sunxifb_alloc(draw_buf_size, "lv_examples");
    if (draw_buf == NULL) 
	{
        sunxifb_exit();               // 分配失败，退出 framebuffer
        LV_LOG_ERROR("sunxifb_alloc error");
        return;
    }

    // 分配第二个缓冲区 draw_buf_1，用于双缓冲显示
    draw_buf_1 = (lv_color_t*) sunxifb_alloc(draw_buf_size, "lv_examples_1");
    if (draw_buf_1 == NULL) 
	{
        sunxifb_exit();               // 分配失败，退出 framebuffer
        LV_LOG_ERROR("sunxifb_alloc error");
        return;
    }
	
    /*-----------------------------
     * 创建 LVGL 绘图缓冲区描述符
     *----------------------------*/
    static lv_disp_draw_buf_t draw_buf_dsc;                         
    lv_disp_draw_buf_init(&draw_buf_dsc, draw_buf, draw_buf_1, width * height);   
    // draw_buf: 第一个缓冲区
    // draw_buf_1: 第二个缓冲区（可为NULL，如果使用单缓冲）
    // width * height: 缓冲区像素数量

    /*-----------------------------------
     * 初始化并注册 LVGL 显示驱动
     *----------------------------------*/
    static lv_disp_drv_t disp_drv;                     
    lv_disp_drv_init(&disp_drv);      // 初始化显示驱动结构体
    disp_drv.draw_buf = &draw_buf_dsc; // 设置绘图缓冲区
    disp_drv.flush_cb = sunxifb_flush; // 设置刷新回调函数
    disp_drv.hor_res = width;          // 水平分辨率
    disp_drv.ver_res = height;         // 垂直分辨率
    disp_drv.rotated = rotated;        // 显示旋转角度

    // 注册驱动到 LVGL
    lv_disp_drv_register(&disp_drv);
}


// 反初始化显示屏，释放绘图缓冲区
void lv_port_disp_deinit(void)
{
    sunxifb_free((void **)&draw_buf,"lv_examples");      // 释放第一个缓冲区
    sunxifb_free((void **)&draw_buf_1,"lv_examples_1"); // 释放第二个缓冲区
}
