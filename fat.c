/*
  This system purely focuses on the lower level FAT12 drivers,
  and pays no homage to virtualized file systems above it - that
  may call it. In actuality, the process should be somewhat
  like this:

  USER(req I/O) -> virtual FILE SYSTEM(check actual physical fs) ->
  kernel(engage driver) -> driver(actually do I/O)

  The module we are at right now sits on the driver stack; i.e,
  the lowest level in the entire stack, where we are more worried
  about getting and sending data rather than what that data is all
  about.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fat.h"

#define MIN(a,b) (a > b ? b : a)

FILE *fptr;

void init_fat12(fat12 *f, char *file) {
    if ((!f) || (!file)) return;

    // read the data into the bpb
    fptr = fopen(file, "rb");
    if (!fptr) return;
    fread(&f->bpb, sizeof(bpb), 1, fptr);

    /* Crucial metadata */
    f->fat1_start_sector = 1;
    f->root_dir_start_sector = f->fat1_start_sector + (f->bpb.sectors_per_fat * 2);
    f->data_area_start_sector = f->root_dir_start_sector + (f->bpb.max_rootdir_entries * 32 / f->bpb.bytes_per_sector);
    
    /* Allocate the FAT1 and FAT2 tables,
     * and read the data from the disk. */
    f->fat_size_bytes = f->bpb.bytes_per_sector * f->bpb.sectors_per_fat;
    f->fat1 = (u8*)malloc(f->fat_size_bytes * sizeof(u8));
    f->fat2 = (u8*)malloc(f->fat_size_bytes * sizeof(u8));
    fseek(fptr, f->fat1_start_sector * f->bpb.bytes_per_sector, SEEK_SET);
    fread(f->fat1, f->fat_size_bytes, 1, fptr);
    fread(f->fat2, f->fat_size_bytes, 1, fptr);

    f->root_dir_size_bytes = f->bpb.max_rootdir_entries * 32;
    f->root_dir = (u8*)malloc(f->root_dir_size_bytes * sizeof(u8));
    fread(f->root_dir, f->root_dir_size_bytes, 1, fptr);

    fclose(fptr);
}

bool file_name_cmp(const char *s1, const char *s2) {
    /* doesn't handle file names with size > 8 for now 
    OR extensions! 

    @todo: fix this shit fool*/
    for (u8 i = 0; i < 8; i++) {
        if (s1[i] == ' ') {
            continue;
        }
        else if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

root_dir_entry *get_rootdir_entry(fat12 *f, char *file) {
    root_dir_entry *entry;

    for (u32 i = 0; i < f->bpb.max_rootdir_entries; i++) {
        entry = (root_dir_entry*)(f->root_dir + (i*32));
        if (file_name_cmp((char*)entry->file_name, file)) {
          return entry;
        }
    }
    return NULL;
}

i32 get_file_size(fat12 *f, char *file) {
    /* read into the ROOT_DIR entries */
    root_dir_entry *entry = get_rootdir_entry(f, file);
    if (!entry) return -1;
    return entry->file_size;
}

i32 read_file(fat12 *f, char *file, u8 **buffer, char *drive) {
    /**
     * FAT12 entries are packed in the shape of 
     * yz Zx XY, where:
     * - first cluster: xyz
     * - second cluster: XYZ
     * cluster -> FAT translation:
     * (cluster * 1.5 + 1)

     * Billy G was **NOT** cooking with this one..
     */

    /* If file doesn't exist, then we will be unable
     * to get a handle on it. */
    root_dir_entry *entry = get_rootdir_entry(f, file);
    if (!entry) return -1;

    i32 file_size = entry->file_size;
    u32 current_cluster = entry->first_logical_cluster;
    u32 offset = 0;
    u32 data_area_offset = f->data_area_start_sector * f->bpb.bytes_per_sector;
    u8  fat_table_entry[3];

    u32 ptr = 0;
    u32 max_allowable_read = f->bpb.bytes_per_sector * f->bpb.sectors_per_cluster;
    u32 remaining_bytes = file_size;
    /* allocate space if the buffer points to a 
     * NULL address type */
    if (*buffer == NULL) {
        (*buffer) = (u8*)calloc(file_size, sizeof(u8));
    }

    fptr = fopen(drive, "rb");
    while (
      (current_cluster & 0xFF0) != CELL_TYPE_NON_USE &&
      current_cluster != CELL_TYPE_END
    ) {
        offset = (u32)((current_cluster * 1.5) - (current_cluster % 2 != 0));
        printf("cluster: %03x, offset: %d\n", current_cluster, offset);
        memcpy(fat_table_entry, &f->fat1[offset], 3);

        /* read the actual data before we jump to the next cluster. 
          clusters are logical groupings of bytes, so the process
          would go as follows:
            * offset into the data area using current cluster address
            * read bytes_per_sector * sectors_per_cluster number of bytes
              for each cluster we come across
            * keep track of remaining bytes
        */

        u32 disk_read_offset = data_area_offset + \
          (current_cluster - 2) * (f->bpb.bytes_per_sector * f->bpb.sectors_per_cluster);
        u32 size_to_read = MIN(max_allowable_read, remaining_bytes);
        fseek(fptr, disk_read_offset, SEEK_SET);
        fread((*(buffer) + ptr), 1, size_to_read, fptr);
        remaining_bytes -= size_to_read;

        /* THIS PIECE OF CODE IS ACCURSED. DO NOT TOUCH. GOD DAMN IT, BILLY G */
        /* fetch the next offset block in the data chain */
        if (current_cluster % 2 == 0) {
            current_cluster = (((u32)(fat_table_entry[1]) << 8) | (fat_table_entry[0])) & 0xFFF;
        } else {
            current_cluster = (((u32)(fat_table_entry[2]) << 4) | (fat_table_entry[1] >> 4)) & 0xFFF;
        }
        
    }
    fclose(fptr);
    return file_size;
}

i32 write_file(fat12 *f, char *file, u8 *buffer, u32 nbytes, char *drive) {
  /* check if the file exists before or not. we'll overrwrite it,
  if so, by deallocating the FAT for that file. */

  root_dir_entry *entry = get_rootdir_entry(f, file);
  // if (entry) delete_file();

  // fptr = fopen(drive, "wb");
  // fclose(fptr);

  /* write nbytes from the buffer into the drive, in
  accordance to FAT12 standards. This should allow,
  in theory, to perform all sorts of CRUD tasks to/fro
  the file system. */
  return nbytes;
}

int main() {
    fat12 f = {};
    init_fat12(&f, DRIVE);

    u8 *buf = NULL;
    i32 b_read = read_file(&f, "HELLO", &buf, DRIVE);
    printf("Bytes read: (%d)\nContents: %s\n", b_read, buf);

    // u8 *msg = "Hi there, written from the OS land!";
    // i32 b_write = write_file(&f, "BYE", msg, strlen(msg), DRIVE);

    free(buf);
    buf = NULL;
    b_read = read_file(&f, "BYE", &buf, DRIVE);
    printf("Contents: %s\n, bytes read: (%d)", buf, b_read);
    return 0;
}
