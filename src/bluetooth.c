#include "bluetooth.h"

// External UART functions from your implementation
extern void uart_init(void);
extern void uart_putc(unsigned char c);
extern void uart_puts(const char *str);
extern void uart_printf(const char *fmt, ...);
extern int uart_available(void);
extern int uart_read_nonblock(void);
extern void gpio_set_function(unsigned int pin, unsigned int function);
extern void gpio_set_pull(unsigned int pin, unsigned int pull);

// Current Bluetooth state
static bt_state_t bt_state = BT_STATE_OFF;
static bt_firmware_status_t firmware_status = BT_FIRMWARE_NOT_LOADED;
static bt_event_callback_t event_callback = NULL;
static bt_device_info_t local_device_info;

// HCI response buffers
static uint8_t hci_event_buffer[256];
static uint16_t hci_event_pos = 0;
static uint16_t hci_event_expected_length = 0;

// Create HCI command opcode from OGF and OCF
#define HCI_OPCODE(ogf, ocf) (((ogf) << 10) | (ocf))

// Utility function to delay (you might need to implement this)
static void delay_ms(uint32_t ms)
{
    // Simple busy-wait implementation
    // In a real system, you might want a more efficient delay
    volatile uint32_t i;
    for (i = 0; i < ms * 10000; i++)
    {
        __asm__("nop");
    }
}

int bt_init_hardware(void)
{
    // Initialize the UART for Bluetooth communication
    uart_init();

    // Configure GPIO pins for Bluetooth

    // Enable the Bluetooth module (GPIO 43 on RPi 4B)
    gpio_set_function(BT_ENABLE_PIN, GPIO_FUNCTION_ALT0);

    // Configure UART pins
    gpio_set_function(BT_UART_TX_PIN, GPIO_FUNCTION_ALT5);  // ALT5 is UART1_TX on these pins
    gpio_set_function(BT_UART_RX_PIN, GPIO_FUNCTION_ALT5);  // ALT5 is UART1_RX on these pins
    gpio_set_function(BT_UART_CTS_PIN, GPIO_FUNCTION_ALT5); // ALT5 is UART1_CTS on these pins
    gpio_set_function(BT_UART_RTS_PIN, GPIO_FUNCTION_ALT5); // ALT5 is UART1_RTS on these pins

    // Set appropriate pull-up/down resistors
    gpio_set_pull(BT_UART_RX_PIN, GPIO_PULL_UP);
    gpio_set_pull(BT_UART_CTS_PIN, GPIO_PULL_UP);

    // Reset Bluetooth module by toggling the enable pin
    gpio_set_function(BT_ENABLE_PIN, GPIO_FUNCTION_ALT0);
    delay_ms(100);
    gpio_set_function(BT_ENABLE_PIN, 0); // Output mode, low
    delay_ms(100);
    gpio_set_function(BT_ENABLE_PIN, GPIO_FUNCTION_ALT0);
    delay_ms(100);

    bt_state = BT_STATE_INITIALIZING;
    uart_printf("Bluetooth hardware initialized\r\n");

    return 0;
}

int bt_send_hci_command(uint16_t opcode, const uint8_t *params, uint8_t length)
{
    uint8_t hdr[4];

    // HCI Command packet
    hdr[0] = HCI_COMMAND_PACKET;
    // Opcode (LSB first)
    hdr[1] = opcode & 0xFF;
    hdr[2] = (opcode >> 8) & 0xFF;
    // Parameter length
    hdr[3] = length;

    // Send header
    for (int i = 0; i < 4; i++)
    {
        uart_putc(hdr[i]);
    }

    // Send parameters if any
    if (length > 0 && params != NULL)
    {
        for (int i = 0; i < length; i++)
        {
            uart_putc(params[i]);
        }
    }

    return 0;
}

void bt_register_event_callback(bt_event_callback_t callback)
{
    event_callback = callback;
}

// Process HCI event packet
static void process_hci_event(uint8_t *data, uint16_t length)
{
    if (length < 2)
        return; // No valid event

    uint8_t event_code = data[0];
    uint8_t param_length = data[1];

    if (event_callback)
    {
        event_callback(event_code, data + 2, param_length);
    }

    // Process some specific events internally
    switch (event_code)
    {
    case HCI_EVENT_COMMAND_COMPLETE:
        if (param_length >= 3)
        {
            uint16_t opcode = data[4] | (data[5] << 8);
            uint8_t status = data[6];

            switch (opcode)
            {
            case HCI_RESET:
                uart_printf("HCI Reset complete, status: %02x\r\n", status);
                break;

            case HCI_READ_BD_ADDR:
                if (status == 0 && param_length >= 9)
                {
                    memcpy(local_device_info.addr, &data[7], 6);
                    uart_printf("BD Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                                local_device_info.addr[5], local_device_info.addr[4],
                                local_device_info.addr[3], local_device_info.addr[2],
                                local_device_info.addr[1], local_device_info.addr[0]);
                }
                break;

            case HCI_READ_LOCAL_NAME:
                if (status == 0 && param_length >= 249)
                {
                    memcpy(local_device_info.name, &data[7], 248);
                    local_device_info.name[248] = '\0'; // Ensure null termination
                    uart_printf("Local name: %s\r\n", local_device_info.name);
                }
                break;
            }
        }
        break;

    case HCI_EVENT_VENDOR_SPECIFIC:
        // Handle vendor-specific events if needed
        break;
    }
}

