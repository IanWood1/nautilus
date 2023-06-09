
#include <nautilus/nautilus.h>
#include <nautilus/dev.h>
#include <nautilus/blkdev.h>
#include <nautilus/fs.h>

#include "fatfs.h"

typedef struct fatfs_state{
    struct nk_block_dev_characteristics chars;
    struct nk_block_dev *dev;
    struct nk_fs *fs;
    FATFS *fatfs;
    int num_pdrv;
} fatfs_state;

void* fatfs_global_state[3] = {NULL, NULL, NULL};

void store_fatfs_state(void* state) {
    fatfs_state *fs_state = (fatfs_state*)state;
    BYTE pdrv = fs_state->fatfs->pdrv;
    fatfs_global_state[pdrv] = state;
}


int fatfs_stat_path(void *state, char *path, struct nk_fs_stat *st)
{
    DEBUG("fatfs_stat_path\n");
    store_fatfs_state(state);
    

    FILINFO* fno = malloc(sizeof(FILINFO));
    if (!fno) {
        ERROR("fatfs_stat_path: malloc failed\n");
        return -1;
    }
    memset(fno, 0, sizeof(FILINFO));

    FRESULT res = f_stat(path, fno);
    if (res != FR_OK) {
        ERROR("fatfs_stat_path: f_stat failed with error %d\n", res);
        return -1;
    }
    st->st_size = fno->fsize;
    return 0;
}

void *fatfs_create_file(void *state, char *path)
{
    DEBUG("fatfs_create_file\n");
    store_fatfs_state(state);
    FIL* file = malloc(sizeof(FIL));
    if (!file) {
        ERROR("fatfs_create_file: malloc failed\n");
        return NULL;
    }

    FRESULT res = f_open(file, path, FA_CREATE_NEW | FA_READ | FA_WRITE);
    if (res != FR_OK) {
        ERROR("fatfs_create_file: f_open failed with error %d\n", res);
        return NULL;
    }
    return file;
}

int fatfs_create_dir(void *state, char *path)
{
    DEBUG("fatfs_create_dir\n");
        store_fatfs_state(state);

    FRESULT res = f_mkdir(path);
    if (res != FR_OK) {
        ERROR("fatfs_create_dir: f_mkdir failed with error %d\n", res);
        return -1;
    }
    return 0;
}

int fatfs_exists(void *state, char *path)
{

    store_fatfs_state(state);
    FRESULT res = f_stat(path, NULL);
    DEBUG("fatfs_exists res = %d\n", res);
    if(res != FR_OK && res != FR_NO_FILE && res != FR_NO_PATH) {
        ERROR("fatfs_exists: f_stat failed with error %d\n", res);
        return 0;
    }
    return res == FR_OK;
}

int fatfs_remove(void *state, char *path)
{
    DEBUG("fatfs_remove\n");
    store_fatfs_state(state);
    FRESULT res = f_unlink(path);
    return res;
}

void *fatfs_open(void *state, char *path)
{
    DEBUG("fatfs_open!\n");
    store_fatfs_state(state);
    FIL* file = malloc(sizeof(FIL));
    if (!file) {
        ERROR("fatfs_open: malloc failed\n");
        return NULL;
    }
    FRESULT res = f_open(file, path, FA_READ | FA_WRITE);
    if(res != FR_OK) {
        ERROR("fatfs_open: f_open failed with error %d\n", res);
        return NULL;
    }
    return file;
}

int fatfs_stat(void *state, void *file, struct nk_fs_stat *st)
{
    DEBUG("fatfs_stat\n");
    store_fatfs_state(state);
    FILINFO* fno = malloc(sizeof(FILINFO));
    if (!fno) {
        ERROR("fatfs_stat: malloc failed\n");
        return -1;
    }
    memset(fno, 0, sizeof(FILINFO));

    FRESULT res = f_stat(file, fno);
    if (res != FR_OK) {
        ERROR("fatfs_stat: f_stat failed with error %d\n", res);
        return -1;
    }

    // Put info into nk_fs_stat st
    st->st_size = fno->fsize;
    free(fno);
    return 0;
}

int fatfs_truncate(void *state, void *file, off_t len)
{
    DEBUG("fatfs_truncate\n");
    store_fatfs_state(state);
    FRESULT res = f_truncate(file);
    if (res != FR_OK) {
        ERROR("fatfs_truncate: f_truncate failed with error %d\n", res);
        return -1;
    }
    return 0;
}

