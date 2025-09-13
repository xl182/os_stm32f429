#include "lv_port_fs.h"
#include "ff.h"
#include "lvgl.h"
#include <stdlib.h> // 添加标准库头文件
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#define ROOT_PATH "0:/"
#define MAX_PATH_LEN 256

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);
static char *convert_path(const char *path);
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw,
                            uint32_t *bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);
static void *fs_dir_open(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn, uint32_t fn_len);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p);
static lv_fs_res_t fs_remove(lv_fs_drv_t *drv, const char *path);

/**********************
 *  STATIC VARIABLES
 **********************/
static FATFS fs;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_fs_init(void) {
    MX_SDIO_SD_Init();
    MX_FATFS_Init();
    fs_init();

    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter     = '0';
    fs_drv.cache_size = 256;

    // 文件操作回调
    fs_drv.open_cb   = fs_open;
    fs_drv.close_cb  = fs_close;
    fs_drv.read_cb   = fs_read;
    fs_drv.write_cb  = fs_write;
    fs_drv.seek_cb   = fs_seek;
    fs_drv.tell_cb   = fs_tell;
    fs_drv.remove_cb = fs_remove;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb  = fs_dir_open;
    fs_drv.dir_read_cb  = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void fs_init(void) {
    FRESULT res = f_mount(&fs, "0:", 1);
    if (res != FR_OK) {
        LV_LOG_ERROR("SD card mount failed! Error: %d\r\n", res);
    } else {
        printf("SD card mounted successfully\r\n");
    }
}

static char *convert_path(const char *path) {
    static char new_path[MAX_PATH_LEN];
    const char *relative_path = strchr(path, ':');

    if (relative_path) {
        relative_path++;
        if (*relative_path == '/') relative_path++;
        snprintf(new_path, MAX_PATH_LEN, "%s/%s", ROOT_PATH, relative_path);
    } else {
        snprintf(new_path, MAX_PATH_LEN, "%s/%s", ROOT_PATH, path);
    }
    return new_path;
}
#include "string.h"
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
    LV_UNUSED(drv);

    char actual_path[LV_FS_MAX_PATH_LENGTH];
    snprintf(actual_path, sizeof(actual_path), "0:/%s", path + 1);
    BYTE fatfs_mode = 0;
    if (mode & LV_FS_MODE_RD) fatfs_mode |= FA_READ;
    if (mode & LV_FS_MODE_WR) {
        fatfs_mode |= FA_WRITE;
        fatfs_mode |= FA_OPEN_ALWAYS; 
    }

    FIL *file = malloc(sizeof(FIL));
    if (file == NULL) {
        LV_LOG_ERROR("FIL alloc failed");
        return (void *)(-1);
    }

    FRESULT res = f_open(file, actual_path, fatfs_mode);
    if (res != FR_OK) {
        LV_LOG_ERROR("f_open(%s) failed: %d", actual_path, res);
        free(file);
        return (void *)(-1);
    }

    return file;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p) {
    LV_UNUSED(drv);
    FIL *file = (FIL *)file_p;

    FRESULT res = f_close(file);
    free(file); // 使用标准库 free

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) {
    LV_UNUSED(drv);
    FIL *file = (FIL *)file_p;

    UINT bytes_read;
    FRESULT res = f_read(file, buf, btr, &bytes_read);

    if (br) *br = bytes_read;

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw,
                            uint32_t *bw) {
    LV_UNUSED(drv);
    FIL *file = (FIL *)file_p;

    UINT bytes_written;
    FRESULT res = f_write(file, buf, btw, &bytes_written);

    if (bw) *bw = bytes_written;

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) {
    LV_UNUSED(drv);
    FIL *file = (FIL *)file_p;

    FRESULT res;
    switch (whence) {
    case LV_FS_SEEK_SET:
        res = f_lseek(file, pos);
        break;
    case LV_FS_SEEK_CUR: {
        FSIZE_t cur = f_tell(file);
        res         = f_lseek(file, cur + pos);
        break;
    }
    case LV_FS_SEEK_END: {
        FSIZE_t size = f_size(file);
        res          = f_lseek(file, size - pos);
        break;
    }
    default:
        return LV_FS_RES_NOT_IMP;
    }

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
    LV_UNUSED(drv);
    FIL *file = (FIL *)file_p;

    *pos_p = (uint32_t)f_tell(file);
    return LV_FS_RES_OK;
}

static void *fs_dir_open(lv_fs_drv_t *drv, const char *path) {
    LV_UNUSED(drv);

    DIR *dir = malloc(sizeof(DIR));
    if (dir == NULL) {
        LV_LOG_ERROR("malloc for DIR failed\r\n");
        return NULL;
    }

    char *actual_path = convert_path(path);
    FRESULT res       = f_opendir(dir, actual_path);

    if (res != FR_OK) {
        LV_LOG_ERROR("f_opendir(%s) failed: %d\r\n", actual_path, res);
        free(dir);
        return NULL;
    }

    return dir;
}

static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn, uint32_t fn_len) {
    LV_UNUSED(drv);
    DIR *dir = (DIR *)rddir_p;

    FILINFO fno;
    FRESULT res = f_readdir(dir, &fno);

    if (res != FR_OK || fno.fname[0] == 0) {
        return LV_FS_RES_UNKNOWN;
    }

    // 过滤系统文件
    if (fno.fname[0] == '.') {
        return fs_dir_read(drv, rddir_p, fn, fn_len);
    }

    // 安全复制文件名
    strncpy(fn, fno.fname, fn_len - 1);
    fn[fn_len - 1] = '\0';

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p) {
    LV_UNUSED(drv);
    DIR *dir = (DIR *)rddir_p;

    FRESULT res = f_closedir(dir);
    free(dir); // 使用标准库 free

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}

lv_fs_res_t fs_remove(lv_fs_drv_t *drv, const char *path) {
    LV_UNUSED(drv);

    char *actual_path = convert_path(path);
    FRESULT res       = f_unlink(actual_path);

    return (res == FR_OK) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
}