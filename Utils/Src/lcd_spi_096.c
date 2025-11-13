#include "setting.h"
#if USE_SPI_LCD
#include "lcd_fonts.h"
#include "lcd_spi_096.h"

static pFONT *LCD_AsciiFonts;
static pFONT *LCD_CHFonts;

uint16_t LCD_Buff[1024];

struct LCD_Struct LCD;

void LCD_WriteCommand(uint8_t lcd_command) {
    while ((LCD_SPI.Instance->SR & 0x0080) != RESET) {
    }

    LCD_DC_Command;
    (&LCD_SPI)->Instance->DR = lcd_command;
    while ((LCD_SPI.Instance->SR & 0x0002) == 0) {
    }

    while ((LCD_SPI.Instance->SR & 0x0080) != RESET) {
    }

    LCD_DC_Data;
}

void LCD_WriteData_8bit(uint8_t lcd_data) {
    LCD_SPI.Instance->DR = lcd_data;
    while ((LCD_SPI.Instance->SR & 0x0002) == 0)
        ;
}

void LCD_WriteData_16bit(uint16_t lcd_data) {
    LCD_SPI.Instance->DR = lcd_data >> 8;
    while ((LCD_SPI.Instance->SR & 0x0002) == 0)
        ;
    LCD_SPI.Instance->DR = lcd_data;
    while ((LCD_SPI.Instance->SR & 0x0002) == 0)
        ;
}

void LCD_WriteBuff(uint16_t *DataBuff, uint16_t DataSize) {
    uint32_t i;

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 |= 0x0800;
    LCD_SPI.Instance->CR1 |= 0x0040;

    LCD_CS_L;

    for (i = 0; i < DataSize; i++) {
        LCD_SPI.Instance->DR = DataBuff[i];
        while ((LCD_SPI.Instance->SR & 0x0002) == 0)
            ;
    }
    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 &= 0xF7FF;
    LCD_SPI.Instance->CR1 |= 0x0040;
}

#define GPIO_LCD_SCK_CLK __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_LCD_SDA_CLK __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_LCD_CS_CLK __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_LCD_DC_CLK __HAL_RCC_GPIOG_CLK_ENABLE()
#define GPIO_LCD_Backlight_CLK __HAL_RCC_GPIOH_CLK_ENABLE()

