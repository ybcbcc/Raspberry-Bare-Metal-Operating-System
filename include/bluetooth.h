#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

// Bluetooth constants
enum
{
    // OGF (OpCode Group Field) values
    OGF_HOST_CONTROL = 0x03,
    OGF_LE_CONTROL = 0x08,
    OGF_VENDOR = 0x3f,

    // Command codes
    COMMAND_SET_BDADDR = 0x01,
    COMMAND_RESET_CHIP = 0x03,
    COMMAND_SET_BAUD = 0x18,
    COMMAND_LOAD_FIRMWARE = 0x2e,

    // Packet types
    HCI_COMMAND_PKT = 0x01,
    HCI_ACL_PKT = 0x02,
    HCI_EVENT_PKT = 0x04,

    // Event codes
    COMMAND_COMPLETE_CODE = 0x0e,
    CONNECT_COMPLETE_CODE = 0x0f,

    // LE constants
    LL_SCAN_ACTIVE = 0x01,
    LL_ADV_NONCONN_IND = 0x03
};

#endif // BLUETOOTH_H