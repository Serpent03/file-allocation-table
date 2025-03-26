/*
 * "DRIVER" FILES FOR FAT12 DRIVE
 * THIS FILE PROVIDES AN "API" BETWEEN THE DRIVE AND THE USER
 */

#pragma once

#include "common.h"

// REF 1: https://www.sqlpassion.at/archive/2022/03/03/reading-files-from-a-fat12-partition/
// REF 2: http://www.c-jump.com/CIS24/Slides/FAT/lecture.html

#define DRIVE "drive.iso"

/**
 * @brief: fields marked with __ignore are irrelevant to
 * reading/writing to the FAT12 disk. We also don't need to 
 * consider heads/tracks/cylinders, as we're operating on a digital
 * file instead of a real floppy.
 */
typedef struct __bpb {
    u8  __ignore_0[11];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  num_fat;
    u16 max_rootdir_entries;
    u16 total_sector_count;
    u8  __ignore_1;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 num_heads;
    u32 __ignore_2;
    u32 total_sectors;
    u16 __ignore_3;
    u8  boot_signature;
    u32 volume_id;
    u8  volume_label[11];
    u8  file_system_type[8];
}__attribute__((packed)) bpb;

/**
 * @brief: struct containing data on a single
 * ROOT_DIR entry. There can be a maximum of 224
 * such entries in a FAT12 drive(by constraint
 * of design).
 */
typedef struct __root_dir_entry {
    u8  file_name[8];
    u8  extension[3];
    u8  attributes;
    u16 reserved;
    u16 creation_time;
    u16 creation_date;
    u16 last_access_date;
    u16 __ignore_0;
    u16 last_write_time;
    u16 last_write_date;
    u16 first_logical_cluster;
    u32 file_size;
}__attribute__((packed)) root_dir_entry;

/**
 * @brief: fat12 structure that we will actually use.
 * root_dir is a table of 32 byte long entries(with a max
 * of such entries). We'll cast the pointer to a struct
 * representing the root directory area.
 */
typedef struct __fat12 {
    bpb bpb;
    u32 fat1_start_sector;
    u32 fat_size_bytes;
    u8  *fat1;
    u8  *fat2;
    u32 root_dir_start_sector;
    u32 root_dir_size_bytes;
    u8  *root_dir;
    u32 data_area_start_sector;
} fat12;

typedef enum {
    CELL_TYPE_NON_USE = 0xFF0,
    CELL_TYPE_END     = 0xFFF,
} FAT_CELL_TYPE;

/**
 * @brief Initialize a FAT12 handle F by 
 * opening a file FILE.
 */
void init_fat12(fat12 *f, char *file);

/**
 * @brief Get a rootdir entry handle ENTRY if
 * file FILE exists inside the FAT12 directory
 * handle F
 * @return NULL if file does not exist, else a handle
 */
root_dir_entry *get_rootdir_entry(fat12 *f, char *file);

/**
 * @brief Get the size of a file FILE, residing
 * in the FAT12 drive with handle F.
 * @return u32: filesize in bytes
 */
i32 get_file_size(fat12 *f, char *file);

/**
 * @brief Read a file FILE from a FAT12 drive DRIVE,
 * with handle F, into buffer BUFFER.
 * @param buffer Allocates a new buffer
 * if buffer is NULL, else fills in that buffer.
 * @return u32: number of bytes read
 */
i32 read_file(fat12 *f, char *file, u8 **buffer, char *drive);