void SPI_LCD_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_SPI2_CLK_ENABLE();

    GPIO_LCD_SCK_CLK;
    GPIO_LCD_SDA_CLK;
    GPIO_LCD_CS_CLK;
    GPIO_LCD_Backlight_CLK;
    GPIO_LCD_DC_CLK;

    GPIO_InitStruct.Pin       = LCD_SPI_SCK_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(LCD_SPI_SCK_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_SPI_SDA_Pin;
    HAL_GPIO_Init(LCD_SPI_SDA_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LCD_SPI_CS_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_SPI_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LCD_BL_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LCD_SPI_DC_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_SPI_DC_GPIO_Port, &GPIO_InitStruct);

    LCD_DC_Data;
    LCD_CS_H;
    LCD_SPI_BL_OFF;

    MX_SPI2_Init();
    __HAL_SPI_ENABLE(&LCD_SPI);
    SPI_1LINE_TX(&LCD_SPI);

    LCD_CS_H;
    LCD_DC_Data;
    LCD_SPI_BL_OFF;

    HAL_Delay(10);

    LCD_CS_L;

    LCD_WriteCommand(0x36);
    LCD_WriteData_8bit(0x00);

    LCD_WriteCommand(0x3A);
    LCD_WriteData_8bit(0x05);

    LCD_WriteCommand(0xB2);
    LCD_WriteData_8bit(0x0C);
    LCD_WriteData_8bit(0x0C);
    LCD_WriteData_8bit(0x00);
    LCD_WriteData_8bit(0x33);
    LCD_WriteData_8bit(0x33);

    LCD_WriteCommand(0xB7);
    LCD_WriteData_8bit(0x35);

    LCD_WriteCommand(0xBB);
    LCD_WriteData_8bit(0x19);

    LCD_WriteCommand(0xC0);
    LCD_WriteData_8bit(0x2C);

    LCD_WriteCommand(0xC2);
    LCD_WriteData_8bit(0x01);

    LCD_WriteCommand(0xC3);
    LCD_WriteData_8bit(0x12);

    LCD_WriteCommand(0xC4);
    LCD_WriteData_8bit(0x20);

    LCD_WriteCommand(0xC6);
    LCD_WriteData_8bit(0x0F);

    LCD_WriteCommand(0xD0);
    LCD_WriteData_8bit(0xA4);
    LCD_WriteData_8bit(0xA1);

    LCD_WriteCommand(0xE0);
    LCD_WriteData_8bit(0xD0);
    LCD_WriteData_8bit(0x04);
    LCD_WriteData_8bit(0x0D);
    LCD_WriteData_8bit(0x11);
    LCD_WriteData_8bit(0x13);
    LCD_WriteData_8bit(0x2B);
    LCD_WriteData_8bit(0x3F);
    LCD_WriteData_8bit(0x54);
    LCD_WriteData_8bit(0x4C);
    LCD_WriteData_8bit(0x18);
    LCD_WriteData_8bit(0x0D);
    LCD_WriteData_8bit(0x0B);
    LCD_WriteData_8bit(0x1F);
    LCD_WriteData_8bit(0x23);

    LCD_WriteCommand(0xE1);
    LCD_WriteData_8bit(0xD0);
    LCD_WriteData_8bit(0x04);
    LCD_WriteData_8bit(0x0C);
    LCD_WriteData_8bit(0x11);
    LCD_WriteData_8bit(0x13);
    LCD_WriteData_8bit(0x2C);
    LCD_WriteData_8bit(0x3F);
    LCD_WriteData_8bit(0x44);
    LCD_WriteData_8bit(0x51);
    LCD_WriteData_8bit(0x2F);
    LCD_WriteData_8bit(0x1F);
    LCD_WriteData_8bit(0x1F);
    LCD_WriteData_8bit(0x20);
    LCD_WriteData_8bit(0x23);

    LCD_WriteCommand(0x21);

    LCD_WriteCommand(0x11);
    HAL_Delay(120);

    LCD_WriteCommand(0x29);

    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;

    LCD_SetDirection(Direction_V);
    LCD_SetBackColor(LCD_BLACK);
    LCD_SetColor(LCD_WHITE);
    LCD_Clear();

    LCD_SetAsciiFont(&ASCII_Font24);
    LCD_ShowNumMode(Fill_Zero);

    LCD_SPI_BL_ON;
}

void LCD_SetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_CS_L;

    LCD_WriteCommand(0x2a);
    LCD_WriteData_16bit(x1 + LCD.X_Offset);
    LCD_WriteData_16bit(x2 + LCD.X_Offset);

    LCD_WriteCommand(0x2b);
    LCD_WriteData_16bit(y1 + LCD.Y_Offset);
    LCD_WriteData_16bit(y2 + LCD.Y_Offset);

    LCD_WriteCommand(0x2c);

    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;
}

void LCD_SetColor(uint32_t Color) {
    uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;

    Red_Value   = (uint16_t)((Color & 0x00F80000) >> 8);
    Green_Value = (uint16_t)((Color & 0x0000FC00) >> 5);
    Blue_Value  = (uint16_t)((Color & 0x000000F8) >> 3);

    LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);
}

void LCD_SetBackColor(uint32_t Color) {
    uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;

    Red_Value   = (uint16_t)((Color & 0x00F80000) >> 8);
    Green_Value = (uint16_t)((Color & 0x0000FC00) >> 5);
    Blue_Value  = (uint16_t)((Color & 0x000000F8) >> 3);

    LCD.BackColor = (uint16_t)(Red_Value | Green_Value | Blue_Value);
}

