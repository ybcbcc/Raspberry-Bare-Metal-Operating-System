#ifndef MAILBOX_H
#define MAILBOX_H
// Mailbox
#define MAILBOX_BASE 0xFE00B880 // 树莓派4的Mailbox地址
#define MAILBOX_READ (*(volatile unsigned int *)(MAILBOX_BASE + 0x00))
#define MAILBOX_STATUS (*(volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE (*(volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_FULL 0x80000000  // Mailbox满
#define MAILBOX_EMPTY 0x40000000 // Mailbox空

void mailbox_write(unsigned int channel, unsigned int data);
unsigned int mailbox_read(unsigned int channel);

#endif // MAILBOX_H