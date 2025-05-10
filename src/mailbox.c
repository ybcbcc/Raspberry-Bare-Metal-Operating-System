#include "mailbox.h"

void mailbox_write(unsigned int channel, unsigned int data)
{
    while (MAILBOX_STATUS & MAILBOX_FULL)
        ; // 等待Mailbox有空间
    MAILBOX_WRITE = (data & ~0xF) | (channel & 0xF);
}

unsigned int mailbox_read(unsigned int channel)
{
    unsigned int data;
    while (1)
    {
        while (MAILBOX_STATUS & MAILBOX_EMPTY)
            ; // 等待Mailbox有数据
        data = MAILBOX_READ;
        if ((data & 0xF) == channel)
            return data & ~0xF;
    }
}