#include "uart.h"
#include "mailbox.h"
#include "framebuffer.h"
#include "bluetooth.h"
// 简单延时函数
void delay(int count)
{
    while (count--)
    {
        asm volatile("nop");
    }
}

// Event callback function
void handle_bluetooth_event(uint8_t event_code, uint8_t *data, uint16_t length)
{
    uart_printf("Received BT event: 0x%02X, length: %d\r\n", event_code, length);

    // Handle events based on event code
    switch (event_code)
    {
    case HCI_EVENT_COMMAND_COMPLETE:
        uart_printf("Command complete event\r\n");
        break;

    case HCI_EVENT_LE_META:
        if (length > 0)
        {
            switch (data[0])
            {
            case HCI_SUBEVENT_LE_ADVERTISING_REPORT:
                uart_printf("Found device in advertising report\r\n");
                // Parse and print device info
                if (length >= 9)
                {
                    uart_printf("  Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                                data[7], data[8], data[9], data[10], data[11], data[12]);
                }
                break;
            }
        }
        break;
    }
}

// Kernel entry point
// Kernel entry point
void kernel_main(void)
{
    // Initialize UART
    uart_init();

    // 请求帧缓冲区，自动检测尺寸
    mailbox_request_fb(0, 0);
    // 填充白色背景
    fill_screen(COLOR_WHITE);

    // 文本位置
    int cursor_x = 10;
    int cursor_y = 10;
    int char_width = 8;        // 假设单个字符宽度为8个像素
    int char_height = 16;      // 假设单个字符高度为16个像素
    int max_width = 1920 - 20; // 屏幕边距
    int I = 0;                 // 闪烁标志
    // 欢迎消息
    uart_puts("UART Terminal started. Type to see on screen.\n");
    draw_string_scaled(cursor_x, cursor_y, "UART Terminal started. Type to see on screen.",
                       COLOR_BLACK, 1, max_width);
    cursor_y += char_height;
    bt_init_hardware();

    // Load firmware - usually would read from SD card
    // For this example, we'll assume firmware is already loaded
    firmware_status = BT_FIRMWARE_LOADED;

    // Register event callback
    bt_register_event_callback(handle_bluetooth_event);

    // Initialize Bluetooth stack
    bt_initialize();

    // Set custom device name
    bt_set_name("RPi4 Baremetal BT");

    // Start advertising so other devices can find us
    bt_start_advertising(1);

    // 主循环
    while (1)
    {
        if (I == 1)
        {
            draw_char_scaled(cursor_x, cursor_y, ' ', COLOR_WHITE, 1);
            I = 0;
        }
        else
        {
            draw_char_scaled(cursor_x, cursor_y, ' ', COLOR_BLACK, 1);
            I = 1;
        }

        // 检查UART是否有数据可读
        int c = uart_read_nonblock();
        if (c != -1)
        {
            // 处理蓝牙事件
            bt_process();

            // 处理回车和退格
            if (c == '\n')
            {
                cursor_x = 10;
                cursor_y += char_height;
            }
            if (c == '\b')
            {
                if (cursor_x == 10)
                {
                    cursor_x = max_width;
                    cursor_y -= char_height;
                }

                cursor_x -= char_width;
                draw_char_scaled(cursor_x, cursor_y, ' ', COLOR_WHITE, 1);
            }
            else
            {
                draw_char_scaled(cursor_x, cursor_y, c, COLOR_BLACK, 1);
                cursor_x += char_width;
            }
            uart_puts("Received");
        }
        else
        {
            // 可以添加简单延时避免CPU满负荷运行
            for (volatile int i = 0; i < 1000; i++)
            {
                asm volatile("nop");
            }
        }

        // 可以添加简单延时避免CPU满负荷运行
        for (volatile int i = 0; i < 1000; i++)
        {
            asm volatile("nop");
        }
    }
}