#pragma once
#include "ff.h"

#define INFO(fmt, args...)  INFO_PRINT("fatfs: " fmt, ##args)
#define DEBUG(fmt, args...) DEBUG_PRINT("fatfs: " fmt, ##args)
#define ERROR(fmt, args...) ERROR_PRINT("fatfs: " fmt, ##args)

#ifndef NAUT_CONFIG_DEBUG_FATFS_FILESYSTEM_DRIVER
#undef DEBUG
#define DEBUG(fmt, args...)
#endif

int nk_fs_fatfs_attach(char *devname, char *fsname, int readonly);
int nk_fs_fatfs_detach(char *fsname);


extern void* fatfs_global_state[3];



// /home/pdinda/HANDOUT/tools/bin/qemu-system-x86_64 -vga virtio -serial stdio -boot d -cdrom nautilus.iso -drive file=disk.img,format=raw,if=none,id=drive-virtio-blk0,werror=stop,rerror=stop -device virtio-blk-pci,drive=drive-virtio-blk0,id=virtio-blk0
