/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <oled_b_click.h>

// DISPLAY 96x39
#define DISPLAY_WIDTH 	96
#define DISPLAY_HEIGHT 	39
#define DISPLAY_HEIGHT_PAGE 5

#define OLED_ADDR_COMMAND 0x3C
#define OLED_ADDR_DATA 0x3D

// Commands --------------------------------
#define OLED_COMMAND								0x00
#define OLED_DATA									0x40
// Fundamental command table -----------
#define OLED_SET_CONTRAST_CONTROL 					0x81 // Data before
#define OLED_SET_ENTTIRE_DISPLAY_RAM 				0xA4
#define OLED_SET_ENTTIRE_DISPLAY_ON 				0xA5
#define OLED_SET_NORMAL_DISPLAY 					0xA6
#define OLED_SET_INVERT_DISPLAY 					0xA7
#define OLED_SET_OFF_DISPLAY 						0xAE
#define OLED_SET_ON_DISPLAY 						0xAF
// -------------------------------------

//Scrolling command table --------------
#define OLED_SCROLL_RIGHT_COMMAND					0x26
#define OLED_SCROLL_LEFT_COMMAND					0x27
#define OLED_SCROLL_SET_VERTICAL_AREA_COMMAND 		0xA3
#define OLED_DESACTIVATE_SCROLL						0x2E
#define OLED_ACTIVATE_SCROLL						0x2F
// -------------------------------------

// Addressing setting command table ----
#define OLED_COLUMN_LOWER_START_ADDRESS_COMMAND		0x00
#define OLED_COLUMN_HIGHER_START_ADDRESS_COMMAND	0x10
#define OLED_SET_MEMORY_ADDRESSING_MODE_COMMAND		0x20
#define OLED_SET_COLUMN_ADDRESS_COMMAND				0x21
#define OLED_SET_PAGE_ADDRESS_COMMAND				0x22
#define OLED_SET_PAGE_START_ADDRESS_COMMAND			0xB0
// -------------------------------------

// Hardware configuration --------------
#define OLED_SET_DISPLAY_START_LINE_COMMAND			0x40
#define OLED_SET_SEGMENT_REMAP_COMMAND				0xA0
#define OLED_SET_MULTIPLEX_RATIO_COMMAND			0xA8
#define OLED_SET_COM_OUTPUT_SCANN_DIRECION_COMMAND	0xC0
#define OLED_SET_DISPLAY_OFFSET_COMMAND				0xD3
#define OLED_SET_COM_PINS_HW_CONFIG_COMMAND			0xDA
// -------------------------------------

// Timing & Driving Scheme Setting -----
#define OLED_SET_DISPLAY_CLOCK_DIV_F_OSC			0xD5
#define OLED_SET_PRE_CHRG_PERIOD					0xD9
#define OLED_SET_VCOM_LEVEL							0xDB
// -------------------------------------
// -----------------------------------------


static uint8_t oled_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {0};


void oledBclick_cs(uint8_t val)
{
    GPIO_PinWrite(GPIO0,16,val);
}

void oledBclick_dc(uint8_t val)
{
  GPIO_PinWrite(GPIO0,17,val);
}

void oledBclick_rst(uint8_t val)
{
  GPIO_PinWrite(GPIO2,28,val);
}

static int oledBclick_i2cWrite(uint8_t slaveAddress, uint8_t *pBuf, uint16_t nBytes)
{
    status_t reVal = kStatus_Fail;
    lpi2c_master_transfer_t masterXfer = {0};
          /* subAddress = 0x01, data = g_master_txBuff - write to slave.
      start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
    uint8_t deviceAddress     = 0x0U;
    masterXfer.slaveAddress   = slaveAddress;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = pBuf;
    masterXfer.dataSize       = nBytes;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master non-blocking data to slave */
    reVal = LPI2C_MasterTransferEDMA(LPI2C0, &g_m_edma_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        asm("nop");
    }

            /*  Wait for transfer completed. */
    while (!g_MasterCompletionFlag)
    {
    }

    g_MasterCompletionFlag = false;



  return 0;


}


// Send options ---------------------------------------------------------------
int32_t oledBclick_send_command(uint8_t command)
{
	 uint8_t d[2];
	 oledBclick_dc(0);
	 d[0] = 0;
	 d[1] = command;
	 oledBclick_i2cWrite(OLED_ADDR_COMMAND,d,2);

	 return 0;
}






