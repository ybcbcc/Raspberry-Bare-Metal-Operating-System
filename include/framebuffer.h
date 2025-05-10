#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "mailbox.h"
#include <stdint.h>
#include "font8x8_basic.h"
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFFFF0000
#define COLOR_YELLOW 0xFF00FFFF
#define COLOR_CYAN 0xFFFFFF00
#define COLOR_MAGENTA 0xFFFF00FF
#define COLOR_BLACK 0xFF000000
#define COLOR_GRAY 0xFF808080

// Mailbox 缓冲区对齐 16 字节
typedef volatile unsigned int mailbox_buffer_t[36] __attribute__((aligned(16)));

// 声明外部 Mailbox 缓冲区
extern mailbox_buffer_t mailbox_fb;
int get_display_size(int *width, int *height);

int mailbox_request_fb(int width, int height);
int get_screen_width(void);
int get_screen_height(void);
void draw_pixel(int x, int y, unsigned int color);
void fill_screen(unsigned int color);
void draw_rect(int x, int y, int w, int h, unsigned int color);
uint32_t convert_color(uint32_t rgba);
void draw_char_scaled(int x, int y, char c, unsigned int color, float scale);
void draw_string_scaled(int x, int y, const char *str, unsigned int color, float scale, int max_width);
#endif // FRAMEBUFFER_H
