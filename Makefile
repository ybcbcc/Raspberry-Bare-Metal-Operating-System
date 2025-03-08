# 树莓派4B裸机操作系统 Makefile - Windows 环境
# 交叉编译工具链路径（使用短路径，避免空格问题）
TOOLCHAIN_PATH = C:\PROGRA~2\ARMGNU~1\14EFD8~1.2RE\bin
CC = $(TOOLCHAIN_PATH)/aarch64-none-elf-gcc
LD = $(TOOLCHAIN_PATH)/aarch64-none-elf-ld
OBJCOPY = $(TOOLCHAIN_PATH)/aarch64-none-elf-objcopy
OBJDUMP = $(TOOLCHAIN_PATH)/aarch64-none-elf-objdump.exe
# 输出文件名
KERNEL = kernel
BUILD_DIR = build
# 编译标志
CFLAGS = -mcpu=cortex-a72 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra -nostartfiles
LDFLAGS = -T linker.ld -nostdlib
# 源文件和头文件目录
SRC_DIR = src
INCLUDE_DIR = include
ASM_DIR = asm
# 源文件
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(ASM_DIR)/*.S)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
# 目标文件
OBJ_FILES = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_SOURCES:$(ASM_DIR)/%.S=$(BUILD_DIR)/%.o)
# 默认目标
all: $(BUILD_DIR)/$(KERNEL).img

# 创建构建目录（修正 mkdir 语法）
$(BUILD_DIR):
	mkdir $(BUILD_DIR) 2>nul

# 编译C源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 编译汇编源文件
$(BUILD_DIR)/%.o: $(ASM_DIR)/%.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 链接
$(BUILD_DIR)/$(KERNEL).elf: $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^

# 创建二进制镜像
$(BUILD_DIR)/$(KERNEL).img: $(BUILD_DIR)/$(KERNEL).elf
	$(OBJCOPY) -O binary $< $@

# 反汇编，用于调试
$(BUILD_DIR)/$(KERNEL).list: $(BUILD_DIR)/$(KERNEL).elf
	$(OBJDUMP) -D $< > $@

# 清理构建文件（修正 Windows 下的 rm 语法）
clean:
	del /s /q $(BUILD_DIR)\* 2>nul && rmdir $(BUILD_DIR) 2>nul

# 运行 qemu（可选）
qemu:
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD_DIR)/$(KERNEL).img -serial stdio

# 将镜像复制到 SD 卡（假设 SD 卡挂载为 E:）
deploy:
	copy $(BUILD_DIR)\$(KERNEL).img E:\

# 确保这些目标不与文件名冲突
.PHONY: all clean qemu deploy