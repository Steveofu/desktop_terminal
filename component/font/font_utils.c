/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:32:07 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:32:07 
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "font_utils.h"

typedef struct
{
    int type;
    char font_url[250];
}font_type_t;

typedef struct
{
	int type;
	uint16_t size;    
	lv_ft_info_t* ft_info;
}font_obj_t;

static bool is_init = false;
static lv_ll_t font_type_list;
static lv_ll_t font_obj_list;

static font_type_t* find_font_type(int type){
    font_type_t* font_type = _lv_ll_get_head(&font_type_list);
    while (font_type)
    {
        if(font_type->type == type){
            return font_type;
        }
        font_type = _lv_ll_get_next(&font_type_list, font_type);
    }
    return NULL;
}

static lv_font_t* create_font_obj(int type, uint16_t size)
{
    font_type_t* font_type = find_font_type(type);
    if(font_type == NULL){
        return NULL;
    }
    lv_ft_info_t* ft_info = malloc(sizeof(lv_ft_info_t));
    ft_info->name = font_type->font_url;
    ft_info->weight = size;
    ft_info->style = FT_FONT_STYLE_NORMAL;
    ft_info->mem = NULL;
    ft_info->mem_size = 0;
    lv_ft_font_init(ft_info);

    font_obj_t* font_obj = _lv_ll_ins_tail(&font_obj_list);
    font_obj->type = type;
    font_obj->size = size;
    font_obj->ft_info = ft_info;

    return font_obj->ft_info->font;
}

/**
 * @brief 向字体链表中添加新的字体类型及其路径
 *
 * @param type      字体类型（例如 0 表示宋体，1 表示黑体，具体取决于上层定义）
 * @param font_url  字体文件路径字符串
 */
void add_font(int type ,char* font_url) {
    // 判断是否已经初始化过字体链表
    if (!is_init) {
        is_init = true;

        // 初始化字体对象链表 (用于存储具体的字体对象)
        _lv_ll_init(&font_obj_list, sizeof(font_obj_t));

        // 初始化字体类型链表 (用于存储字体类型及路径)
        _lv_ll_init(&font_type_list, sizeof(font_type_t));
    }

    // 在字体类型链表的尾部插入一个新节点
    font_type_t* font_type = _lv_ll_ins_tail(&font_type_list);

    // 设置该节点的字体类型
    font_type->type = type;

    // 将传入的字体路径字符串复制到节点的 font_url 成员中
    strcpy(font_type->font_url, font_url);
}


lv_font_t* get_font(int type, uint16_t size)
{
    font_obj_t* font_obj = _lv_ll_get_head(&font_obj_list);
    while (font_obj)
    {
        if (font_obj->type == type && font_obj->size == size)
        {
            return font_obj->ft_info->font;
        }
        font_obj = _lv_ll_get_next(&font_obj_list, font_obj);
    }
    return create_font_obj(type, size);
}
