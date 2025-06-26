/*-----------------------------------------------------------------------*/
/*--------Library for working SD Card with PFF by Yuri Yuriy 2025--------*/
/*--------------------------------RP 2040--------------------------------*/
/*-----------------------------------------------------------------------*/
#include "sd_card.h"

void init_spi_sd_card(void)
{
    spi_init(SPI_PORT, 300 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    sleep_ms(1);
}

void sd_card_cmd(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4, uint8_t crc)
{
    IND_SD_CARD_OFF
    uint8_t packet[6] = {cmd, arg1, arg2, arg3, arg4, crc};
    spi_write_blocking(SPI_PORT, packet, 6);
    IND_SD_CARD_ON
}
uint8_t sd_card_check_R1(void)
{
    uint8_t buff_ff = 0xFF;
    uint8_t result_R1 = 0xFF;

    for (uint16_t i = 0; i < 0xFFFF; i++)
    {
        spi_write_read_blocking(SPI_PORT, &buff_ff, &result_R1, 1);
        if (result_R1 == 0x00 || result_R1 == 0x01)
            return result_R1; // OK
    }
    return result_R1; // Error
}
uint8_t sd_card_check_R3(void)
{
    uint8_t buff_ff = 0xFF;
    uint8_t result_R3[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    result_R3[0] = sd_card_check_R1();

    if (result_R3[0] == 0x00 || result_R3[0] == 0x01)
    {
        for (uint8_t n = 1; n < 5; n++)
        {
            spi_write_read_blocking(SPI_PORT, &buff_ff, &result_R3[n], 1);
        }
        // printf("R3 response: %02X %02X %02X %02X %02X\n", result_R3[0], result_R3[1], result_R3[2], result_R3[3], result_R3[4]);
        return result_R3[0]; // OK
    }

    return result_R3[0]; // Error
}
uint8_t sd_card_check_R7(void)
{
    uint8_t buff_ff = 0xFF;
    uint8_t result_R7[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    result_R7[0] = sd_card_check_R1();

    if (result_R7[0] == 0x00 || result_R7[0] == 0x01)
    {
        for (uint8_t n = 1; n < 5; n++)
        {
            spi_write_read_blocking(SPI_PORT, &buff_ff, &result_R7[n], 1);
        }
        return result_R7[4]; // OK
    }

    return result_R7[0]; // Error
}
void sd_card_check_busy(void)
{
    uint8_t result_b = 0;
    uint8_t buf_ff = 0xFF;

    for (uint16_t i = 0; i < 0xFFFF; i++)
    {
        spi_write_read_blocking(SPI_PORT, &buf_ff, &result_b, 1);
        if (result_b == 0xFF)
            break;
    }
}

uint8_t init_sd_card(void)
{
    IND_SD_CARD_ON

    uint8_t buf_ff = 0xFF;
    uint8_t result = 0;

    for (uint8_t i = 0; i < 8; ++i)
    {
        spi_write_blocking(SPI_PORT, &buf_ff, 1);
    }
    //----------SendCMD0--------------------------------------------------------------
    gpio_put(PIN_CS, 0);
    sd_card_cmd(CMD0, 0x00, 0x00, 0x00, 0x00, 0x95);
    //----------CheckCMD0-------------------------------------------------------------
    result = sd_card_check_R1();
    spi_write_blocking(SPI_PORT, &buf_ff, 1);
    // printf("CMD0: %X\n", result);
    //----------ErrorCMD0-------------------------------------------------------------
    if (result != 0x01)
    {
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    //----------SendCMD8--------------------------------------------------------------
    sd_card_cmd(CMD8, 0x00, 0x00, 0x01, 0xAA, 0x87);
    sleep_ms(10);
    //----------CheckCMD8-------------------------------------------------------------
    result = sd_card_check_R7();
    spi_write_blocking(SPI_PORT, &buf_ff, 1);
    // printf("CMD8: %X\n", result);
    //----------ErrorCMD8-------------------------------------------------------------
    if (result != 0xAA)
    {
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    //----------SendACMD58------------------------------------------------------------
    sd_card_cmd(CMD58, 0x00, 0x00, 0x00, 0x00, 0xFD);
    //----------CheckCMD58------------------------------------------------------------
    result = sd_card_check_R3();
    spi_write_blocking(SPI_PORT, &buf_ff, 1);
    //----------ErrorCMD58------------------------------------------------------------
    if (result > 0x01)
    {
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    //----------SendACMD41------------------------------------------------------------
    for (uint16_t i = 0; i < 0xFFFF; i++)
    {
        sd_card_cmd(CMD55, 0x00, 0x00, 0x00, 0x00, 0x65);
        result = sd_card_check_R1();
        // printf("CMD55: %X\n", result);
        spi_write_blocking(SPI_PORT, &buf_ff, 1);

        sd_card_cmd(ACMD41, 0x40, 0x00, 0x00, 0x00, 0x77);
        result = sd_card_check_R1();
        // printf("CMD41: %X\n", result);
        spi_write_blocking(SPI_PORT, &buf_ff, 1);
        if (result == 0x00)
            break;
    }
    //----------SendACMD58------------------------------------------------------------
    if (result == 0x00)
    {
        sd_card_cmd(CMD58, 0x00, 0x00, 0x00, 0x00, 0xFD);
        //----------CheckCMD58------------------------------------------------------------
        result = sd_card_check_R3();
        spi_write_blocking(SPI_PORT, &buf_ff, 1);
        gpio_put(PIN_CS, 1);
        // printf("OK INIT: \n");
        spi_init(SPI_PORT, 20 * 1000 * 1000);
        IND_SD_CARD_OFF
        return 0;
    }
    gpio_put(PIN_CS, 1);
    // printf("ERROR ACMD41: \n");
    IND_SD_CARD_OFF
    return 1;
}
uint8_t write_sd_card(uint8_t *buff_w, uint8_t data_0, uint8_t data_1, uint8_t data_2, uint8_t data_3)
{
    IND_SD_CARD_ON

    uint8_t buf_ff = 0xFF;
    uint8_t StartBlockToken = 0xFE;
    uint8_t result_w;

    //----------SendCMD24-------------------------------------------------------------
    gpio_put(PIN_CS, 0);
    sd_card_cmd(CMD24, data_0, data_1, data_2, data_3, 0xFF);
    //----------CheckCMD24----------------------------------- -------------------------
    result_w = sd_card_check_R1();
    // printf("CMD24: %X\n", result_w);
    //----------ErrorCMD24------------------------------------------------------------
    if (result_w != 0x00)
    {
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    //----------CheckBusyToken--------------------------------------------------------
    sd_card_check_busy();
    //----------SendStartBlockToken---------------------------------------------------
    spi_write_blocking(SPI_PORT, &StartBlockToken, 1);
    //----------SendData--------------------------------------------------------------
    spi_write_blocking(SPI_PORT, buff_w, 512);
    //----------SendCRC---------------------------------------------------------------
    spi_write_blocking(SPI_PORT, &buf_ff, 1);
    spi_write_blocking(SPI_PORT, &buf_ff, 1);
    //----------CheckResponseToken----------------------------------------------------
    spi_write_read_blocking(SPI_PORT, &buf_ff, &result_w, 1);
    if (result_w != 0xE5)
    {
        // printf("Data Response Token: %X\n", result_w);
        // printf("ERROR write: %X\n", result_w);
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    // printf("Data Response Token: %X\n", result_w);
    //----------CheckBusyToken--------------------------------------------------------
    sd_card_check_busy();
    // printf("OK write:\n");
    gpio_put(PIN_CS, 1);
    IND_SD_CARD_OFF
    return 0;
}
uint8_t read_sd_card(uint8_t *buff_r, uint8_t data_0, uint8_t data_1, uint8_t data_2, uint8_t data_3)
{
    IND_SD_CARD_ON

    uint8_t buf_ff = 0xFF;
    uint8_t buf_CRC[2] = {0, 0};
    uint8_t result_r;

    //----------SendCMD17-------------------------------------------------------------
    gpio_put(PIN_CS, 0);
    sd_card_cmd(CMD17, data_0, data_1, data_2, data_3, 0xFF);
    //----------CheckCMD17------------------------------------------------------------
    result_r = sd_card_check_R1();
    // printf("CMD17: %X\n", result_r);
    //----------ErrorCMD17------------------------------------------------------------
    if (result_r != 0x00)
    {
        gpio_put(PIN_CS, 1);
        return result_r;
    }
    //----------CheckStartBlockToken--------------------------------------------------
    for (uint16_t i = 0; i < 0xFFFF; i++)
    {
        spi_write_read_blocking(SPI_PORT, &buf_ff, &result_r, 1);
        if (result_r == 0xFE)
            break;
    }
    if (result_r != 0xFE)
    {
        // printf("ERROR read: %X\n", result_r);
        gpio_put(PIN_CS, 1);
        IND_SD_CARD_OFF
        return 1;
    }
    //----------GetData---------------------------------------------------------------
    uint8_t tx_dummy[512];
    memset(tx_dummy, 0xFF, sizeof(tx_dummy));
    spi_write_read_blocking(SPI_PORT, tx_dummy, buff_r, 512);

    //----------GetCRC----------------------------------------------------------------
    spi_write_read_blocking(SPI_PORT, &buf_ff, &buf_CRC[0], 1);
    spi_write_read_blocking(SPI_PORT, &buf_ff, &buf_CRC[1], 1);

    // printf("OK read:\n");
    gpio_put(PIN_CS, 1);
    IND_SD_CARD_OFF
    return 0;
}