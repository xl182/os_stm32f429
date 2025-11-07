/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    sd_diskio.c
 * @brief   SD Disk I/O driver
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Note: code generation based on sd_diskio_dma_rtos_template_bspv1.c v2.1.4
   as FreeRTOS is enabled. */

/* USER CODE BEGIN firstSection */
/* can be used to modify / undefine following code or add new definitions */
#define BLOCKSIZE SD_DEFAULT_BLOCK_SIZE
/* USER CODE END firstSection*/

/* Includes ------------------------------------------------------------------*/
#include "sd_diskio.h"
#include "ff_gen_drv.h"

#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SD_TIMEOUT (500 * 1000)
#define SD_DEFAULT_BLOCK_SIZE 512

/* USER CODE BEGIN disableSDInit */
/* #define DISABLE_SD_INIT */
/* USER CODE END disableSDInit */

/* USER CODE BEGIN enableSDDmaCacheMaintenance */
/* #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 */
/* USER CODE END enableSDDmaCacheMaintenance */

/* USER CODE BEGIN enableScratchBuffer */
/* #define ENABLE_SCRATCH_BUFFER */
/* USER CODE END enableScratchBuffer */

/* Private variables ---------------------------------------------------------*/
#if defined(ENABLE_SCRATCH_BUFFER)
#if defined(ENABLE_SD_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]);
#else
__ALIGN_BEGIN static uint8_t scratch[BLOCKSIZE] __ALIGN_END;
#endif
#endif

static volatile DSTATUS Stat = STA_NOINIT;