int32_t oledBclick_send_data(uint8_t* data, uint8_t len)
{
	uint8_t d[100];

	d[0] = 0x60;

	oledBclick_dc(1);

    memcpy(&d[1],data,len);

    oledBclick_i2cWrite(OLED_ADDR_DATA,d,len+1);

    return 0;
}
// ----------------------------------------------------------------------------
void oledBclick_init(void)
{
    oledBclick_rst(0);
    SDK_DelayAtLeastUs(10000, SystemCoreClock);
    oledBclick_rst(1);
    SDK_DelayAtLeastUs(10000, SystemCoreClock);


    oledBclick_send_command(OLED_SET_OFF_DISPLAY);                 //0xAE Set OLED Display Off
    oledBclick_send_command(OLED_SET_DISPLAY_CLOCK_DIV_F_OSC);     //0xD5 Set Display Clock Divide Ratio/Oscillator Frequency
    oledBclick_send_command(0x80);
    oledBclick_send_command(OLED_SET_MULTIPLEX_RATIO_COMMAND);     //0xA8 Set Multiplex Ratio
    oledBclick_send_command(0x27);
    oledBclick_send_command(OLED_SET_DISPLAY_OFFSET_COMMAND);      //0xD3 Set Display Offset
    oledBclick_send_command(0x00);
    oledBclick_send_command(OLED_SET_DISPLAY_START_LINE_COMMAND);  //0x40 Set Display Start Line
    oledBclick_send_command(0x8D);                                 //0x8D Set Charge Pump
    oledBclick_send_command(0x14);                                 //0x14 Enable Charge Pump
    oledBclick_send_command(OLED_SET_COM_OUTPUT_SCANN_DIRECION_COMMAND);             //0xC8 Set COM Output Scan Direction
    oledBclick_send_command(0xA0);
    oledBclick_send_command(OLED_SET_COM_PINS_HW_CONFIG_COMMAND);             //0xDA Set COM Pins Hardware Configuration
    oledBclick_send_command(0x12);
    oledBclick_send_command(OLED_SET_CONTRAST_CONTROL);            //0x81 Set Contrast Control
    oledBclick_send_command(0xAF);
    oledBclick_send_command(OLED_SET_PRE_CHRG_PERIOD);           //0xD9 Set Pre-Charge Period
    oledBclick_send_command(0x25);
    oledBclick_send_command(OLED_SET_VCOM_LEVEL);          //0xDB Set VCOMH Deselect Level
    oledBclick_send_command(OLED_SET_MEMORY_ADDRESSING_MODE_COMMAND);
    oledBclick_send_command(OLED_SET_ENTTIRE_DISPLAY_RAM);    //0xA4 Set Entire Display On/Off
    oledBclick_send_command(OLED_SET_NORMAL_DISPLAY);          //0xA6 Set Normal/Inverse Display
    oledBclick_send_command(OLED_SET_ON_DISPLAY);
}




int32_t oledBclick_set_page_start_address_for_page_addressing_mode(uint8_t start)
{
	int32_t retval;
	start &= 0x07;
	start |= OLED_SET_PAGE_START_ADDRESS_COMMAND;
	retval = oledBclick_send_command(start);
	return retval;
}




int32_t oledBclick_set_higher_column_start_address(uint8_t column_start_address)
{
	int32_t retval;
	column_start_address &= 0x0F;
	column_start_address |= OLED_COLUMN_HIGHER_START_ADDRESS_COMMAND;
	retval = oledBclick_send_command(column_start_address);
	return retval;
}




void oledBclick_ClearBuffer(void)
{
    memset(oled_buffer,0,sizeof(oled_buffer));
}


void oledBclick_Update(void)
{
    uint8_t page;

    for(page = 0; page < DISPLAY_HEIGHT_PAGE; page++)
    {
    	oledBclick_set_higher_column_start_address(0);
    	oledBclick_set_page_start_address_for_page_addressing_mode(page);

        oledBclick_send_data(&oled_buffer[page * DISPLAY_WIDTH],DISPLAY_WIDTH);

    }


}

void oledBclick_DrawPixel(uint8_t x,
                    uint8_t y,
                    uint8_t color)
{
    uint16_t index;
    uint8_t page;
    uint8_t bit;

    if(x >= DISPLAY_WIDTH)
        return;

    if(y >= DISPLAY_HEIGHT)
        return;

    page = y >> 3;
    bit  = y & 0x07;

    index = page * DISPLAY_WIDTH + x;

    if(color)
    {
        oled_buffer[index] |= (1 << bit);
    }
    else
    {
        oled_buffer[index] &= ~(1 << bit);
    }
}





void oledBclick_DrawPixelRotate180(uint8_t x,
                             uint8_t y,
                             uint8_t color)
{
    oledBclick_DrawPixel(
        DISPLAY_WIDTH  - 1 - x,
        DISPLAY_HEIGHT - 1 - y,
        color
    );
}



void oledBclick_DrawChar8x16(uint8_t x,
                   uint8_t y,
                   char ch)
{
    uint8_t col;
    uint8_t row;
    uint8_t data;

    if (x >= DISPLAY_WIDTH)
        return;

    if (y >= (DISPLAY_HEIGHT - 16))
        return;


    if(ch < 0x20 || ch > 0x7E)
    {
        ch='?';
    }

    if(ch=='m')
    {
    	ch = 10;
    }

    else
    {
    	ch-=48;
    }

    for(col = 0; col < 8; col++)
    {
        data = Font8x16[(uint8_t)ch][col];

        for(row = 0; row < 8; row++)
        {
            oledBclick_DrawPixelRotate180(
                x + col,
                y + row,
                (data & (1 << row)) ? 1 : 0
            );
        }

        data = Font8x16[(uint8_t)ch][col + 8];

        for(row = 0; row < 8; row++)
        {
            oledBclick_DrawPixelRotate180(
                x + col,
                y + row + 8,
                (data & (1 << row)) ? 1 : 0
            );
        }
    }
}


void oledBclick_WriteString8x16(uint8_t x,
                      uint8_t y,
                      const char *str)
{
    while(*str)
    {
        oledBclick_DrawChar8x16(x, y, *str++);

        x += 8;

        if(x >= (DISPLAY_WIDTH - 8))
        {
            break;
        }
    }
}



void oledBclick_clear_screen(void)
{
  oledBclick_ClearBuffer();
  oledBclick_Update();
}





void oledBclick_show(char* buf)
{
  oledBclick_ClearBuffer();
  oledBclick_WriteString8x16(0,0,buf);
  oledBclick_Update();
}



