
/*
  ******************************************************************************
  * File Name          : at24c02.h
  * Description        : this code is used for at24c02 application
  * Author             : JackWang
  * Date               : 2019-05-07
  ******************************************************************************
*/

#ifndef __AT24C02_H
#define __AT24C02_H

/*! -------------------------------------------------------------------------- */
/*! Include headers */
#include <stdint.h>
#include "setting.h"
/*! -------------------------------------------------------------------------- */

#if USE_AT24C02 == 1

/*! Public functions prototype */
int AT24C02_write(uint8_t addr, uint8_t *dataPtr, uint16_t dataSize);

int AT24C02_read(uint8_t addr, uint8_t *dataPtr, uint16_t dataSize);

int AT24C02_read_8bit(uint8_t addr, uint8_t *data);

int AT24C02_read_16bit(uint8_t addr, uint16_t *data);

int AT24C02_read_32bit(uint8_t addr, uint32_t *data);

int AT24C02_write_8bit(uint8_t addr, uint8_t data);

int AT24C02_write_16bit(uint8_t addr, uint16_t data);

int AT24C02_write_32bit(uint8_t addr, uint32_t data);

#endif

#endif
/*! end of the file */

