
/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
extern "C"
{
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_edma.h"
#include "fsl_edma.h"
#include "app.h"
#include "power_mode_switch.h"  
#include "oled_b_click.h"
}

#include <tof_driver_wrapper.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/



/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void InitI2C(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct tmf882x_msg_meas_results results;
volatile bool g_MasterCompletionFlag = false;
AT_NONCACHEABLE_SECTION(lpi2c_master_edma_handle_t g_m_edma_handle);

edma_handle_t g_edmaTxHandle;
edma_handle_t g_edmaRxHandle;


bool lptmr_flag = false;
bool button_flag = false;
bool active_flag = false;


TMF882X  tmf882x;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    char buf[20];  
    uint32_t presence;

    BOARD_InitHardware();
    
    InitI2C();
    
    oledBclick_init();

    oledBclick_clear_screen();
   
        
    tmf882x.init();

    
    while (1)
    {
      low_power_enter();

     tmf882x.measure(results);
     presence = results.results[0].distance_mm;
     sprintf(buf,"%ldmm",presence);

            
      if(button_flag)
      {
        button_flag = false;

        active_flag = true;
      }
      
      if((lptmr_flag==true) &&  presence<=100)
      {
        lptmr_flag = false;
        active_flag = true;
      }

      if(lptmr_flag==true && presence>100 && active_flag==false)
      {
    	  oledBclick_clear_screen();
      }
      

      if(active_flag)
      {
    	oledBclick_show(buf);
        active_flag = false;
      }
      
    }
}

void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_edma_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_MasterCompletionFlag = true;
    }
}


void InitI2C(void)
{
    edma_config_t userConfig;

  
    /* EDMA init */
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&userConfig);
    
    /* Release peripheral reset */
    RESET_ReleasePeripheralReset(kDMA0_RST_SHIFT_RSTn);
#if defined(BOARD_GetEDMAConfig)
    BOARD_GetEDMAConfig(userConfig);
#endif
    EDMA_Init(DMA0, &userConfig);
      /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    lpi2c_master_config_t masterConfig;
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = 1000000U;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(LPI2C0, &masterConfig, CLOCK_GetLpi2cClkFreq(0u));

    /* Create the EDMA channel handles */
    EDMA_CreateHandle(&g_edmaTxHandle, DMA0, 2U);
    EDMA_CreateHandle(&g_edmaRxHandle, DMA0, 3U);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
#if defined(DEMO_LPI2C_TRANSMIT_EDMA_CHANNEL)
    EDMA_SetChannelMux(EXAMPLE_LPI2C_MASTER_DMA, LPI2C_TRANSMIT_DMA_CHANNEL, DEMO_LPI2C_TRANSMIT_EDMA_CHANNEL);
#endif
#if defined(DEMO_LPI2C_RECEIVE_EDMA_CHANNEL)
    EDMA_SetChannelMux(EXAMPLE_LPI2C_MASTER_DMA, LPI2C_RECEIVE_DMA_CHANNEL, DEMO_LPI2C_RECEIVE_EDMA_CHANNEL);
#endif
#endif
    /* Create the LPI2C master DMA driver handle */
    LPI2C_MasterCreateEDMAHandle(LPI2C0, &g_m_edma_handle, &g_edmaRxHandle, &g_edmaTxHandle,
                                 lpi2c_master_callback, NULL);
  
}








