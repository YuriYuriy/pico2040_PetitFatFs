/*-----------------------------------------------------------------------*/
/*--------Library for working SD Card with PFF by Yuri Yuriy 2025--------*/
/*--------------------------------RP 2040--------------------------------*/
/*-----------------------------------------------------------------------*/
#ifndef _SD_CARD
#define _SD_CARD

#include "main.h"

//--------------------------------------------------------------------------------//
// Commands SD Card
//--------------------------------------------------------------------------------//
#define CMD0 0x40 + 0    // GO_IDLE_STATE
#define CMD8 0x40 + 8    // SEND_IF_COND проверки рабочего состояния интерфейса SD-карты
#define CMD10 0x40 + 10  // READ_CSD
#define CMD17 0x40 + 17  // READ_SINGLE_BLOCK
#define CMD24 0x40 + 24  //
#define CMD55 0x40 + 55  // APP_CMD
#define CMD58 0x40 + 58  // READ_OCR
#define ACMD41 0x40 + 41 // SD_SEND_OP_COND инициализации и проверки, завершена ли

//--------------------------------------------------------------------------------//
// Pins SD Card
//--------------------------------------------------------------------------------//
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 20
#define PIN_SCK 18
#define PIN_MOSI 19

//--------------------------------------------------------------------------------//
// Pins for indicator of work SD Card
//--------------------------------------------------------------------------------//
#define PIN_IND_SD_CARD 25
#define INI_IND_SD_CARD         \
    gpio_init(PIN_IND_SD_CARD); \
    gpio_set_dir(PIN_IND_SD_CARD, GPIO_OUT);
#define IND_SD_CARD_OFF gpio_put(PIN_IND_SD_CARD, 0);
#define IND_SD_CARD_ON gpio_put(PIN_IND_SD_CARD, 1);

//--------------------------------------------------------------------------------//
// Function lib
//--------------------------------------------------------------------------------//
void init_spi_sd_card(void);
void sd_card_cmd(uint8_t byte_0, uint8_t byte_1, uint8_t byte_2, uint8_t byte_3, uint8_t byte_4, uint8_t byte_5);

uint8_t sd_card_check_R1(void);
uint8_t sd_card_check_R3(void);
uint8_t sd_card_check_R7(void);

uint8_t init_sd_card(void);

uint8_t write_sd_card(uint8_t *buff_w, uint8_t data_0, uint8_t data_1, uint8_t data_2, uint8_t data_3);
uint8_t read_sd_card(uint8_t *buff_r, uint8_t data_0, uint8_t data_1, uint8_t data_2, uint8_t data_3);

#endif