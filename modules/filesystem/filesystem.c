/*************************************************
Copyright (c) 2020
All rights reserved.
File name:     filesystem.c
Description:   
History:
1. Version:    V1.0.0
Date:      2020-09-06
Author:    WKJay
Modify:    
*************************************************/
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <fal.h>
#include <stdio.h>

#define FS_MOUNT_MAX_RETRY 3

#define DBG_TAG "fs"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static int fs_is_ready = 0;
extern int stm32_sdcard_mount(void);

int filesystem_init(void)
{
    uint8_t retry = 0;
    struct rt_device *flash_dev = fal_blk_device_create("filesystem");
_spiflash_mount_retry:
    if (dfs_mount(flash_dev->parent.name, "/", "elm", 0, 0) != 0)
    {
        rt_kprintf("norflash mount failed!\n");
        dfs_mkfs("elm", flash_dev->parent.name);
        if (retry == FS_MOUNT_MAX_RETRY)
            return -1;

        retry++;
        goto _spiflash_mount_retry;
    }
    else
    {
        stm32_sdcard_mount();
    }
    fs_is_ready = 1;
    return 0;
}

int root_filesystem_is_ready(void)
{
    return fs_is_ready;
}

static void wifi_image_write(void)
{
    uint8_t buf[2048];
    uint32_t len = 0, off = 0;
    int fp = 0;
    const struct fal_partition *part = fal_partition_find("wifi_image");
    ;

    fal_partition_erase(part, 0, 1024 * 1024);
    fp = open("/sd/duo-wifi-r1.rbl", O_RDONLY);
    if (fp)
    {
        while ((len = read(fp, buf, 2048)) == 2048)
        {
            fal_partition_write(part, off, buf, len);
            off += len;
        }
        fal_partition_write(part, off, buf, len);
        LOG_I("WIFI IMAGE WRITE SUCCESS!");
        close(fp);
    }
    else
    {
        LOG_E("cannot find file");
    }
}
MSH_CMD_EXPORT(wifi_image_write, wifi image write);
