// #include "sd.h"
// #include "uart.h"
// #include <string.h>
// // 通用SD命令

// #define SDHCI_BASE 0xFE340000 // 树莓派 4 的 SDHCI 控制器基地址

// // SDHCI 寄存器
// #define SDHCI_ARG2 (*(volatile uint32_t *)(SDHCI_BASE + 0x00))
// #define SDHCI_BLKSIZECNT (*(volatile uint32_t *)(SDHCI_BASE + 0x04))
// #define SDHCI_ARG1 (*(volatile uint32_t *)(SDHCI_BASE + 0x08))
// #define SDHCI_CMDTM (*(volatile uint32_t *)(SDHCI_BASE + 0x0C))
// #define SDHCI_RESP0 (*(volatile uint32_t *)(SDHCI_BASE + 0x10))
// #define SDHCI_RESP1 (*(volatile uint32_t *)(SDHCI_BASE + 0x14))
// #define SDHCI_RESP2 (*(volatile uint32_t *)(SDHCI_BASE + 0x18))
// #define SDHCI_RESP3 (*(volatile uint32_t *)(SDHCI_BASE + 0x1C))
// #define SDHCI_DATA (*(volatile uint32_t *)(SDHCI_BASE + 0x20))
// #define SDHCI_STATUS (*(volatile uint32_t *)(SDHCI_BASE + 0x24))
// #define SDHCI_CONTROL0 (*(volatile uint32_t *)(SDHCI_BASE + 0x28))
// #define SDHCI_CONTROL1 (*(volatile uint32_t *)(SDHCI_BASE + 0x2C))
// #define SDHCI_INTERRUPT (*(volatile uint32_t *)(SDHCI_BASE + 0x30))
// #define SDHCI_INTMASK (*(volatile uint32_t *)(SDHCI_BASE + 0x34))

// // SD 卡命令
// #define CMD_GO_IDLE_STATE 0    // CMD0  复位 SD 卡
// #define CMD_SEND_IF_COND 8     // CMD8  检查 SD 卡电压范围
// #define CMD_APP_CMD 55         // CMD55  告诉 SD 卡接下来要发 ACMD
// #define CMD_SD_SEND_OP_COND 41 // ACMD41  查询 SD 卡是否初始化完成
// #define CMD_READ_SINGLE 17     // CMD17  读取单个扇区

// #define SD_CARD_BLOCK_SIZE 512 // 一个扇区的大小
// // 等待控制器准备好
// static void sd_wait()
// {
//     while (SDHCI_STATUS & (1 << 0))
//     {
//     } // 等待 BUSY 位清零
// }

// // 发送 SD 卡命令
// static void sd_send_command(uint32_t cmd, uint32_t arg)
// {
//     sd_wait();
//     SDHCI_ARG1 = arg;
//     SDHCI_CMDTM = (cmd & 0x3F) | (1 << 31) | (1 << 21) | (1 << 20); // 启动命令
// }

// // 读取 SD 卡响应
// static uint32_t sd_get_response()
// {
//     sd_wait();
//     return SDHCI_RESP0;
// }
// // 初始化SD卡

// int sd_init()
// {
//     // 1. 复位 SD 卡（CMD0）
//     sd_send_command(CMD_GO_IDLE_STATE, 0);
//     if (sd_get_response() != 0x01)
//         return -1; // 检查 SD 卡是否复位成功

//     // 2. 检查 SD 卡电压范围（CMD8）
//     sd_send_command(CMD_SEND_IF_COND, 0x1AA);
//     uint32_t resp = sd_get_response();
//     if ((resp & 0xFFF) != 0x1AA)
//         return -1; // SD 卡不支持 3.3V 电压

//     // 3. 等待 SD 卡准备好（ACMD41）
//     do
//     {
//         sd_send_command(CMD_APP_CMD, 0);
//         sd_send_command(CMD_SD_SEND_OP_COND, 0x40000000); // HCS = 1，支持 SDHC
//         resp = sd_get_response();
//     } while (!(resp & (1 << 31))); // 等待 READY 位变 1

//     return 0; // 初始化成功
// }

// // 读取MBR
// // MBR 结构体定义
// typedef struct MBR
// {
//     uint8_t boot_code[446];      // 引导代码
//     uint8_t partition_table[64]; // 分区表
//     uint16_t boot_signature;     // 引导签名
// } __attribute__((packed)) MBR;