void fatfs_close(void *state, void *file)
{
    fatfs_state* s = (fatfs_state*)state;
    struct nk_block_dev *dev = s->dev;
    DEBUG("dev->state = %p\n", dev->dev.state);
    store_fatfs_state(state);
    DEBUG("-----fatfs_close-----\n");
    FRESULT res = f_close(file);
    DEBUG("-----testfs1-----\n");

    if (res != FR_OK) {
        ERROR("fatfs_close: f_close failed with error %d\n", res);
        return;
    }
}

ssize_t fatfs_read(void *state, void *file, void *dest, off_t offset, size_t n)
{
    DEBUG("fatfs_read offset = %d, n = %d\n", offset, n);
    DEBUG("fp->obj.objsize = %d\n", ((FIL*)file)->obj.objsize);
    DEBUG("fp = %p\n", file);
    store_fatfs_state(state);

    int bytes_read = 0;
    FIL* f = (FIL*)file;
    f->fptr = offset; // set file pointer to offset
    FRESULT res = f_read(f, dest, n, &bytes_read);
    if (res != FR_OK) {
        ERROR("fatfs_read: f_read failed with error %d\n", res);
        return -1;
    }
    return bytes_read;
    
}

ssize_t fatfs_write(void *state, void *file, void *src, off_t offset, size_t n)
{
    store_fatfs_state(state);
    int bytes_written = 0;
    store_fatfs_state(state);
    FRESULT res = f_write(file, src, n, &bytes_written);
    if (res != FR_OK) {
        ERROR("fatfs_write: f_write failed with error %d\n", res);
        return -1;
    }  
    return bytes_written;
}

static struct nk_fs_int fatfs_inter = {
    .stat_path = fatfs_stat_path,
    .create_file = fatfs_create_file,
    .create_dir = fatfs_create_dir,
    .exists = fatfs_exists,
    .remove = fatfs_remove,
    .open_file = fatfs_open,
    .stat = fatfs_stat,
    .trunc_file = fatfs_truncate,
    .close_file = fatfs_close,
    .read_file = fatfs_read,
    .write_file = fatfs_write,
};

static void fatfs_test(struct fatfs_state *s){
    DEBUG("---STARTING FATFS TEST!---\n");


    DEBUG("---Creating file /temp.txt---\n");
    void* file = fatfs_create_file(s, "/tmp3.txt");

    DEBUG("---Writing to File---\n");
    char* buf = "Hello World!\n";
    fatfs_write(s, file, buf, 0, strlen(buf));
    DEBUG("---Closing file---\n");
    fatfs_close(s, file);


    DEBUG("---Checking if file exists---\n");
    int exists = fatfs_exists(s, "/tmp3.txt");
    if (!exists){
        ERROR("fatfs_test: file does not exist\n");
        return;
    }

    DEBUG("---Reopening file---\n");
    file = fatfs_open(s, "/tmp3.txt");

    DEBUG("---Reading from file---\n");
    char* buf2 = malloc(strlen(buf));
    fatfs_read(s, file, buf2, 0, strlen(buf));
    DEBUG("buf2 = %s\n", buf2);
    int c = memcmp(buf, buf2, strlen(buf));
    if (c != 0){
        ERROR("fatfs_test: write != read\n");
        return;
    }

    DEBUG("---Deleting file---\n");
    fatfs_remove(s, "/tmp3.txt");  

    DEBUG("---Checking to see if it is deleted---\n");  
    exists = fatfs_exists(s, "/tmp3.txt");
    if (exists){
        ERROR("fatfs_test: file still exists\n");
        return;
    }

    DEBUG("---Creating directory /test---\n");
    fatfs_create_dir(s, "/test");

    DEBUG("---Checking if directory exists---\n");
    exists = fatfs_exists(s, "/test");

    if (!exists){
        ERROR("fatfs_test: directory does not exist\n");
        return;
    }

    DEBUG("---Creating file /test/test.txt---\n");
    file = fatfs_create_file(s, "/test/test.txt");

    DEBUG("---Writing to file---\n");
    buf = "/test/test.txt: Hello World!\n";
    fatfs_write(s, file, buf, 0, strlen(buf));

    DEBUG("---Closing file---\n");
    fatfs_close(s, file);

    DEBUG("---Opening file---\n");
    file = fatfs_open(s, "/test/test.txt");

    DEBUG("---Reading from file---\n");
    buf2 = malloc(strlen(buf));
    fatfs_read(s, file, buf2, 0, strlen(buf));
    DEBUG("buf2 = %s\n", buf2);
    c = memcmp(buf, buf2, strlen(buf));
    if (c != 0){
        ERROR("fatfs_test: write != read\n");
        return;
    }

    DEBUG("---Deleting file---\n");
    fatfs_remove(s, "/test/test.txt");

    DEBUG("---Deleting directory---\n");
    fatfs_remove(s, "/test");

    DEBUG("---Checking if directory exists---\n");
    exists = fatfs_exists(s, "/test");

    if (exists){
        ERROR("fatfs_test: directory still exists\n");
        return;
    }

    DEBUG("---Checking if file exists---\n");
    exists = fatfs_exists(s, "/test/test.txt");

    if (exists){
        ERROR("fatfs_test: file still exists\n");
        return;
    }

    DEBUG("---PASS!---\n");
}


