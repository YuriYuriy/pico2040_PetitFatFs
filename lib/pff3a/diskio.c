/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/
#include "diskio.h"

static uint8_t write_buffer[512];
static uint16_t write_index = 0;
static DWORD write_sector = 0;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(void)
{
	DSTATUS stat;
	uint8_t result_init = 0;

	result_init = init_sd_card();

	if (result_init == 0)
	{
		stat = RES_OK;
	}
	else
	{
		stat = STA_NOINIT;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp(
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	DRESULT res;

	uint8_t temp_sector[512];
    uint8_t result = read_sd_card(temp_sector,
                                  (sector >> 24) & 0xFF,
                                  (sector >> 16) & 0xFF,
                                  (sector >> 8) & 0xFF,
                                  sector & 0xFF);

    if (result != 0)
    {
        return RES_ERROR;
    }

    memcpy(buff, temp_sector + offset, count);
    return RES_OK;

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
    if (buff)
    {
        // Передача данных
        memcpy(&write_buffer[write_index], buff, sc);
        write_index += sc;
        return RES_OK;
    }
    else
    {
        if (sc)
        {
            // Инициализация записи
            write_index = 0;
            write_sector = sc;
            return RES_OK;
        }
        else
        {
            // Завершение записи
            uint8_t result = write_sd_card(write_buffer,
                                           (write_sector >> 24) & 0xFF,
                                           (write_sector >> 16) & 0xFF,
                                           (write_sector >> 8) & 0xFF,
                                           write_sector & 0xFF);
            return (result == 0) ? RES_OK : RES_ERROR;
        }
    }
}