void LCD_SetDirection(uint8_t direction) {
    LCD.Direction = direction;

    LCD_CS_L;

    if (direction == Direction_H) {
        LCD_WriteCommand(0x36);
        LCD_WriteData_8bit(0x70);
        LCD.X_Offset = 0;
        LCD.Y_Offset = 0;
        LCD.Width    = LCD_HEIGHT;
        LCD.Height   = LCD_WIDTH;
    } else if (direction == Direction_V) {
        LCD_WriteCommand(0x36);
        LCD_WriteData_8bit(0x00);
        LCD.X_Offset = 0;
        LCD.Y_Offset = 0;
        LCD.Width    = LCD_WIDTH;
        LCD.Height   = LCD_HEIGHT;
    } else if (direction == Direction_H_Flip) {
        LCD_WriteCommand(0x36);
        LCD_WriteData_8bit(0xA0);
        LCD.X_Offset = 80;
        LCD.Y_Offset = 0;
        LCD.Width    = LCD_HEIGHT;
        LCD.Height   = LCD_WIDTH;
    } else if (direction == Direction_V_Flip) {
        LCD_WriteCommand(0x36);
        LCD_WriteData_8bit(0xC0);
        LCD.X_Offset = 0;
        LCD.Y_Offset = 80;
        LCD.Width    = LCD_WIDTH;
        LCD.Height   = LCD_HEIGHT;
    }

    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;
}

void LCD_SetAsciiFont(pFONT *Asciifonts) { LCD_AsciiFonts = Asciifonts; }

void LCD_Clear(void) {
    uint32_t i;

    LCD_SetAddress(0, 0, LCD.Width - 1, LCD.Height - 1);

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 |= 0x0800;
    LCD_SPI.Instance->CR1 |= 0x0040;

    LCD_CS_L;

    for (i = 0; i < LCD.Width * LCD.Height; i++) {
        LCD_SPI.Instance->DR = LCD.BackColor;
        while ((LCD_SPI.Instance->SR & 0x0002) == 0)
            ;
    }
    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 &= 0xF7FF;
    LCD_SPI.Instance->CR1 |= 0x0040;
}

void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t i;

    LCD_SetAddress(x, y, x + width - 1, y + height - 1);

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 |= 0x0800;
    LCD_SPI.Instance->CR1 |= 0x0040;

    LCD_CS_L;

    for (i = 0; i < width * height; i++) {
        LCD_SPI.Instance->DR = LCD.BackColor;
        while ((LCD_SPI.Instance->SR & 0x0002) == 0)
            ;
    }
    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 &= 0xF7FF;
    LCD_SPI.Instance->CR1 |= 0x0040;
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint32_t color) {
    LCD_SetAddress(x, y, x, y);

    LCD_CS_L;

    LCD_WriteData_16bit(color);

    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;
}

void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t c) {
    uint16_t index = 0, counter = 0, i = 0, w = 0;
    uint8_t disChar;

    c = c - 32;

    LCD_CS_L;

    for (index = 0; index < LCD_AsciiFonts->Sizes; index++) {
        disChar = LCD_AsciiFonts->pTable[c * LCD_AsciiFonts->Sizes + index];
        for (counter = 0; counter < 8; counter++) {
            if (disChar & 0x01) {
                LCD_Buff[i] = LCD.Color;
            } else {
                LCD_Buff[i] = LCD.BackColor;
            }
            disChar >>= 1;
            i++;
            w++;
            if (w == LCD_AsciiFonts->Width) {

                w = 0;
                break;
            }
        }
    }
    LCD_SetAddress(x, y, x + LCD_AsciiFonts->Width - 1, y + LCD_AsciiFonts->Height - 1);
    LCD_WriteBuff(LCD_Buff, LCD_AsciiFonts->Width * LCD_AsciiFonts->Height);
}

void LCD_DisplayString(uint16_t x, uint16_t y, char *p) {
    while ((x < LCD.Width) && (*p != 0)) {
        LCD_DisplayChar(x, y, *p);
        x += LCD_AsciiFonts->Width;
        p++;
    }
}