// Copied from fat32.c
int nk_fs_fatfs_attach(char *devname, char *fsname, int readonly)
{
    struct nk_block_dev *dev = nk_block_dev_find(devname);
    uint64_t flags = readonly ? NK_FS_READONLY : 0;
    
    if (!dev) { 
        ERROR("Cannot find device %s\n",devname);
        return -1;
    }
    
    struct fatfs_state *s = malloc(sizeof(*s));  
    
    if (!s) { 
        ERROR("Cannot allocate space for fs %s\n", fsname);
        return -1;
    }
    
    memset(s,0,sizeof(*s));
    
    s->dev = dev;
    
    if (nk_block_dev_get_characteristics(dev,&s->chars)) { 
        ERROR("Cannot get characteristics of device %s\n", devname);
        free(s);
        return -1;
    }

    DEBUG("Device %s has block size %lu and numblocks %lu (fatfs)\n",dev->dev.name, s->chars.block_size, s->chars.num_blocks);
    

    s->fatfs = malloc(sizeof(FATFS));
    if (!s->fatfs) { 
        ERROR("Cannot allocate space for fatfs %s\n", fsname);
        free(s);
        return -1;
    }


    // Associate the new mounted volume (and state) with 1 of the 3 possible physical drives
    // Note: this can be increased up to 10 by changing the FF_VOLUMES macro in ffconf.h
    // and then changing the logic below. Also, each physical drive is mapped to a logical drive
    char* d_num;
    if (fatfs_global_state[0] == NULL){
        d_num = "0:";
        fatfs_global_state[0] = s;
    } else if (fatfs_global_state[1] == NULL){
        d_num = "1:";
        fatfs_global_state[1] = s;
    } else if (fatfs_global_state[2] == NULL){
        d_num = "2:";
        fatfs_global_state[2] = s;
    } else {
        ERROR("fatfs_attach: too many filesystems\n");
        return -1;
    }

    DEBUG("mounting to d_num %s\n", d_num);

    FRESULT res = f_mount(s->fatfs, d_num, 1);
    if (res != FR_OK) {
        ERROR("Unable to mount filesystem %s with error %d\n", fsname, res);
        free(s);
        return -1;
    }

    s->fs = nk_fs_register(fsname, flags, &fatfs_inter, s);
    if (!s->fs) { 
        ERROR("Unable to register filesystem %s\n", fsname);
        free(s);
        return -1;
    }

    INFO("filesystem %s on device %s is attached (%s)\n", fsname, devname, readonly ?  "readonly" : "read/write");

#if 0
    // basic tests of the filesystem
    fatfs_test(s);
#endif    

    return 0;
}

int nk_fs_fatfs_detach(char *fsname)
{
    struct nk_fs *fs = nk_fs_find(fsname);

    // clear the global state when the filesystem is detached
    for (int i = 0; i < 3; i++){
        if (fatfs_global_state[i] != NULL && ((fatfs_state*)fatfs_global_state[i])->fs == fs){
            free(fatfs_global_state[i]);
            fatfs_global_state[i] = NULL;
            return 0;
        }
    }
    if (!fs) {
        return -1;
    } else {
        return nk_fs_unregister(fs);
    }
}