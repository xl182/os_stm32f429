#ifndef __AT24C02_H__
#define __AT24C02_H__

#include "setting.h"
#if USE_AT24C02 == 1

#include "at24c02.h"
#include "i2c.h"

/*! -------------------------------------------------------------------------- */
/*! Private macros define */
#define AT24CXX_Write_ADDR 0xA0
#define AT24CXX_Read_ADDR  0xA1
#define AT24CXX_MAX_SIZE   256
#define AT24CXX_PAGE_SIZE  8
#define AT24CXX_PAGE_TOTAL (AT24CXX_MAX_SIZE/AT24CXX_PAGE_SIZE)

/*! -------------------------------------------------------------------------- */
/*! Public functions list */
/*! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int
AT24C02_write(uint8_t addr, uint8_t *dataPtr, uint16_t dataSize) {
    if (0 == dataSize) {
        return -1;
    }

    int res;
    int selectPage_idx = addr % AT24CXX_PAGE_SIZE;
    int selectPage_rest = AT24CXX_PAGE_SIZE - selectPage_idx;

    if (dataSize <= selectPage_rest) {
        res = HAL_I2C_Mem_Write(&hi2c1,
                                AT24CXX_Write_ADDR,
                                addr,
                                I2C_MEMADD_SIZE_8BIT,
                                dataPtr,
                                dataSize,
                                0xFF);

        if (HAL_OK != res) { return -1; }

        HAL_Delay(10);

    } else {

        /*! 1 write selectPage rest*/
        res = HAL_I2C_Mem_Write(&hi2c1,
                                AT24CXX_Write_ADDR,
                                addr,
                                I2C_MEMADD_SIZE_8BIT,
                                dataPtr,
                                selectPage_rest,
                                0xFF);

        if (HAL_OK != res) {
            return -1;
        }

        addr += selectPage_rest;
        dataSize -= selectPage_rest;
        dataPtr += selectPage_rest;

        HAL_Delay(5);

        /*! 2 write nextPage full */
        int fullPage = dataSize / AT24CXX_PAGE_SIZE;
        for (int iPage = 0; iPage < fullPage; ++iPage) {
            res = HAL_I2C_Mem_Write(&hi2c1,
                                    AT24CXX_Write_ADDR,
                                    addr,
                                    I2C_MEMADD_SIZE_8BIT,
                                    dataPtr,
                                    AT24CXX_PAGE_SIZE,
                                    0xFF);

            if (HAL_OK != res) { return -1; }

            HAL_Delay(5);

            addr += AT24CXX_PAGE_SIZE;
            dataSize -= AT24CXX_PAGE_SIZE;
            dataPtr += AT24CXX_PAGE_SIZE;
        }

        /*! 3 write rest */
        if (0 != dataSize) {
            res = HAL_I2C_Mem_Write(&hi2c1,
                                    AT24CXX_Write_ADDR,
                                    addr,
                                    I2C_MEMADD_SIZE_8BIT,
                                    dataPtr,
                                    dataSize,
                                    0xFF);

            if (HAL_OK != res) { return -1; }

            HAL_Delay(5);
        }
    }

    return 0;
}

/*! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int
AT24C02_read(uint8_t addr, uint8_t *dataPtr, uint16_t dataSize) {
    int res = HAL_I2C_Mem_Read(&hi2c1,
                               AT24CXX_Read_ADDR,
                               addr,
                               I2C_MEMADD_SIZE_8BIT,
                               dataPtr,
                               dataSize,
                               0xFF);

    if (HAL_OK != res) { return -1; }

    return 0;
}

int AT24C02_write_8bit(uint8_t addr, uint8_t data) {
    AT24C02_write(addr, &data, 1);
    return 0;
}

int AT24C02_write_32bit(uint8_t addr, uint32_t data) {
    uint8_t t;
    uint8_t tmp;
    for (t = 0; t < 4; t++) {
        tmp = (data >> (8 * t)) & 0xff;
        AT24C02_write(addr + t, &tmp, 1);
    }
    return 0;
}

int AT24C02_write_16bit(uint8_t addr, uint16_t data) {
    uint8_t t;
    uint8_t tmp;
    for (t = 0; t < 2; t++) {
        tmp = (data >> (8 * t)) & 0xff;
        AT24C02_write(addr + t, &tmp, 1);
    }
    return 0;
}

int AT24C02_read_32bit(uint8_t addr, uint32_t *data) {
    *data = 0;
    uint8_t t;
    uint8_t tmp;
    for (t = 0; t < 4; t++) {
        AT24C02_read(addr + t, &tmp, 1);
        *data += tmp << (8 * t);
    }
    return 0;
}

int AT24C02_read_16bit(uint8_t addr, uint16_t *data) {
    *data = 0;
    uint8_t t;
    uint8_t tmp;
    for (t = 0; t < 2; t++) {
        AT24C02_read(addr + t, &tmp, 1);
        *data += tmp << (8 * t);
    }
    return 0;
}

int AT24C02_read_8bit(uint8_t addr, uint8_t *data) {
    *data = 0;
    AT24C02_read(addr, data, 1);
    return 0;
}

#endif
#endif