void LCD_SetTextFont(pFONT *fonts) {
    LCD_CHFonts = fonts;
    switch (fonts->Width) {
    case 12:
        LCD_AsciiFonts = &ASCII_Font12;
        break;
    case 16:
        LCD_AsciiFonts = &ASCII_Font16;
        break;
    case 20:
        LCD_AsciiFonts = &ASCII_Font20;
        break;
    case 24:
        LCD_AsciiFonts = &ASCII_Font24;
        break;
    case 32:
        LCD_AsciiFonts = &ASCII_Font32;
        break;
    default:
        break;
    }
}

void LCD_DisplayChinese(uint16_t x, uint16_t y, char *pText) {
    uint16_t i = 0, index = 0, counter = 0;
    uint16_t addr;
    uint8_t disChar;
    uint16_t Xaddress = 0;

    while (1) {

        if (*(LCD_CHFonts->pTable + (i + 1) * LCD_CHFonts->Sizes + 0) == *pText &&
            *(LCD_CHFonts->pTable + (i + 1) * LCD_CHFonts->Sizes + 1) == *(pText + 1)) {
            addr = i;
            break;
        }
        i += 2;

        if (i >= LCD_CHFonts->Table_Rows) break;
    }
    i = 0;
    for (index = 0; index < LCD_CHFonts->Sizes; index++) {
        disChar = *(LCD_CHFonts->pTable + (addr)*LCD_CHFonts->Sizes + index);

        for (counter = 0; counter < 8; counter++) {
            if (disChar & 0x01) {
                LCD_Buff[i] = LCD.Color;
            } else {
                LCD_Buff[i] = LCD.BackColor;
            }
            i++;
            disChar >>= 1;
            Xaddress++;

            if (Xaddress == LCD_CHFonts->Width) {

                Xaddress = 0;
                break;
            }
        }
    }
    LCD_SetAddress(x, y, x + LCD_CHFonts->Width - 1, y + LCD_CHFonts->Height - 1);
    LCD_WriteBuff(LCD_Buff, LCD_CHFonts->Width * LCD_CHFonts->Height);
}

void LCD_DisplayText(uint16_t x, uint16_t y, char *pText) {
    while (*pText != 0) {
        if (*pText <= 0x7F) {
            LCD_DisplayChar(x, y, *pText);
            x += LCD_AsciiFonts->Width;
            pText++;
        } else {
            LCD_DisplayChinese(x, y, pText);
            x += LCD_CHFonts->Width;
            pText += 2;
        }
    }
}

void LCD_ShowNumMode(uint8_t mode) { LCD.ShowNum_Mode = mode; }

void LCD_DisplayNumber(uint16_t x, uint16_t y, int32_t number, uint8_t len) {
    char Number_Buffer[15];

    if (LCD.ShowNum_Mode == Fill_Zero) {
        sprintf(Number_Buffer, "%0.*d", len, number);
    } else {
        sprintf(Number_Buffer, "%*d", len, number);
    }

    LCD_DisplayString(x, y, (char *)Number_Buffer);
}

void LCD_DisplayDecimals(uint16_t x, uint16_t y, double decimals, uint8_t len, uint8_t decs) {
    char Number_Buffer[20];

    if (LCD.ShowNum_Mode == Fill_Zero) {
        sprintf(Number_Buffer, "%0*.*lf", len, decs, decimals);
    } else {
        sprintf(Number_Buffer, "%*.*lf", len, decs, decimals);
    }

    LCD_DisplayString(x, y, (char *)Number_Buffer);
}

