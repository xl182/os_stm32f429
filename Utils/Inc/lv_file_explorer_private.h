/**
 * @file lv_file_explorer_private.h
 *
 */

#ifndef LV_FILE_EXPLORER_PRIVATE_H
#define LV_FILE_EXPLORER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * INCLUDES
 */
#include "lv_file_explorer.h"
#if USE_LVGL
#include "lv_obj_private.h"

#include "stdio.h"
/*
 * DEFINES
 */

/**
 * TYPEDEFS
 **/

/*Data of canvas*/
struct lv_file_explorer_t {
    lv_obj_t obj;
    lv_obj_t * cont;
    lv_obj_t * head_area;
    lv_obj_t * browser_area;
    lv_obj_t * file_table;
    lv_obj_t * path_label;
#if LV_FILE_EXPLORER_QUICK_ACCESS
    lv_obj_t * quick_access_area;
    lv_obj_t * list_device;
    lv_obj_t * list_places;
    char * home_dir;
    char * music_dir;
    char * pictures_dir;
    char * video_dir;
    char * docs_dir;
    char * fs_dir;
#endif
    const char * sel_fn;
    char   current_path[LV_FILE_EXPLORER_PATH_MAX_LEN];
    lv_file_explorer_sort_t sort;
    
    // 新增成员
    char menu_file_path[LV_FILE_EXPLORER_PATH_MAX_LEN];
    lv_obj_t *context_menu;
    lv_obj_t *file_grid;
    char sel_path[LV_FILE_EXPLORER_PATH_MAX_LEN];
    lv_obj_t * ctrl_area; 
    lv_obj_t * btn_del;        
    lv_obj_t * btn_copy;       
    lv_obj_t * btn_cut;        
    lv_obj_t * btn_paste;      
    lv_obj_t * btn_new;        
    lv_obj_t * btn_rename;     
    lv_obj_t * btn_send;       
    lv_obj_t * btn_open;       
    lv_obj_t * btn_path;       
};

typedef enum {
    FE_MENU_CUT,
    FE_MENU_COPY,
    FE_MENU_DELETE,
    FE_MENU_RENAME,
    FE_MENU_SEND,
    FE_MENU_COPY_PATH
} fe_menu_action_t;

/**
 * GLOBAL PROTOTYPES
 **/
static void browser_file_event_handler(lv_event_t *e);
static void menu_event_handler(lv_event_t *e);
/**
 * MACROS
 **/

#endif /* LV_USE_FILE_EXPLORER != 0 */
typedef struct lv_file_explorer_t lv_file_explorer_t;
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif
