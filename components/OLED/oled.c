#include <driver/spi_master.h>
#include <freertos/task.h>
#include "oled.h"
#include "oledfont.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "io_config.h"
#include "peripheral_init.h"

uint8_t **Hzk;

spi_device_handle_t oled_spi_handle;

void OLED_Send(uint8_t dc, uint8_t data) {
    spi_transaction_t t = {0};
    t.length = 8;
    t.tx_buffer = &data;
    gpio_set_level(OLED_DC_Pin, dc == 0x40);
    spi_device_polling_transmit(oled_spi_handle, &t);
}

/// OLED init commands
uint8_t CMD_Data[] = {
        0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F,
        0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05, 0xD9, 0xF1, 0xDA,
        0x12, 0xD8, 0x30, 0x8D, 0x14, 0xAF
};

void OLED_WR_CMD(uint8_t cmd) {
    OLED_Send(0x00, cmd);
}

void OLED_WR_DATA(uint8_t data) {
    OLED_Send(0x40, data);
}

void OLED_Init(void) {
    OLED_GPIO_Init();
    OLED_SPI_Init();
    gpio_set_level(OLED_RES_Pin, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(OLED_RES_Pin, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    uint8_t i = 0;
    for (i = 0; i < 27; i++) {
        OLED_Send(0x00, CMD_Data[i]);
    }
}

void OLED_Clear() {
    uint8_t i, n;
    for (i = 0; i < 8; i++) {
        OLED_WR_CMD(0xb0 + i);
        OLED_WR_CMD(0x00);
        OLED_WR_CMD(0x10);
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(0);
    }
}

void OLED_Display_On(void) {
    OLED_WR_CMD(0X8D);  //SET DCDC命令
    OLED_WR_CMD(0X14);  //DCDC ON
    OLED_WR_CMD(0XAF);  //DISPLAY ON
}

void OLED_Display_Off(void) {
    OLED_WR_CMD(0X8D);  //SET DCDC命令
    OLED_WR_CMD(0X10);  //DCDC OFF
    OLED_WR_CMD(0XAE);  //DISPLAY OFF
}

void OLED_Set_Pos(uint8_t x, uint8_t y) {
    OLED_WR_CMD(0xb0 + y);
    OLED_WR_CMD(((x & 0xf0) >> 4) | 0x10);
    OLED_WR_CMD(x & 0x0f);
}

void OLED_On(void) {
    uint8_t i, n;
    for (i = 0; i < 8; i++) {
        OLED_WR_CMD(0xb0 + i);    //设置页地址（0~7）
        OLED_WR_CMD(0x00);      //设置显示位置—列低地址
        OLED_WR_CMD(0x10);      //设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(1);
    } //更新显示
}

unsigned int oled_pow(uint8_t m, uint8_t n) {
    unsigned int result = 1;
    while (n--)result *= m;
    return result;
}

void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size) {
    unsigned char c = 0, i = 0;
    c = chr - ' ';//得到偏移后的值
    if (x > 128 - 1) {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16) {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8x16[c * 16 + i]);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8x16[c * 16 + i + 8]);
    } else {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_DATA(F6x8[c][i]);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size) {
    unsigned char j = 0;
    while (chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > 120) {
            x = 0;
            y += 2;
        }
        j++;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t Char_Size) {
    uint32_t j = 1;
    if (num < 0) {
        OLED_ShowChar(x, y, '-', Char_Size);
        x += 8;
        if (x > 120) {
            x = 0;
            y += 2;
        }
        num = -num;
    }
    while (j <= num) {
        j *= 10;
    }
    j /= 10;
    while (j > 1) {
        OLED_ShowChar(x, y, num / j + '0', Char_Size);
        x += 8;
        if (x > 120) {
            x = 0;
            y += 2;
        }
        num %= j;
        j /= 10;
    }
    OLED_ShowChar(x, y, num + '0', Char_Size);
}

void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no) {
    uint8_t t, adder = 0;
    OLED_Set_Pos(x, y);
    for (t = 0; t < 16; t++) {
        OLED_WR_DATA(Hzk[2 * no][t]);
        adder += 1;
    }
    OLED_Set_Pos(x, y + 1);
    for (t = 0; t < 16; t++) {
        OLED_WR_DATA(Hzk[2 * no + 1][t]);
        adder += 1;
    }
}

void OLED_FullFlash(const char *buf) {
    for (uint8_t i = 0; i < 8; i++) {
        OLED_Set_Pos(0, i);
        for (uint8_t j = 0; j < 128; j++) {
            OLED_WR_DATA(*(buf + i * 128 + j));
        }
    }
}
