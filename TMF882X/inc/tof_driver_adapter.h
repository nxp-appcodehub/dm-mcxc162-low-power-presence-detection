/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "tmf882x.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_edma.h"



#ifdef __cplusplus
extern "C" {
#endif


void tof_dbg(void* pTarget, const char* fmt, ...);

void tof_info(void* pTarget, const char* fmt, ...);

void tof_err(void* pTarget, const char* fmt, ...);

int32_t tof_i2c_read(void* pTarget, uint8_t reg, uint8_t* buf, int32_t len);

int32_t tof_i2c_write(void* pTarget, uint8_t reg, const uint8_t* buf, int32_t len);

int32_t tof_set_register(void* pTarget, uint8_t reg, uint8_t val);

int32_t tof_get_register(void* pTarget, uint8_t reg, uint8_t* val);

int32_t tof_queue_msg(void* pTarget, struct tmf882x_msg* msg);

void tof_usleep(void* pTarget, uint32_t usec);

void tof_get_timespec(struct timespec* ts);


// Utility routines needed for the underling sdk
unsigned long millis(void);
void usleep(uint32_t usec);
void msleep(uint32_t msec);
void output(const char* fmt, va_list args);

#ifdef __cplusplus
}
#endif
