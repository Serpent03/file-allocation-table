#include <stdio.h>
#include <stdlib.h>
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
    u32 fat_size_bytes = f->bpb.bytes_per_sector * f->bpb.sectors_per_fat;
    f->fat1 = (u8*)malloc(fat_size_bytes * sizeof(u8));
    f->fat2 = (u8*)malloc(fat_size_bytes * sizeof(u8));
    fseek(fptr, f->fat1_start_sector * f->bpb.bytes_per_sector, SEEK_SET);
    fread(f->fat1, fat_size_bytes, 1, fptr);
    fread(f->fat2, fat_size_bytes, 1, fptr);

    fclose(fptr);
}

int main() {
    fat12 f = {};
    init_fat12(&f, DRIVE);
    return 0;
}
