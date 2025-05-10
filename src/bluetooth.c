// #include "uart.h"

// #define BT_REG_ON 43 // 蓝牙芯片电源控制引脚
// #define BT_RESET_PIN 45

// bool bt_init(void)
// {
//     // 蓝牙串口初始化
//     uart_init(1, 14, 15, 2);

//     // 给蓝牙模块通电
//     gpio_set_function(BT_REG_ON, 1); // 配置 GPIO43 为输出
//     gpio_set_pull(BT_REG_ON, 0);     // 取消上拉/下拉
//     gpio_write(BT_REG_ON, 1);        // 置高，开启蓝牙芯片

//     // 硬件复位
//     gpio_set_output(BT_RESET_PIN); // 设置为输出模式
//     gpio_write(BT_RESET_PIN, 0);   // 拉低（复位）
//     delay(10);                     // 延迟 10ms
//     gpio_write(BT_RESET_PIN, 1);   // 拉高（解除复位）
//     delay(50);                     // 延迟 50ms，等待蓝牙芯片启动

//     // 发送命令 软件复位
//     uint8_t hci_reset[] = {
//         HCI_COMMAND_PKT,                 // 0x01
//         COMMAND_RESET_CHIP,              // 0x03
//         (OGF_HOST_CONTROL << 10) & 0xFF, // 0x0C（高字节）
//         0x00                             // 参数长度
//     };
//     for (size_t i = 0; i < sizeof(hci_reset); i++)
//     {
//         uart_putc(uart_num, hci_reset[i]);
//     }
//     // 读取 HCI 设备响应
//     uint8_t response[7];
//     for (size_t i = 0; i < sizeof(response); i++)
//     {
//         int c;
//         while ((c = uart_read_nonblock(0)) == -1)
//             ; // 等待数据
//         response[i] = (uint8_t)c;
//     }
// }