// // 分区表项结构
// typedef struct MBRPartitionEntry
// {
//     uint8_t boot_flag;        // 启动标志，0x80 表示可启动分区
//     uint8_t start_head;       // 起始磁头
//     uint8_t start_sector_cyl; // 起始扇区/柱面
//     uint8_t partition_type;   // 分区类型，例如 0x0B 表示 FAT32
//     uint8_t end_head;         // 结束磁头
//     uint8_t end_sector_cyl;   // 结束扇区/柱面
//     uint32_t start_lba;       // 分区的起始逻辑块地址
//     uint32_t num_sectors;     // 分区大小（以扇区为单位）
// } __attribute__((packed)) MBRPartitionEntry;

// // 读取BPB
// // BPB（BIOS Parameter Block）结构体
// typedef struct BPB
// {
//     uint8_t jump[3];              // 引导跳转指令
//     uint8_t oem_name[8];          // OEM 名称
//     uint16_t bytes_per_sector;    // 每扇区字节数
//     uint8_t sectors_per_cluster;  // 每簇扇区数
//     uint16_t reserved_sectors;    // 保留扇区数
//     uint8_t num_fats;             // FAT 数量
//     uint16_t root_entry_count;    // 根目录最大文件数（仅用于 FAT12/FAT16）
//     uint16_t total_sectors;       // 总扇区数（如果是 FAT12/FAT16，则最多可以存储 32MB）
//     uint8_t media_type;           // 媒体类型
//     uint16_t fat_size;            // FAT 区域的扇区数（FAT12/FAT16）
//     uint16_t sectors_per_track;   // 每磁道扇区数
//     uint16_t num_heads;           // 磁头数
//     uint32_t hidden_sectors;      // 隐藏扇区数（通常为 MBR 前的扇区数）
//     uint32_t total_sectors_large; // 总扇区数（大于 32MB 的分区）

//     // FAT32 专用
//     uint32_t fat_size_32;        // FAT32 区域的扇区数
//     uint16_t ext_flags;          // 扩展标志
//     uint16_t fs_version;         // 文件系统版本
//     uint32_t root_cluster;       // 根目录起始簇号
//     uint16_t fs_info;            // FS 信息区起始扇区
//     uint16_t backup_boot_sector; // 备份引导扇区
//     uint8_t reserved[12];        // 保留字段
//     uint8_t drive_number;        // 驱动器编号
//     uint8_t reserved2;           // 保留
//     uint8_t boot_signature;      // 启动签名
//     uint32_t volume_id;          // 卷 ID
//     uint8_t volume_label[11];    // 卷标
//     uint8_t fs_type[8];          // 文件系统类型
// } __attribute__((packed)) BPB;
// // 读取根目录
// typedef struct DirectoryEntry
// {
//     char name[11];    // 8+3 文件名（未使用部分填充空格）
//     uint8_t attr;     // 文件属性（是否是目录、隐藏文件等）
//     uint8_t reserved; // 保留字段
//     uint8_t create_time_tenth;
//     uint16_t create_time;
//     uint16_t create_date;
//     uint16_t last_access_date;
//     uint16_t cluster_high; // 文件起始簇号（高 16 位）
//     uint16_t mod_time;
//     uint16_t mod_date;
//     uint16_t cluster_low; // 文件起始簇号（低 16 位）
//     uint32_t size;        // 文件大小（字节）
// } __attribute__((packed)) DirectoryEntry;

// // 读取某个扇区
// int sd_read_block(uint32_t lba, uint8_t *buffer)
// {
//     // 发送读取命令
//     sd_send_command(CMD_READ_SINGLE, lba);
//     if (sd_get_response() != 0x00)
//         return -1;

//     // 读取 512 字节数据
//     for (int i = 0; i < 128; i++)
//     {
//         ((uint32_t *)buffer)[i] = SDHCI_DATA;
//     }

//     return 0; // 读取成功
// }

// uint32_t get_next_cluster(uint32_t current_cluster, uint32_t Partition_LBA, uint16_t reserved_sectors)
// {
//     // FAT表起始LBA = 分区起始LBA + 保留扇区数
//     uint32_t fat_start_lba = Partition_LBA + reserved_sectors;

//     // 计算FAT表中的字节偏移量（FAT32每个簇条目占4字节）
//     uint32_t fat_offset = current_cluster * 4;

//     // 计算对应的扇区地址和扇区内偏移
//     uint32_t fat_sector = fat_start_lba + (fat_offset / 512); // 每个扇区512字节
//     uint32_t entry_offset = fat_offset % 512;