/* Private function prototypes -----------------------------------------------*/
static DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize(BYTE);
DSTATUS SD_status(BYTE);
DRESULT SD_read(BYTE, BYTE *, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT SD_write(BYTE, const BYTE *, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE, BYTE, void *);
#endif /* _USE_IOCTL == 1 */

/* FatFS 驱动注册 */
const Diskio_drvTypeDef SD_Driver = {
    SD_initialize, SD_status, SD_read,
#if _USE_WRITE == 1
    SD_write,
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
    SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  带超时的SD卡状态检查
 * @param  timeout：超时时间（ms）
 * @retval 0：成功（SD卡就绪），-1：超时（SD卡未就绪）
 */
static int SD_CheckStatusWithTimeout(uint32_t timeout) {
    uint32_t timer;
#if (osCMSIS <= 0x20000U)
    timer = osKernelSysTick();
    while ((osKernelSysTick() - timer) < timeout)
#else
    timer = osKernelGetTickCount();
    while ((osKernelGetTickCount() - timer) < timeout)
#endif
    {
        if (BSP_SD_GetCardState() == MSD_OK) {
            return 0;
        }
    }
    return -1;
}

/**
 * @brief  SD卡状态检查
 * @param  lun：逻辑单元号（未使用）
 * @retval DSTATUS：磁盘状态（STA_NOINIT/0）
 */
static DSTATUS SD_CheckStatus(BYTE lun) {
    Stat = STA_NOINIT;
    if (BSP_SD_GetCardState() == MSD_OK) {
        Stat &= ~STA_NOINIT;
    }
    return Stat;
}

/**
 * @brief  SD卡初始化（FatFS 调用）
 * @param  lun：逻辑单元号（未使用）
 * @retval DSTATUS：初始化状态
 * @修复点：移除复杂的消息队列/信号量机制
 */
DSTATUS SD_initialize(BYTE lun) {
    Stat = STA_NOINIT;

    /* 确保RTOS内核已启动 */
#if (osCMSIS <= 0x20000U)
    if (!osKernelRunning())
#else
    if (osKernelGetState() != osKernelRunning)
#endif
    {
        return Stat;
    }

    /* 初始化SD卡（按需禁用） */
#if !defined(DISABLE_SD_INIT)
    if (BSP_SD_Init() == MSD_OK) {
        Stat = SD_CheckStatus(lun);
    }
#else
    Stat = SD_CheckStatus(lun);
#endif

    return Stat;
}

/**
 * @brief  获取SD卡状态（FatFS 调用）
 * @param  lun：逻辑单元号（未使用）
 * @retval DSTATUS：磁盘状态
 */
DSTATUS SD_status(BYTE lun) { return SD_CheckStatus(lun); }

/**
 * @brief  SD卡读扇区（FatFS 调用）
 * @param  lun：逻辑单元号（未使用）
 * @param  buff：数据缓存地址
 * @param  sector：扇区地址（LBA）
 * @param  count：扇区数（1~128）
 * @retval DRESULT：读结果（RES_OK/RES_ERROR）
 * @修复点：简化实现，直接依赖SD卡状态检查
 */
extern uint8_t log_flag;
#include "sdio.h"
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) {
    uint8_t ret;
    DRESULT res = RES_ERROR;
    uint32_t timer;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
    uint32_t alignedAddr;
#endif

    if (log_flag) {
        printf("=== SD_read START ===\n");
        printf("sector: %lu, count: %u\n", sector, count);
        printf("SD card state before: %d\n", BSP_SD_GetCardState());
    }

    /* 检查SD卡就绪 */
    if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0) {
        if (log_flag) printf("SD_CheckStatusWithTimeout error\n");
        return res;
    }

#if defined(ENABLE_SCRATCH_BUFFER)
    /* 检查缓冲地址是否4字节对齐（DMA要求） */
    if (!((uint32_t)buff & 0x3)) {
#endif
        /* Fast Path：缓冲对齐，直接DMA读 */
        ret = BSP_SD_ReadBlocks_DMA((uint32_t *)buff, (uint32_t)sector, count);
        if (log_flag) printf("BSP_SD_ReadBlocks_DMA returned: %d\n", ret);
        
        if (ret == MSD_OK) {
            /* 简化：直接等待SD卡就绪，不依赖信号量/消息队列 */
            timer = osKernelGetTickCount();
            while ((osKernelGetTickCount() - timer) < SD_TIMEOUT) {
                if (BSP_SD_GetCardState() == MSD_OK) {
                    res = RES_OK;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
                    /* 缓存维护 */
                    alignedAddr = (uint32_t)buff & ~0x1F;
                    SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr,
                                                 count * SD_DEFAULT_BLOCK_SIZE +
                                                     ((uint32_t)buff - alignedAddr));
#endif
                    if (log_flag) printf("SD read completed successfully\n");
                    break;
                }
                /* 添加小延迟，避免过度占用CPU */
                osDelay(1);
            }
            
            if (res != RES_OK) {
                if (log_flag) printf("Timeout waiting for SD card ready after read\n");
            }
        } else {
            if (log_flag) printf("BSP_SD_ReadBlocks_DMA error: %d\n", ret);
        }

#if defined(ENABLE_SCRATCH_BUFFER)
    } else {
        /* Slow Path：缓冲未对齐，用scratch buffer中转（逐扇区读） */
        int i;
        for (i = 0; i < count; i++) {
            ret = BSP_SD_ReadBlocks_DMA((uint32_t *)scratch, (uint32_t)(sector + i), 1);
            if (ret != MSD_OK) break;

            /* 等待SDIO就绪 */
            timer = osKernelGetTickCount();
            while ((osKernelGetTickCount() - timer) < SD_TIMEOUT) {
                if (BSP_SD_GetCardState() == MSD_OK) break;
                osDelay(1);
            }
            if (BSP_SD_GetCardState() != MSD_OK) break;

#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
            /* 缓存失效（确保读取最新数据） */
            SCB_InvalidateDCache_by_Addr((uint32_t *)scratch, SD_DEFAULT_BLOCK_SIZE);
#endif

            /* 数据拷贝到目标缓冲 */
            memcpy(buff + i * SD_DEFAULT_BLOCK_SIZE, scratch, SD_DEFAULT_BLOCK_SIZE);
        }

        /* 所有扇区读取成功则返回OK */
        if (i == count) res = RES_OK;
    }
#endif

    if (log_flag) printf("=== SD_read END: result=%d ===\n\n", res);
    return res;
}

/**
 * @brief  SD卡写扇区（FatFS 调用）
 * @param  lun：逻辑单元号（未使用）
 * @param  buff：待写数据地址
 * @param  sector：扇区地址（LBA）
 * @param  count：扇区数（1~128）
 * @retval DRESULT：写结果（RES_OK/RES_ERROR）
 * @修复点：简化实现，直接依赖SD卡状态检查
 */
#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) {
    DRESULT res = RES_ERROR;
    uint32_t timer;
#if defined(ENABLE_SCRATCH_BUFFER)
    int32_t ret;
#endif

    if (log_flag) {
        printf("=== SD_write START ===\n");
        printf("sector: %lu, count: %u\n", sector, count);
    }

    /* 检查SD卡就绪 */
    if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0) {
        if (log_flag) printf("SD_CheckStatusWithTimeout error\n");
        return res;
    }

