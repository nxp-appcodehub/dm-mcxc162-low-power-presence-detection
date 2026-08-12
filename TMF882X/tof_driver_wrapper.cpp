/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tof_driver_adapter.h>
#include <tof_driver_wrapper.h>

#include "mcu_tmf882x_config.h"
#include "inc/tmf882x_host_interface.h"

static inline bool fail(void *ctx, const char *msg)
{
    tof_err(ctx, msg);
    return false;
}

static inline bool switchMode(tmf882x_tof *tof, void *ctx, tmf882x_mode_t mode, const char *err)
{
    return tmf882x_mode_switch(tof, mode) ? fail(ctx, err) : true;
}

bool TMF882X::setupDevice(void)
{
    tmf882x_init(&_tof, this);
    if (_dbg) tmf882x_set_debug(&_tof, true);

    if (tmf882x_open(&_tof))
        return fail((void *)this, "ERROR - Unable to open the TMF882X");

    if (!uploadFirmware(tof_bin_image, tof_bin_image_length))
        if (!switchMode(&_tof, (void *)this, TMF882X_MODE_APP,
                        "ERROR - Fallback switch to APP mode failed"))
            return false;

    if (tmf882x_get_mode(&_tof) != TMF882X_MODE_APP)
        return fail((void *)this, "ERROR - The TMF882X failed to enter APP mode.");

    return true;
}

bool TMF882X::uploadFirmware(const unsigned char *img, unsigned long sz)
{
    if (!img || !sz) return false;

    if (!switchMode(&_tof, (void *)this, TMF882X_MODE_BOOTLOADER,
                    "ERROR - Switch to TMF882X Bootloader failed"))
        return false;

    if (tmf882x_fwdl(&_tof, FWDL_TYPE_BIN, img, sz))
        return fail((void *)this, "ERROR - Upload of firmware image failed");

    return true;
}

bool TMF882X::init(void)
{
    if (_ready) return true;
    if (!setupDevice()) return false;
    _ready = true;
    return true;
}

int TMF882X::measure(uint32_t count, uint32_t ms)
{
    if (!_ready) return -1;
    return runLoop(count, ms);
}

int TMF882X::measure(struct tmf882x_msg_meas_results &result, uint32_t ms)
{
    if (!_ready) return -1;
    if (!runLoop(1, ms)) return -1;
    if (!_lastResult) { memset(&result, 0, sizeof(result)); return -1; }
    memcpy(&result, _lastResult, sizeof(result));
    return 1;
}

void TMF882X::halt(void) { _halted = true; }

int TMF882X::runLoop(uint16_t count, uint32_t ms)
{
    if (!_ready) return -1;

    _halted = false; _lastResult = nullptr; _cnt = 0;

    if (!count && !(_cbMeas || _cbHist || _cbMsg || ms))
        return -1;

    if (tmf882x_start(&_tof)) return -1;

    uint32_t t0 = ms ? millis() : 0;

    do {
        if (tmf882x_process_irq(&_tof)) break;
        if (_halted) break;
        if (count && _cnt == count) break;
        if (ms && millis() - t0 >= ms) break;
        msleep(_pollMs);
    } while (true);

    tmf882x_stop(&_tof);
    return _cnt;
}

int32_t TMF882X::dispatchMsg(struct tmf882x_msg *msg)
{
    if (!msg || !_ready) return -1;

    if (_cbMsg) _cbMsg(msg);

    switch (msg->hdr.msg_id) {
    case ID_MEAS_RESULTS:
        _lastResult = &msg->meas_result_msg;
        _cnt++;
        if (_cbMeas) _cbMeas(_lastResult);
        break;
    case ID_HISTOGRAM:
        if (_cbHist) _cbHist(&msg->hist_msg);
        break;
    case ID_MEAS_STATS:
        if (_cbStats) _cbStats(&msg->meas_stat_msg);
        break;
    case ID_ERROR:
        if (_cbErr) _cbErr(&msg->err_msg);
        break;
    default: break;
    }
    return 0;
}