#define ABS(X) ((X) > 0 ? (X) : -(X))

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0,
            den = 0, num = 0, numadd = 0, numpixels = 0, curpixel = 0;

    deltax = ABS(x2 - x1); /* The difference between the x's */
    deltay = ABS(y2 - y1); /* The difference between the y's */
    x      = x1;           /* Start x off at the first pixel */
    y      = y1;           /* Start y off at the first pixel */

    if (x2 >= x1) /* The x-values are increasing */
    {
        xinc1 = 1;
        xinc2 = 1;
    } else /* The x-values are decreasing */
    {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1) /* The y-values are increasing */
    {
        yinc1 = 1;
        yinc2 = 1;
    } else /* The y-values are decreasing */
    {
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay) /* There is at least one x-value for every y-value */
    {
        xinc1     = 0; /* Don't change the x when numerator >= denominator */
        yinc2     = 0; /* Don't change the y for every iteration */
        den       = deltax;
        num       = deltax / 2;
        numadd    = deltay;
        numpixels = deltax; /* There are more x-values than y-values */
    } else                  /* There is at least one y-value for every x-value */
    {
        xinc2     = 0; /* Don't change the x for every iteration */
        yinc1     = 0; /* Don't change the y when numerator >= denominator */
        den       = deltay;
        num       = deltay / 2;
        numadd    = deltax;
        numpixels = deltay; /* There are more y-values than x-values */
    }
    for (curpixel = 0; curpixel <= numpixels; curpixel++) {
        LCD_DrawPoint(x, y, LCD.Color); /* Draw the current pixel */
        num += numadd;                  /* Increase the numerator by the top of the fraction */
        if (num >= den)                 /* Check if numerator >= denominator */
        {
            num -= den; /* Calculate the new numerator value */
            x += xinc1; /* Change the x as appropriate */
            y += yinc1; /* Change the y as appropriate */
        }
        x += xinc2; /* Change the x as appropriate */
        y += yinc2; /* Change the y as appropriate */
    }
}

void LCD_DrawLine_V(uint16_t x, uint16_t y, uint16_t height) {
    uint16_t i;

    for (i = 0; i < height; i++) {
        LCD_Buff[i] = LCD.Color;
    }
    LCD_SetAddress(x, y, x, y + height - 1);

    LCD_WriteBuff(LCD_Buff, height);
}

void LCD_DrawLine_H(uint16_t x, uint16_t y, uint16_t width) {
    uint16_t i;

    for (i = 0; i < width; i++) {
        LCD_Buff[i] = LCD.Color;
    }
    LCD_SetAddress(x, y, x + width - 1, y);

    LCD_WriteBuff(LCD_Buff, width);
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {

    LCD_DrawLine_H(x, y, width);
    LCD_DrawLine_H(x, y + height - 1, width);

    LCD_DrawLine_V(x, y, height);
    LCD_DrawLine_V(x + width - 1, y, height);
}

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r) {
    int Xadd = -r, Yadd = 0, err = 2 - 2 * r, e2;
    do {
        LCD_DrawPoint(x - Xadd, y + Yadd, LCD.Color);
        LCD_DrawPoint(x + Xadd, y + Yadd, LCD.Color);
        LCD_DrawPoint(x + Xadd, y - Yadd, LCD.Color);
        LCD_DrawPoint(x - Xadd, y - Yadd, LCD.Color);

        e2 = err;
        if (e2 <= Yadd) {
            err += ++Yadd * 2 + 1;
            if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
        }
        if (e2 > Xadd) err += ++Xadd * 2 + 1;
    } while (Xadd <= 0);
}