#if defined(ENABLE_SCRATCH_BUFFER)
    /* 检查缓冲地址是否4字节对齐 */
    if (!((uint32_t)buff & 0x3)) {
#endif
        /* Fast Path：缓冲对齐，直接DMA写 */
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
        /* 缓存清理 */
        uint32_t alignedAddr = (uint32_t)buff & ~0x1F;
        SCB_CleanDCache_by_Addr((uint32_t *)alignedAddr,
                                count * SD_DEFAULT_BLOCK_SIZE + ((uint32_t)buff - alignedAddr));
#endif

        if (BSP_SD_WriteBlocks_DMA((uint32_t *)buff, (uint32_t)sector, count) == MSD_OK) {
            /* 简化：直接等待SD卡就绪 */
            timer = osKernelGetTickCount();
            while ((osKernelGetTickCount() - timer) < SD_TIMEOUT) {
                if (BSP_SD_GetCardState() == MSD_OK) {
                    res = RES_OK;
                    break;
                }
                osDelay(1);
            }
        }

#if defined(ENABLE_SCRATCH_BUFFER)
    } else {
        /* Slow Path：缓冲未对齐，用scratch buffer中转（逐扇区写） */
        int i;
        for (i = 0; i < count; i++) {
            /* 1. 数据拷贝到scratch buffer */
            memcpy(scratch, buff + i * SD_DEFAULT_BLOCK_SIZE, SD_DEFAULT_BLOCK_SIZE);
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
            /* 缓存清理（确保数据写入内存） */
            SCB_CleanDCache_by_Addr((uint32_t *)scratch, SD_DEFAULT_BLOCK_SIZE);
#endif

            /* 2. DMA写scratch buffer到SD卡 */
            ret = BSP_SD_WriteBlocks_DMA((uint32_t *)scratch, (uint32_t)(sector + i), 1);
            if (ret != MSD_OK) break;

            /* 3. 等待SDIO就绪 */
            timer = osKernelGetTickCount();
            while ((osKernelGetTickCount() - timer) < SD_TIMEOUT) {
                if (BSP_SD_GetCardState() == MSD_OK) break;
                osDelay(1);
            }
            if (BSP_SD_GetCardState() != MSD_OK) break;
        }

        /* 所有扇区写入成功则返回OK */
        if (i == count) res = RES_OK;
    }
#endif

    if (log_flag) printf("=== SD_write END: result=%d ===\n\n", res);
    return res;
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  SD卡IO控制（FatFS 调用）
 * @param  lun：逻辑单元号（未使用）
 * @param  cmd：控制命令
 * @param  buff：参数缓冲
 * @retval DRESULT：控制结果
 */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff) {
    DRESULT res = RES_ERROR;
    BSP_SD_CardInfo CardInfo;

    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd) {
    /* 同步（确保无未完成写操作） */
    case CTRL_SYNC:
        res = RES_OK;
        break;

    /* 获取总扇区数 */
    case GET_SECTOR_COUNT:
        BSP_SD_GetCardInfo(&CardInfo);
        *(DWORD *)buff = CardInfo.LogBlockNbr;
        res            = RES_OK;
        break;

    /* 获取扇区大小（字节） */
    case GET_SECTOR_SIZE:
        BSP_SD_GetCardInfo(&CardInfo);
        *(WORD *)buff = CardInfo.LogBlockSize;
        res           = RES_OK;
        break;

    /* 获取擦除块大小（扇区数） */
    case GET_BLOCK_SIZE:
        BSP_SD_GetCardInfo(&CardInfo);
        *(DWORD *)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
        res            = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}
#endif /* _USE_IOCTL == 1 */

/**
 * @brief  SD卡写完成回调（BSP层调用）
 * @note   空实现，不再使用消息队列/信号量
 */
void BSP_SD_WriteCpltCallback(void) {
    /* 空实现，不再需要消息队列/信号量机制 */
}

/**
 * @brief  SD卡读完成回调（BSP层调用）
 * @note   空实现，不再使用消息队列/信号量
 */
void BSP_SD_ReadCpltCallback(void) {
    /* 空实现，不再需要消息队列/信号量机制 */
}

/* USER CODE BEGIN ErrorAbortCallbacks */
/* 按需启用SD卡错误/中止回调 */

/* USER CODE END ErrorAbortCallbacks */

/* USER CODE BEGIN lastSection */
/* 自定义扩展代码 */
/* USER CODE END lastSection */
