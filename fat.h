/*
 * "DRIVER" FILES FOR FAT16 DRIVE
 * THIS FILE PROVIDES AN "API" BETWEEN THE DRIVE AND THE USER
 */

#pragma once

#include "common.h"

// REF 1: https://www.sqlpassion.at/archive/2022/03/03/reading-files-from-a-fat12-partition/
// REF 2: http://www.c-jump.com/CIS24/Slides/FAT/lecture.html

#define DRIVE "drive.bin"

/** @todo make a BPB(REF 1) through the use of assembly */

#define SECTOR_SIZE 512 // 512 byte sectors
#define NUM_SECTORS 64 // arbitrary: let's just go with 64 * 512B sectors.
#define NUM_SECTORS_PER_CLUSTER 2 // arbitrary: 2 sectors per cluster 
#define NUM_CLUSTERS (NUM_SECTORS / NUM_SECTORS_PER_CLUSTER) // this gives us about 33 clusters.

#define ROOT_DIR_NAME_LIMIT 64 // arbitrary: set a 64 byte limit for names


// REF 3: https://www.hdd-tool.com/hdd-basic/what-is-fat-file-system.html
typedef struct FAT {
  // boot sector >> 512 bytes
  // allocation_table >> NUM CLUSTERS * 16 bits (2 bytes) || why? because we want to address about NUM_CLUSTERS different clusters, up to a max number of 2^16 clusters.
  // root dir >> 
  // data cluster ()
} FAT;

typedef struct sector {} sector;

typedef struct cluster {} cluster;

/**
 * @brief File allocation table.
 * When fetching a file, we are given its first cluster number.
 * If that file is bigger than the cluster size(sector size * number of sectors in a cluster),
 * then we have to parse several clusters until we go to end of file.
 *
 * NEXT_CLUSTER_NUMBER = table[CURRENT_CLUSTER_NUMBER] 
 * If NEXT_CLUSTER_NUMBER  == 0xFFFFF .., then it is the end of file.
 * If NEXT_CLUSTER_NUMBER == 0x00 .., then the cluster is free and could be used for data.
 */
typedef struct allocation_table {
  u16 table[4096]; /** @todo Max table size based on clusters and sectors */
} allocation_table;

typedef struct root_dir {
  String fileName[200];
  u16 fileSize; 
  cluster startCluster;
} root_dir;

// DATA BLOCKS [4096], [4096], [4096], [4096]
// CLUSTERS [DATA BLOCKS], [DATA BLOCKS]
//






