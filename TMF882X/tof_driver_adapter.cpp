/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tof_driver_adapter.h>

#include <tof_driver_wrapper.h>


extern edma_handle_t g_edmaTxHandle;
extern edma_handle_t g_edmaRxHandle;
extern volatile bool g_MasterCompletionFlag;
extern lpi2c_master_edma_handle_t g_m_edma_handle;


unsigned long millis(void)
{
    return 0;
}



void msleep(uint32_t msec)
{
    SDK_DelayAtLeastUs(msec*1000, SystemCoreClock);
}



void usleep(uint32_t usec)
{
  SDK_DelayAtLeastUs(usec, SystemCoreClock);
}







//#include "fsl_debug_console.h"
void output(const char* fmt, va_list args)
{
//  DbgConsole_Vprintf(fmt, args);
//  PRINTF("\r\n");

}

int tof_writeRegisterRegion(uint8_t i2c_address, uint8_t offset, uint8_t *data, uint16_t length)
{
    status_t reVal = kStatus_Fail;
    lpi2c_master_transfer_t masterXfer = {0};
          /* subAddress = 0x01, data = g_master_txBuff - write to slave.
      start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
    masterXfer.slaveAddress   = i2c_address;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = (uint32_t)offset;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = data;
    masterXfer.dataSize       = length;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master non-blocking data to slave */
    reVal = LPI2C_MasterTransferEDMA(LPI2C0, &g_m_edma_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return -1;
    }

    /*  Wait for transfer completed. */
    while (!g_MasterCompletionFlag)
    {
    }
    g_MasterCompletionFlag = false;

    return 0;

}


int tof_readRegisterRegion(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t numBytes)
{
    status_t reVal = kStatus_Fail;
    lpi2c_master_transfer_t masterXfer = {0};
    /* subAddress = 0x01, data = g_master_rxBuff - read from slave.
      start + slaveaddress(w) + subAddress + repeated start + slaveaddress(r) + rx data buffer + stop */


    masterXfer.slaveAddress   = addr;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = (uint32_t)reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = data;
    masterXfer.dataSize       = numBytes;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master non-blocking data to slave */
    reVal = LPI2C_MasterTransferEDMA(LPI2C0, &g_m_edma_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return -1;
    }

    /*  Wait for transfer completed. */
    while (!g_MasterCompletionFlag)
    {
    }
    g_MasterCompletionFlag = false;
    return 0;
}


void tof_info(void* pTarget, const char* fmt, ...)
{
    // Is our library outputting info messages?
    if (!fmt || !(((TMF882X*)pTarget)->getLogLevel() & TMF882X_MSG_INFO))
        return;

    // Grab our args, send to our generic output routine
    va_list ap;
    va_start(ap, fmt);
    output(fmt, ap);
    va_end(ap);
}


void tof_dbg(void* pTarget, const char* fmt, ...)
{
    // Is our library outputting debug messages?    
    if (!fmt || !(((TMF882X*)pTarget)->getLogLevel() & TMF882X_MSG_DEBUG))
        return;

    // Grab our args, send to our generic output routine
    va_list ap;
    va_start(ap, fmt);
    output(fmt, ap);
    va_end(ap);
}



void tof_err(void* pTarget, const char* fmt, ...)
{
    // Is our library outputting error messages?        
    if (!fmt || !(((TMF882X*)pTarget)->getLogLevel() & TMF882X_MSG_ERROR))
        return;

    // Grab our args, send to our generic output routine
    va_list ap;
    va_start(ap, fmt);
    output(fmt, ap);
    va_end(ap);
}



int32_t tof_i2c_read(void* pTarget, uint8_t reg, uint8_t* buf, int32_t len)
{
    // Just relay up to our library object

    return tof_readRegisterRegion(0x41, reg, buf, len);
}



int32_t tof_i2c_write(void* pTarget, uint8_t reg, const uint8_t* buf, int32_t len)
{
    // Just relay up to our library object

    return tof_writeRegisterRegion(0x41, reg, (uint8_t*)buf, len);
}



int32_t tof_set_register(void* pTarget, uint8_t reg, uint8_t val)
{
    // use our generic routine in this interface

    return tof_i2c_write(pTarget, reg, &val, 1);
}



int32_t tof_get_register(void* pTarget, uint8_t reg, uint8_t* val)
{
    // use our generic routine in this interface    
    return tof_i2c_read(pTarget, reg, val, 1);
}



int32_t tof_queue_msg(void* pTarget, struct tmf882x_msg* msg)
{
    // relay up to our main library object

    return ((TMF882X*)pTarget)->dispatchMsg(msg);
}



void tof_usleep(void* pTarget, uint32_t usec)
{
    usleep(usec);
}


void tof_get_timespec(struct timespec* ts)
{
    ts->tv_sec = millis();
    ts->tv_nsec = 0;
}





