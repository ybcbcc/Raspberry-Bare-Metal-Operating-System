#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "gpio.h"
#include "uart.h"

// HCI Packet types
#define HCI_COMMAND_PACKET 0x01
#define HCI_ACL_DATA_PACKET 0x02
#define HCI_SCO_DATA_PACKET 0x03
#define HCI_EVENT_PACKET 0x04

// HCI Commands - Opcode groups
#define OGF_LINK_CONTROL 0x01
#define OGF_LINK_POLICY 0x02
#define OGF_CONTROLLER 0x03
#define OGF_INFORMATIONAL 0x04
#define OGF_STATUS 0x05
#define OGF_TESTING 0x06
#define OGF_LE_CONTROLLER 0x08
#define OGF_VENDOR_SPECIFIC 0x3f

// Command opcodes
#define HCI_RESET 0x0c03
#define HCI_READ_LOCAL_VERSION 0x1001
#define HCI_READ_BD_ADDR 0x1009
#define HCI_WRITE_LOCAL_NAME 0x0c13
#define HCI_READ_LOCAL_NAME 0x0c14
#define HCI_SET_EVENT_MASK 0x0c01
#define HCI_READ_BUFFER_SIZE 0x1005
#define HCI_LE_SET_SCAN_PARAMETERS 0x200b
#define HCI_LE_SET_SCAN_ENABLE 0x200c
#define HCI_LE_CREATE_CONNECTION 0x200d
#define HCI_LE_SET_ADVERTISING_PARAMETERS 0x2006
#define HCI_LE_SET_ADVERTISING_DATA 0x2008
#define HCI_LE_SET_ADVERTISE_ENABLE 0x200a

// HCI Events
#define HCI_EVENT_COMMAND_COMPLETE 0x0e
#define HCI_EVENT_COMMAND_STATUS 0x0f
#define HCI_EVENT_LE_META 0x3e
#define HCI_EVENT_VENDOR_SPECIFIC 0xff

// LE Meta subevent codes
#define HCI_SUBEVENT_LE_CONNECTION_COMPLETE 0x01
#define HCI_SUBEVENT_LE_ADVERTISING_REPORT 0x02

// Bluetooth error codes
#define BT_SUCCESS 0x00
#define BT_ERROR_UNKNOWN_COMMAND 0x01
#define BT_ERROR_HARDWARE_FAILURE 0x03
#define BT_ERROR_INVALID_PARAMETERS 0x12

// GPIO functions
#define GPIO_FUNCTION_ALT0 0x04
#define GPIO_FUNCTION_ALT1 0x05
#define GPIO_FUNCTION_ALT2 0x06
#define GPIO_FUNCTION_ALT3 0x07
#define GPIO_FUNCTION_ALT4 0x03
#define GPIO_FUNCTION_ALT5 0x02

// GPIO pull types
#define GPIO_PULL_NONE 0x00
#define GPIO_PULL_UP 0x01
#define GPIO_PULL_DOWN 0x02

// Bluetooth GPIO pins for Raspberry Pi 4B
#define BT_ENABLE_PIN 43   // GPIO pin for Bluetooth enable
#define BT_UART_TX_PIN 14  // UART TX pin for Bluetooth
#define BT_UART_RX_PIN 15  // UART RX pin for Bluetooth
#define BT_UART_CTS_PIN 16 // UART CTS pin for Bluetooth
#define BT_UART_RTS_PIN 17 // UART RTS pin for Bluetooth

// Firmware status
typedef enum
{
    BT_FIRMWARE_NOT_LOADED,
    BT_FIRMWARE_LOADING,
    BT_FIRMWARE_LOADED,
    BT_FIRMWARE_ERROR
} bt_firmware_status_t;

// Connection status
typedef enum
{
    BT_STATE_OFF,
    BT_STATE_INITIALIZING,
    BT_STATE_READY,
    BT_STATE_SCANNING,
    BT_STATE_ADVERTISING,
    BT_STATE_CONNECTED,
    BT_STATE_ERROR
} bt_state_t;

// Bluetooth device information
typedef struct
{
    uint8_t addr[6];
    char name[248];
    uint8_t features[8];
} bt_device_info_t;

// Callback for HCI events
typedef void (*bt_event_callback_t)(uint8_t event_code, uint8_t *data, uint16_t length);

/**
 * Initialize Bluetooth hardware
 *
 * This function initializes the GPIO pins and UART interface
 * for Bluetooth communication on Raspberry Pi 4B
 *
 * @return 0 on success, error code otherwise
 */
int bt_init_hardware(void);

/**
 * Load Bluetooth firmware
 *
 * @param firmware Pointer to firmware data
 * @param size Size of firmware data
 * @return 0 on success, error code otherwise
 */
int bt_load_firmware(const uint8_t *firmware, size_t size);

/**
 * Initialize Bluetooth stack
 *
 * This initializes the Bluetooth controller and prepares it for use
 *
 * @return 0 on success, error code otherwise
 */
int bt_initialize(void);

/**
 * Send HCI command
 *
 * @param opcode Command opcode
 * @param params Command parameters
 * @param length Length of parameters
 * @return 0 on success, error code otherwise
 */
int bt_send_hci_command(uint16_t opcode, const uint8_t *params, uint8_t length);

/**
 * Register callback for HCI events
 *
 * @param callback Function to call when events are received
 */
void bt_register_event_callback(bt_event_callback_t callback);

/**
 * Process incoming HCI data
 *
 * This function should be called regularly to process any
 * incoming HCI data from the Bluetooth controller
 */
void bt_process(void);

/**
 * Set Bluetooth device name
 *
 * @param name Device name
 * @return 0 on success, error code otherwise
 */
int bt_set_name(const char *name);

/**
 * Start Bluetooth advertising
 *
 * @param discoverable Whether the device should be discoverable
 * @return 0 on success, error code otherwise
 */
int bt_start_advertising(int discoverable);

/**
 * Stop Bluetooth advertising
 *
 * @return 0 on success, error code otherwise
 */
int bt_stop_advertising(void);

/**
 * Start Bluetooth scanning
 *
 * @return 0 on success, error code otherwise
 */
int bt_start_scanning(void);

/**
 * Stop Bluetooth scanning
 *
 * @return 0 on success, error code otherwise
 */
int bt_stop_scanning(void);

/**
 * Get current Bluetooth state
 *
 * @return Current state
 */
bt_state_t bt_get_state(void);

/**
 * Get local device information
 *
 * @param info Pointer to store device information
 * @return 0 on success, error code otherwise
 */
int bt_get_local_info(bt_device_info_t *info);

/**
 * Shutdown Bluetooth stack
 *
 * @return 0 on success, error code otherwise
 */
int bt_shutdown(void);

#endif /* BLUETOOTH_H */