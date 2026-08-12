/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"
#include "board.h"
#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_wuu.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_lptmr.h"
#include "power_mode_switch.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
__WEAK void APP_PowerPreSwitchHook(app_power_mode_t targetPowerMode);
__WEAK void APP_PowerPostSwitchHook(void);

static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode);

/*******************************************************************************
 * Variables
 ******************************************************************************/
char *const g_modeNameArray[] = APP_POWER_MODE_NAME;
char *const g_modeDescArray[] = APP_POWER_MODE_DESC;


/*******************************************************************************
 * Code
 ******************************************************************************/
   
/*! @brief WakeUp Timer configuration. */
static void APP_WakeUpTimerConfig(uint8_t timeOutValue)
{
    lptmr_config_t lptmr_config;
    LPTMR_GetDefaultConfig(&lptmr_config);
    lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
    LPTMR_Init(LPTMR0, &lptmr_config);

    LPTMR_ClearStatusFlags(APP_WUU_WAKEUP_TIMER, kLPTMR_TimerCompareFlag);
    IRQ_ClearPendingIRQ(APP_WUU_WAKEUP_TIMER_IRQN);
    LPTMR_SetTimerPeriod(APP_WUU_WAKEUP_TIMER, (APP_WUU_WAKEUP_TIMER_CLOCK_SOURCE * timeOutValue) - 1U);
    LPTMR_EnableInterrupts(APP_WUU_WAKEUP_TIMER, kLPTMR_TimerInterruptEnable);
    EnableIRQ(APP_WUU_WAKEUP_TIMER_IRQN);

    LPTMR_StartTimer(APP_WUU_WAKEUP_TIMER);
}



void low_power_enter(void)
{
    if (kCMC_CoreClockGated == CMC_GetCoreClockGatedStatus(APP_CMC))
    {
          CMC_ClearCoreClockGatedStatus(APP_CMC);
    }
    
    APP_WakeUpTimerConfig(1);
    
    GPIO_SetPinInterruptConfig(GPIO3, 14, kGPIO_InterruptFallingEdge);
    EnableIRQ(GPIO3_IRQn);

    APP_PowerPreSwitchHook(kAPP_PowerModeSleep);
    APP_PowerModeSwitch(kAPP_PowerModeSleep);
    APP_PowerPostSwitchHook();

//    PRINTF("\r\nNext loop.\r\n");
}




void GPIO3_IRQHandler(void)
{
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT) || \
    (!defined(FSL_FEATURE_SOC_PORT_COUNT))
    /* Clear external interrupt flag. */
    GPIO_GpioClearInterruptFlags(GPIO3, 1U << 14);
    button_flag= true;
    
#else
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
#endif
    /* Change state of button. */
    //g_ButtonPress = true;
    
    SDK_ISR_EXIT_BARRIER;
}



/*! @brief WakeUp Timer interrupt handler. */
void LPTMR0_IRQHandler(void)
{
    if ((LPTMR_GetStatusFlags(APP_WUU_WAKEUP_TIMER) & (uint32_t)kLPTMR_TimerCompareFlag) != 0UL)
    {
        LPTMR_DisableInterrupts(APP_WUU_WAKEUP_TIMER, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(APP_WUU_WAKEUP_TIMER, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(APP_WUU_WAKEUP_TIMER);
    }
    
    lptmr_flag = true;
}



/************************** Wakeup Configuration *******************************/

/*************************** Power Mode Switch ********************************/


/*! @brief Do low power mode switch. */
static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode)
{
    cmc_power_domain_config_t config;
    
    if (targetPowerMode == kAPP_PowerModeSleep)
    {                
      config.clock_mode  = kCMC_GateNoneClock;  //It has problem kCMC_GateCoreClock
      config.main_domain = kCMC_ActiveOrSleepMode;


      CMC_SetPowerModeProtection(APP_CMC, kCMC_AllowAllLowPowerModes);
      CMC_EnterLowPowerMode(APP_CMC, &config);
    }
}

__WEAK void APP_PowerPreSwitchHook(app_power_mode_t targetPowerMode)
{
    (void)targetPowerMode;
}

__WEAK void APP_PowerPostSwitchHook(void)
{
}

/*************************** Power Mode Switch ********************************/