//     // 读取FAT表扇区
//     uint8_t fat_buffer[512];
//     if (sd_read_block(fat_sector, fat_buffer) == -1)
//     {
//         uart_printf("读取FAT表失败");
//         return 0x0FFFFFFF; // 返回错误标记
//     }

//     // 从缓冲区提取下一簇号（注意小端序处理）
//     uint32_t next_cluster = *(uint32_t *)(fat_buffer + entry_offset);

//     // FAT32高4位是保留位，需要掩码
//     next_cluster &= 0x0FFFFFFF;

//     return next_cluster;
// }

// // 查找文件
// int sd_readlist()
// {
//     // 初始化
//     if (sd_init() == -1)
//     {
//         /* code */
//         uart_printf("SD卡初始化失败");
//         return -1;
//     }

//     // 读取MBR扇区
//     uint8_t mbr_buffer[512];
//     if (sd_read_block(0, mbr_buffer) == -1)
//     {
//         uart_printf("读取失败");
//         return -1;
//     }
//     for (int i = 0; i < 4; i++)
//     {
//         MBRPartitionEntry *partition = (MBRPartitionEntry *)(mbr_buffer + 0x1BE + i * 16); // 直接从mbr的地址基础上通过偏移地址取出来分区表地址。mbr_buffer + 0x1BE就是分区表首地址。
//         uint32_t Partition_LBA = partition->start_lba;                                     // FAT32 分区起始 LBA
//         if (partition->partition_type != 0x0B && partition->partition_type != 0x0C)
//         {
//             uart_printf("非FAT32分区\n");
//         }
//         else
//         {
//             uint8_t bpb_buffer[512];
//             if (sd_read_block(Partition_LBA, bpb_buffer) == -1)
//             {
//                 uart_printf("读取失败");
//             }
//             BPB *bpb = (BPB *)(bpb_buffer);
//             uint32_t root_addr = bpb->root_cluster;
//             uint8_t sectors_per_cluster = bpb->sectors_per_cluster;
//             uint16_t reserved_sectors = bpb->reserved_sectors;
//             uint32_t fat_size_32 = bpb->fat_size_32;
//             uint8_t num_fats = bpb->num_fats;
//             uint32_t data_start_lba = Partition_LBA + (uint32_t)reserved_sectors + (num_fats * fat_size_32);
//             if (root_addr < 2)
//             {
//                 uart_printf("错误: root_cluster 不能小于 2");
//                 return -1;
//             }
//             uint32_t root_directory_lba = data_start_lba + (root_addr - 2) * sectors_per_cluster;
//             /*根目录读取时仅读取第一个扇区，可能不完整，特别是当根目录大小超过一个扇区时。*/
//             /*没搞清楚簇和根目录展示的关系*/
//             uint8_t root_dir_buffer[512]; // 一个扇区大小
//             uint32_t current_cluster = root_addr;
//             do
//             {
//                 // 读取当前簇的所有扇区
//                 for (int s = 0; s < sectors_per_cluster; s++)
//                 {
//                     uint32_t lba = data_start_lba + (current_cluster - 2) * sectors_per_cluster + s;
//                     if (sd_read_block(lba, root_dir_buffer) == -1)
//                     {
//                         uart_printf("读取根目录失败");
//                         return -1;
//                     }
//                     // 处理目录项
//                     DirectoryEntry *entries = (DirectoryEntry *)root_dir_buffer;
//                     for (int j = 0; j < 16; j++)
//                     { // 512 / 32 = 16 个目录项
//                         if (entries[j].name[0] == 0x00)
//                         {
//                             break; // 遇到空条目，说明没有更多文件了
//                         }

//                         if (entries[j].name[0] == 0xE5)
//                         {
//                             continue; // 该目录项已删除，跳过
//                         }

//                         char name[9], ext[4];
//                         memcpy(name, entries[j].name, 8);
//                         name[8] = '\0';
//                         memcpy(ext, entries[j].name + 8, 3);
//                         ext[3] = '\0';
//                         uart_printf("%s.%s", name, ext);
//                     }
//                 }
//                 // 查找FAT表获取下一簇号
//                 // 关键修改：获取下一簇号
//                 uint32_t next_cluster = get_next_cluster(current_cluster, Partition_LBA, reserved_sectors);

//                 // 错误处理
//                 if (next_cluster == 0x0FFFFFFF)
//                 {
//                     uart_printf("FAT表读取错误\n");
//                     return -1;
//                 }

//                 // 更新当前簇号
//                 current_cluster = next_cluster;
//             } while (current_cluster < 0x0FFFFFF7);
//         }
//     }
// }