void bt_process(void)
{
    // Process any incoming HCI data
    while (uart_available())
    {
        int c = uart_read_nonblock();
        if (c < 0)
            break;

        uint8_t byte = (uint8_t)c;

        // Simple HCI packet parsing state machine
        if (hci_event_pos == 0)
        {
            // Looking for packet type
            if (byte == HCI_EVENT_PACKET)
            {
                hci_event_buffer[hci_event_pos++] = byte;
            }
        }
        else if (hci_event_pos == 1)
        {
            // Event code
            hci_event_buffer[hci_event_pos++] = byte;
        }
        else if (hci_event_pos == 2)
        {
            // Parameter length
            hci_event_buffer[hci_event_pos++] = byte;
            hci_event_expected_length = byte + 3; // Total length = header (3) + param_length
        }
        else
        {
            // Parameter data
            hci_event_buffer[hci_event_pos++] = byte;

            // If we've received all expected bytes, process the packet
            if (hci_event_pos >= hci_event_expected_length)
            {
                process_hci_event(hci_event_buffer + 1, hci_event_pos - 1); // Skip packet type
                hci_event_pos = 0;
                hci_event_expected_length = 0;
            }

            // Prevent buffer overflow
            if (hci_event_pos >= sizeof(hci_event_buffer))
            {
                hci_event_pos = 0;
                hci_event_expected_length = 0;
            }
        }
    }
}

int bt_load_firmware(const uint8_t *firmware, size_t size)
{
    if (bt_state != BT_STATE_INITIALIZING)
    {
        return -1;
    }

    uart_printf("Loading Bluetooth firmware (%d bytes)...\r\n", size);

    // In a real implementation, you would need to send the firmware
    // to the controller using vendor-specific HCI commands
    // This is a placeholder that pretends to load firmware

    firmware_status = BT_FIRMWARE_LOADING;

    // Simulate firmware loading (in a real implementation, you'd send chunks of data)
    delay_ms(500);

    firmware_status = BT_FIRMWARE_LOADED;
    uart_printf("Bluetooth firmware loaded successfully\r\n");

    return 0;
}

