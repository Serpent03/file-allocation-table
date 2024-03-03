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

typedef struct sector {
  u8 data[SECTOR_SIZE]; // 512 bytes
} sector;

typedef struct cluster {
  sector sectors[NUM_SECTORS_PER_CLUSTER]; // 2 sectors
} cluster;

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
  u16 table[NUM_CLUSTERS]; /** @todo Max table size based on clusters and sectors */
} allocation_table;

typedef struct root_dir {
  String fileName[11]; // 11-byte limit enforced by FAT12/16
  u16 fileSize; 
  cluster startCluster;
} root_dir;


// REF 3: https://www.hdd-tool.com/hdd-basic/what-is-fat-file-system.html
typedef struct FAT {
  // boot sector >> 512 bytes. this has all the metadata
  allocation_table table;
  root_dir root_dir;
  cluster clusters[NUM_CLUSTERS];
} FAT;






