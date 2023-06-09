/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <nautilus/blkdev.h>
#include <nautilus/nautilus.h>
#include <nautilus/dev.h>
#include <nautilus/fs.h>

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "fatfs.h"
extern void* fatfs_global_state[3];

typedef struct fatfs_pdrv_to_blkdev{
    int pdrv;
    struct nk_block_dev *dev;
} fatfs_pdrv_to_blkdev;

typedef struct fatfs_state{
    struct nk_block_dev_characteristics chars;
    struct nk_block_dev *dev;
    struct nk_fs *fs;
    struct FATFS *fatfs;
    fatfs_pdrv_to_blkdev **pdrv_to_blkdev;
    int num_pdrv;
} fatfs_state;

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
fatfs_state* get_fatfs_global_state(BYTE pdrv){
	return (fatfs_state*)fatfs_global_state[pdrv];
}


DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DEBUG("disk_status\n");
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DEBUG("disk_initialize\n");
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;
	fatfs_state *s = get_fatfs_global_state(pdrv);
	DEBUG("disk_read: pdrv=%d, sector=%d, count=%d\n", pdrv, sector, count);
	if(nk_block_dev_read(s->dev, sector, count, buff, NK_DEV_REQ_BLOCKING, 0, 0) != 0){
		ERROR("disk_read: nk_block_dev_read failed\n");
		return RES_ERROR;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
	DEBUG("disk_write, pdrv=%d, sector=%d, count=%d\n , buff=%p\n, state=%p\n", pdrv, sector, count, buff, fatfs_global_state);

	fatfs_state *s = get_fatfs_global_state(pdrv);

	int pass =nk_block_dev_write(s->dev, sector, count, (void*)buff, NK_DEV_REQ_BLOCKING, 0, 0);
	DEBUG("HERE\n");

	if (pass != 0){
		ERROR_PRINT("disk_write: nk_block_dev_write failed\n");
		return RES_ERROR;
	}	
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DEBUG("disk_ioctl, command = %d\n", cmd);
	// only use is to sync but nk forces disk to block so no need to sync
	return RES_OK;
}

