#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fat.h"

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
    /* doesn't handle file names with size > 8 for now */
    for (u8 i = 0; i < 8; i++) {
        printf("%c :: %c\n", s1[i], s2[i]);
        if (s1[i] == ' ') {
            continue;
        }
        else if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

i32 get_file_size(fat12 *f, char *file) {
    /* read into the ROOT_DIR entries */
    u32 file_size = -1;
    root_dir_entry *entry;
    bool is_same;

    for (u32 i = 0; i < f->bpb.max_rootdir_entries; i++) {
        entry = (root_dir_entry*)(f->root_dir + (i*32));
        is_same = file_name_cmp((char*)entry->file_name, file);
        printf("Cluster @: %d\n", entry->first_logical_cluster);
        if (is_same) {
            file_size = entry->file_size;
            break;
        }
    }
    return file_size;
}

i32 read_file(fat12 *f, char *file, u8 *buffer) {
    u32 bytes_read = -1;
    return bytes_read;
}

int main() {
    fat12 f = {};
    init_fat12(&f, DRIVE);
    i32 file_size = get_file_size(&f, "HELLO.txt");
    printf("File size: %d\n", file_size);
    return 0;
}
