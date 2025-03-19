#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdio.h>

static void sd_wait();
static void sd_send_command(uint32_t cmd, uint32_t arg);
static uint32_t sd_get_response();
int sd_init();
int sd_read_block(uint32_t lba, uint8_t *buffer);
void parse_mbr(uint8_t *buffer);
void read_bpb(uint32_t start_lba, uint8_t *buffer);
void read_root_directory();
uint32_t fat_read_next_cluster(uint32_t cluster);
void sd_readlist();
#endif