int bt_initialize(void)
{
    if (bt_state != BT_STATE_INITIALIZING && firmware_status != BT_FIRMWARE_LOADED)
    {
        return -1;
    }

    uart_printf("Initializing Bluetooth stack...\r\n");

    // Reset the controller
    bt_send_hci_command(HCI_RESET, NULL, 0);
    delay_ms(100);

    // Set event mask to receive all events
    uint8_t event_mask[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    bt_send_hci_command(HCI_SET_EVENT_MASK, event_mask, 8);
    delay_ms(100);

    // Read local version information
    bt_send_hci_command(HCI_READ_LOCAL_VERSION, NULL, 0);
    delay_ms(100);

    // Read BD_ADDR (Bluetooth address)
    bt_send_hci_command(HCI_READ_BD_ADDR, NULL, 0);
    delay_ms(100);

    // Read buffer sizes
    bt_send_hci_command(HCI_READ_BUFFER_SIZE, NULL, 0);
    delay_ms(100);

    // Set a default name
    char name[248] = "Raspberry Pi Baremetal BT";
    memset(name + strlen(name), 0, 248 - strlen(name));
    bt_send_hci_command(HCI_WRITE_LOCAL_NAME, (uint8_t *)name, 248);
    delay_ms(100);

    // Read back the name to confirm
    bt_send_hci_command(HCI_READ_LOCAL_NAME, NULL, 0);
    delay_ms(100);

    bt_state = BT_STATE_READY;
    uart_printf("Bluetooth stack initialized\r\n");

    return 0;
}

int bt_set_name(const char *name)
{
    if (bt_state < BT_STATE_READY)
    {
        return -1;
    }

    // Prepare the name parameter (fixed 248 byte field)
    uint8_t name_param[248];
    size_t name_len = strlen(name);

    if (name_len > 247)
        name_len = 247; // Ensure room for null terminator

    memcpy(name_param, name, name_len);
    memset(name_param + name_len, 0, 248 - name_len);

    // Send the write name command
    bt_send_hci_command(HCI_WRITE_LOCAL_NAME, name_param, 248);

    // Update local copy
    memcpy(local_device_info.name, name_param, 248);

    return 0;
}

int bt_start_advertising(int discoverable)
{
    if (bt_state < BT_STATE_READY)
    {
        return -1;
    }

    // Set advertising parameters
    uint8_t adv_params[15] = {
        0x40, 0x06,       // Advertising interval min (0x0640 = 1s)
        0x40, 0x06,       // Advertising interval max (0x0640 = 1s)
        0x02,             // Advertising type (0x02 = ADV_SCAN_IND)
        0x00,             // Own address type (0x00 = Public)
        0x00,             // Peer address type
        0x00, 0x00, 0x00, // Peer address (6 bytes)
        0x00, 0x00, 0x00,
        0x07, // Advertising channel map (all channels)
        0x00  // Advertising filter policy
    };

    bt_send_hci_command(HCI_LE_SET_ADVERTISING_PARAMETERS, adv_params, 15);
    delay_ms(100);

    // Set advertising data
    uint8_t adv_data[31] = {0};
    uint8_t adv_data_len = 0;

    // Add flags
    adv_data[adv_data_len++] = 2;    // Length of this data
    adv_data[adv_data_len++] = 0x01; // Type: Flags
    adv_data[adv_data_len++] = 0x06; // Flags: LE General Discoverable, BR/EDR not supported

    // Add device name
    size_t name_len = strlen(local_device_info.name);
    if (name_len > 29 - adv_data_len)
        name_len = 29 - adv_data_len;

    adv_data[adv_data_len++] = name_len + 1; // Length of this data
    adv_data[adv_data_len++] = 0x09;         // Type: Complete Local Name
    memcpy(&adv_data[adv_data_len], local_device_info.name, name_len);
    adv_data_len += name_len;

    // Set the total length at the beginning
    uint8_t total_adv_data[32];
    total_adv_data[0] = adv_data_len;
    memcpy(&total_adv_data[1], adv_data, adv_data_len);

    bt_send_hci_command(HCI_LE_SET_ADVERTISING_DATA, total_adv_data, adv_data_len + 1);
    delay_ms(100);

    // Enable advertising
    uint8_t enable[1] = {0x01}; // Enable advertising
    bt_send_hci_command(HCI_LE_SET_ADVERTISE_ENABLE, enable, 1);

    bt_state = BT_STATE_ADVERTISING;
    uart_printf("Bluetooth advertising started\r\n");

    return 0;
}

int bt_stop_advertising(void)
{
    if (bt_state != BT_STATE_ADVERTISING)
    {
        return -1;
    }

    // Disable advertising
    uint8_t disable[1] = {0x00}; // Disable advertising
    bt_send_hci_command(HCI_LE_SET_ADVERTISE_ENABLE, disable, 1);

    bt_state = BT_STATE_READY;
    uart_printf("Bluetooth advertising stopped\r\n");

    return 0;
}

int bt_start_scanning(void)
{
    if (bt_state < BT_STATE_READY)
    {
        return -1;
    }

    // Set scan parameters
    uint8_t scan_params[7] = {
        0x01,       // Scan type (0x01 = Active scanning)
        0x20, 0x03, // Scan interval (0x0320 = 500ms)
        0x20, 0x03, // Scan window (0x0320 = 500ms)
        0x00,       // Own address type (0x00 = Public)
        0x00        // Scanning filter policy
    };

    bt_send_hci_command(HCI_LE_SET_SCAN_PARAMETERS, scan_params, 7);
    delay_ms(100);

    // Enable scanning
    uint8_t enable[2] = {
        0x01, // Enable scanning
        0x00  // Filter duplicates disabled
    };

    bt_send_hci_command(HCI_LE_SET_SCAN_ENABLE, enable, 2);

    bt_state = BT_STATE_SCANNING;
    uart_printf("Bluetooth scanning started\r\n");

    return 0;
}

int bt_stop_scanning(void)
{
    if (bt_state != BT_STATE_SCANNING)
    {
        return -1;
    }

    // Disable scanning
    uint8_t disable[2] = {
        0x00, // Disable scanning
        0x00  // Filter duplicates parameter (ignored when disabling)
    };

    bt_send_hci_command(HCI_LE_SET_SCAN_ENABLE, disable, 2);

    bt_state = BT_STATE_READY;
    uart_printf("Bluetooth scanning stopped\r\n");

    return 0;
}

bt_state_t bt_get_state(void)
{
    return bt_state;
}

int bt_get_local_info(bt_device_info_t *info)
{
    if (bt_state < BT_STATE_READY)
    {
        return -1;
    }

    memcpy(info, &local_device_info, sizeof(bt_device_info_t));
    return 0;
}

int bt_shutdown(void)
{
    // Stop any ongoing operations
    if (bt_state == BT_STATE_ADVERTISING)
    {
        bt_stop_advertising();
    }
    else if (bt_state == BT_STATE_SCANNING)
    {
        bt_stop_scanning();
    }

    // Reset the controller
    bt_send_hci_command(HCI_RESET, NULL, 0);
    delay_ms(100);

    // Disable the Bluetooth module
    gpio_set_function(BT_ENABLE_PIN, 0); // Set to output mode, low state

    bt_state = BT_STATE_OFF;
    firmware_status = BT_FIRMWARE_NOT_LOADED;

    uart_printf("Bluetooth shutdown complete\r\n");

    return 0;
}