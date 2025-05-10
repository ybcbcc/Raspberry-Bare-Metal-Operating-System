#include "framebuffer.h"

// Framebuffer
// Mailbox 缓冲区 (真正分配内存的地方)
volatile unsigned int __attribute__((aligned(16))) mailbox_fb[36];

// 全局变量存储当前屏幕尺寸
static int screen_width = 0;
static int screen_height = 0;

// 查询当前显示尺寸
int get_display_size(int *width, int *height)
{
    // 准备请求获取屏幕尺寸的消息
    volatile unsigned int __attribute__((aligned(16))) mbox[7];

    // 首先尝试获取实际物理显示尺寸
    mbox[0] = 7 * 4;      // 消息大小
    mbox[1] = 0;          // 这是一个请求
    mbox[2] = 0x00040003; // TAG: 获取物理显示尺寸
    mbox[3] = 8;          // 请求缓冲区大小
    mbox[4] = 8;          // 请求代码
    mbox[5] = 0;          // 用于存储宽度
    mbox[6] = 0;          // 用于存储高度

    mailbox_write(8, (uint32_t)(uintptr_t)mbox);

    // 检查响应
    if (mailbox_read(8) == 0 && mbox[1] == 0x80000000 && mbox[4] == 8)
    {
        *width = mbox[5];
        *height = mbox[6];

        if (*width > 0 && *height > 0)
        {
            return 1; // 成功
        }
    }

    // 如果上面失败，尝试获取虚拟显示尺寸
    mbox[0] = 7 * 4;
    mbox[1] = 0;
    mbox[2] = 0x00040004; // TAG: 获取虚拟显示尺寸
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 0;
    mbox[6] = 0;

    mailbox_write(8, (uint32_t)(uintptr_t)mbox);

    if (mailbox_read(8) == 0 && mbox[1] == 0x80000000 && mbox[4] == 8)
    {
        *width = mbox[5];
        *height = mbox[6];

        if (*width > 0 && *height > 0)
        {
            return 1; // 成功
        }
    }

    // 如果仍然失败，使用默认值并记录

    *width = 1920;
    *height = 1080;
    return 0; // 失败
}
int mailbox_request_fb(int width, int height)
{
    // 如果传入的尺寸为0，则尝试自动检测
    if (width == 0 || height == 0)
    {
        get_display_size(&width, &height);
    }

    // 保存尺寸到全局变量
    screen_width = width;
    screen_height = height;

    mailbox_fb[0] = 35 * 4; // 消息大小（字节数）
    mailbox_fb[1] = 0;      // 请求代码 (0 表示请求)

    mailbox_fb[2] = 0x00048003;
    mailbox_fb[3] = 8;
    mailbox_fb[4] = 8;
    mailbox_fb[5] = width;  // 物理宽度
    mailbox_fb[6] = height; // 物理高度

    mailbox_fb[7] = 0x00048004;
    mailbox_fb[8] = 8;
    mailbox_fb[9] = 8;
    mailbox_fb[10] = width;  // 虚拟宽度
    mailbox_fb[11] = height; // 虚拟高度

    mailbox_fb[12] = 0x00048005;
    mailbox_fb[13] = 4;
    mailbox_fb[14] = 4;
    mailbox_fb[15] = 32; // 深度 32 bits

    mailbox_fb[16] = 0x00048006;
    mailbox_fb[17] = 4;
    mailbox_fb[18] = 4;
    mailbox_fb[19] = 1; // 无换行

    mailbox_fb[20] = 0x00040001;
    mailbox_fb[21] = 8;
    mailbox_fb[22] = 8;
    mailbox_fb[23] = 0; // GPU分配的地址
    mailbox_fb[24] = 0; // GPU分配的大小

    mailbox_fb[25] = 0x0004800B;
    mailbox_fb[26] = 4;
    mailbox_fb[27] = 4;
    mailbox_fb[28] = 0; // 颜色顺序

    mailbox_fb[29] = 0x0004800C;
    mailbox_fb[30] = 4;
    mailbox_fb[31] = 4;
    mailbox_fb[32] = 0; // 透明度

    mailbox_fb[33] = 0; // 结束标志

    mailbox_write(8, (uint32_t)(uintptr_t)mailbox_fb);
    if (mailbox_read(8) != 0)
    {
        return mailbox_fb[23]; // 返回 Framebuffer 地址
    }
    return 0;
}