void LCD_DrawEllipse(int x, int y, int r1, int r2) {
    int Xadd = -r1, Yadd = 0, err = 2 - 2 * r1, e2;
    float K = 0, rad1 = 0, rad2 = 0;

    rad1 = r1;
    rad2 = r2;

    if (r1 > r2) {
        do {
            K = (float)(rad1 / rad2);

            LCD_DrawPoint(x - Xadd, y + (uint16_t)(Yadd / K), LCD.Color);
            LCD_DrawPoint(x + Xadd, y + (uint16_t)(Yadd / K), LCD.Color);
            LCD_DrawPoint(x + Xadd, y - (uint16_t)(Yadd / K), LCD.Color);
            LCD_DrawPoint(x - Xadd, y - (uint16_t)(Yadd / K), LCD.Color);

            e2 = err;
            if (e2 <= Yadd) {
                err += ++Yadd * 2 + 1;
                if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
            }
            if (e2 > Xadd) err += ++Xadd * 2 + 1;
        } while (Xadd <= 0);
    } else {
        Yadd = -r2;
        Xadd = 0;
        do {
            K = (float)(rad2 / rad1);

            LCD_DrawPoint(x - (uint16_t)(Xadd / K), y + Yadd, LCD.Color);
            LCD_DrawPoint(x + (uint16_t)(Xadd / K), y + Yadd, LCD.Color);
            LCD_DrawPoint(x + (uint16_t)(Xadd / K), y - Yadd, LCD.Color);
            LCD_DrawPoint(x - (uint16_t)(Xadd / K), y - Yadd, LCD.Color);

            e2 = err;
            if (e2 <= Xadd) {
                err += ++Xadd * 3 + 1;
                if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
            }
            if (e2 > Yadd) err += ++Yadd * 3 + 1;
        } while (Yadd <= 0);
    }
}

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r) {
    int32_t D;     /* Decision Variable */
    uint32_t CurX; /* Current X Value */
    uint32_t CurY; /* Current Y Value */

    D = 3 - (r << 1);

    CurX = 0;
    CurY = r;

    while (CurX <= CurY) {
        if (CurY > 0) {
            LCD_DrawLine_V(x - CurX, y - CurY, 2 * CurY);
            LCD_DrawLine_V(x + CurX, y - CurY, 2 * CurY);
        }

        if (CurX > 0) {

            LCD_DrawLine_V(x - CurY, y - CurX, 2 * CurX);
            LCD_DrawLine_V(x + CurY, y - CurX, 2 * CurX);
        }
        if (D < 0) {
            D += (CurX << 2) + 6;
        } else {
            D += ((CurX - CurY) << 2) + 10;
            CurY--;
        }
        CurX++;
    }
    LCD_DrawCircle(x, y, r);
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    uint16_t i;

    LCD_SetAddress(x, y, x + width - 1, y + height - 1);

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 |= 0x0800;
    LCD_SPI.Instance->CR1 |= 0x0040;

    LCD_CS_L;

    for (i = 0; i < width * height; i++) {
        LCD_SPI.Instance->DR = color;
        while ((LCD_SPI.Instance->SR & 0x0002) == 0)
            ;
    }
    while ((LCD_SPI.Instance->SR & 0x0080) != RESET)
        ;
    LCD_CS_H;

    LCD_SPI.Instance->CR1 &= 0xFFBF;
    LCD_SPI.Instance->CR1 &= 0xF7FF;
    LCD_SPI.Instance->CR1 |= 0x0040;
}

void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *pImage) {
    uint8_t disChar;
    uint16_t Xaddress = x;
    uint16_t Yaddress = y;
    uint16_t i = 0, j = 0, m = 0;
    uint16_t BuffCount   = 0;
    uint16_t Buff_Height = 0;

    Buff_Height = (sizeof(LCD_Buff) / 2) / height;

    for (i = 0; i < height; i++) {
        for (j = 0; j < (float)width / 8; j++) {
            disChar = *pImage;

            for (m = 0; m < 8; m++) {
                if (disChar & 0x01) {
                    LCD_Buff[BuffCount] = LCD.Color;
                } else {
                    LCD_Buff[BuffCount] = LCD.BackColor;
                }
                disChar >>= 1;
                Xaddress++;
                BuffCount++;
                if ((Xaddress - x) == width) {
                    Xaddress = x;
                    break;
                }
            }
            pImage++;
        }
        if (BuffCount == Buff_Height * width) {
            BuffCount = 0;

            LCD_SetAddress(x, Yaddress, x + width - 1, Yaddress + Buff_Height - 1);
            LCD_WriteBuff(LCD_Buff, width * Buff_Height);

            Yaddress = Yaddress + Buff_Height;
        }
        if ((i + 1) == height) {
            LCD_SetAddress(x, Yaddress, x + width - 1, i + y);
            LCD_WriteBuff(LCD_Buff, width * (i + 1 + y - Yaddress));
        }
    }
}
#endif
