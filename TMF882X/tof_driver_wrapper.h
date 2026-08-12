/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "inc/tmf882x.h"
#include "tmf882x_interface.h"

#define kDefaultTMF882XAddress 0x41
#define kDefaultSampleDelayMS  50

#define TMF882X_MSG_INFO  0x01
#define TMF882X_MSG_DEBUG 0x02
#define TMF882X_MSG_ERROR 0x04
#define TMF882X_MSG_ALL   0x07
#define TMF882X_MSG_NONE  0x00

typedef void (*TMF882XMeasurementHandler)(struct tmf882x_msg_meas_results *);
typedef void (*TMF882XHistogramHandler)(struct tmf882x_msg_histogram *);
typedef void (*TMF882XStatsHandler)(struct tmf882x_msg_meas_stats *);
typedef void (*TMF882XErrorHandler)(struct tmf882x_msg_error *);
typedef void (*TMF882XMessageHandler)(struct tmf882x_msg *);

class TMF882X
{
public:
    TMF882X()
        : _ready{false}, _pollMs{kDefaultSampleDelayMS},
          _logMask{TMF882X_MSG_NONE}, _dbg{false},
          _cbMeas{nullptr}, _cbHist{nullptr},
          _cbStats{nullptr}, _cbErr{nullptr},
          _cbMsg{nullptr}, _addr{0} {}

    bool init();
    bool uploadFirmware(const unsigned char *img, unsigned long sz);

    void onMessage(TMF882XMessageHandler fn)     { _cbMsg   = fn; }
    void onMeasurement(TMF882XMeasurementHandler fn) { _cbMeas = fn; }
    void onHistogram(TMF882XHistogramHandler fn)  { _cbHist  = fn; }
    void onStats(TMF882XStatsHandler fn)          { _cbStats = fn; }
    void onError(TMF882XErrorHandler fn)          { _cbErr   = fn; }

    int  measure(uint32_t count = 0, uint32_t ms = 0);
    int  measure(struct tmf882x_msg_meas_results &result, uint32_t ms = 0);
    void halt(void);

    void     setPollInterval(uint16_t ms) { if (ms) _pollMs = ms; }
    uint16_t getPollInterval(void)        { return _pollMs; }

    tmf882x_tof &getContext(void) { return _tof; }

    void enableDebug(bool on)
    {
        _dbg = on;
        if (on) _logMask |= TMF882X_MSG_DEBUG;
        else    _logMask &= ~TMF882X_MSG_DEBUG;
    }
    bool isDebugOn(void) { return _dbg; }

    void enableInfoMsg(bool on)
    {
        if (on) _logMask |= TMF882X_MSG_INFO;
        else    _logMask &= ~TMF882X_MSG_INFO;
    }

    void    setLogLevel(uint8_t lvl) { _logMask = lvl; }
    uint8_t getLogLevel(void)        { return _logMask; }

    int32_t dispatchMsg(struct tmf882x_msg *msg);

private:
    bool setupDevice(void);
    int  runLoop(uint16_t count, uint32_t ms);

    bool     _ready;
    uint16_t _pollMs;
    uint8_t  _logMask;
    bool     _dbg;

    TMF882XMeasurementHandler _cbMeas;
    TMF882XHistogramHandler   _cbHist;
    TMF882XStatsHandler       _cbStats;
    TMF882XErrorHandler       _cbErr;
    TMF882XMessageHandler     _cbMsg;

    uint8_t  _addr;
    tmf882x_tof _tof;
    uint16_t _cnt;
    struct tmf882x_msg_meas_results *_lastResult;
    bool _halted;
};