// 获取当前屏幕宽度
int get_screen_width(void)
{
    return screen_width;
}

// 获取当前屏幕高度
int get_screen_height(void)
{
    return screen_height;
}

// 颜色转换函数
uint32_t convert_color(uint32_t rgba)
{
    return ((rgba & 0xFF000000) |         // A (透明度，不变)
            ((rgba & 0x00FF0000) >> 16) | // R (红) 右移 16 位变 B
            (rgba & 0x0000FF00) |         // G (绿) 保持不变
            ((rgba & 0x000000FF) << 16)); // B (蓝) 左移 16 位变 R
}

// Framebuffer 绘图函数
void draw_pixel(int x, int y, unsigned int color)
{
    volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)(mailbox_fb[23] & 0x3FFFFFFF);

    // 检查像素是否在屏幕范围内
    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height)
    {
        fb[y * screen_width + x] = color;
    }
}

// 绘制矩形
void draw_rect(int x, int y, int w, int h, unsigned int color)
{
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            draw_pixel(x + j, y + i, color);
        }
    }
}

void fill_screen(unsigned int color)
{
    volatile uint32_t *fb = (volatile uint32_t *)(uintptr_t)(mailbox_fb[23] & 0x3FFFFFFF);
    // 获取 GPU 分配的 Framebuffer 地址
    if (!fb)
        return; // 确保 Framebuffer 有效

    for (int y = 0; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            fb[y * screen_width + x] = color;
        }
    }
}

// 绘制单个字符（可缩放）
void draw_char_scaled(int x, int y, char c, unsigned int color, float scale)
{
    if ((unsigned char)c > 127)
        return; // 确保字符在支持的范围内

    const uint8_t *glyph = (const uint8_t *)font8x8_basic[(uint8_t)c];
    int pixel_size = (int)scale; // 每个像素点的大小

    if (pixel_size < 1)
        pixel_size = 1; // 确保缩放比例至少为1

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (glyph[i] & (1 << j))
            { // 逐位检查是否需要绘制像素
                // 绘制放大后的像素块
                for (int sy = 0; sy < pixel_size; sy++)
                {
                    for (int sx = 0; sx < pixel_size; sx++)
                    {
                        draw_pixel(x + j * pixel_size + sx, y + i * pixel_size + sy, color);
                    }
                }
            }
        }
    }
}

// 绘制字符串（可缩放，支持自动换行）
void draw_string_scaled(int x, int y, const char *str, unsigned int color, float scale, int max_width)
{
    int cursor_x = x;
    int cursor_y = y;
    int char_width = 8 * (int)scale;   // 字符固定宽度为8像素 * 缩放比例
    int char_spacing = (int)scale;     // 字符间额外间距
    int line_height = 12 * (int)scale; // 计算缩放后的行高

    // 设置屏幕宽度和最大宽度
    int screen_width = 1920; // 屏幕宽度为1920

    // 如果未指定最大宽度，使用从当前位置到屏幕右边缘的宽度
    if (max_width <= 0)
    {
        max_width = screen_width - x;
    }

    while (*str)
    {
        // 处理换行符
        if (*str == '\n')
        {
            cursor_x = x;            // 回到起始x位置
            cursor_y += line_height; // 移动到下一行
            str++;
            continue;
        }

        // 检查当前字符是否会超出最大宽度
        if (cursor_x + char_width > x + max_width)
        {
            cursor_x = x;            // 回到起始x位置
            cursor_y += line_height; // 移动到下一行
        }

        // 检查当前位置是否已经超出屏幕右边缘
        if (cursor_x + char_width > screen_width)
        {
            cursor_x = x;            // 回到起始x位置
            cursor_y += line_height; // 移动到下一行
        }

        draw_char_scaled(cursor_x, cursor_y, *str, color, scale);
        cursor_x += char_width + char_spacing; // 移动到下一个字符位置
        str++;
    }
}