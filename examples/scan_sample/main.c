/*
 * copyright (c) 2012 - 2018, nordic semiconductor asa
 * all rights reserved.
 *
 * redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. redistributions in binary form, except as embedded into a nordic
 *    semiconductor asa integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** Includes */
#include "ble.h"
#include "sd_rpc.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>

/** Definitions */
#define DEFAULT_BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */

#ifdef _WIN32
#define DEFAULT_UART_PORT_NAME "COM1"
#endif
#ifdef __APPLE__
#define DEFAULT_UART_PORT_NAME "/dev/tty.usbmodem00000"
#endif
#ifdef __linux__
#define DEFAULT_UART_PORT_NAME "/dev/ttyACM0"
#endif

#define SCAN_INTERVAL 0x00A0 /**< Determines scan interval in units of 0.625 milliseconds. */
#define SCAN_WINDOW   0x0050 /**< Determines scan window in units of 0.625 milliseconds. */
#define SCAN_TIMEOUT  0x0    /**< Scan timeout between 0x01 and 0xFFFF in seconds, 0x0 disables timeout. */

#define STRING_BUFFER_SIZE 50

typedef struct
{
    uint8_t *     p_data;   /**< Pointer to data. */
    uint16_t      data_len; /**< Length of data. */
} data_t;

/** Global variables */
static adapter_t * m_adapter                    = NULL;
static char        peer_addr[STRING_BUFFER_SIZE]= {0};

static uint8_t     mp_data[BLE_GAP_SCAN_BUFFER_EXTENDED_MIN]                 = { 0 };
static ble_data_t  m_adv_report_buffer;
static uint32_t    m_config_id                  = 1;

static ble_gap_scan_params_t m_scan_param =
{
    0,                       // Set if accept extended advertising packetets.
    0,                       // Set if report inomplete reports.
    0,                       // Set if active scanning.
    BLE_GAP_SCAN_FP_ACCEPT_ALL,
    BLE_GAP_PHY_CODED,
    (uint16_t)SCAN_INTERVAL,
    (uint16_t)SCAN_WINDOW,
    (uint16_t)SCAN_TIMEOUT
    , { 0 }                  // Set chennel mask.
};

/** Global functions */

/**@brief Function for handling the log message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] severity Level of severity that the log message is associated with.
 * @param[in] message The log message that the callback is associated with.
 */
static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
        case SD_RPC_LOG_ERROR:
            printf("Error: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_WARNING:
            printf("Warning: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_INFO:
            printf("Info: %s\n", message);
            fflush(stdout);
            break;

        default:
            printf("Log: %s\n", message);
            fflush(stdout);
            break;
    }
}

/**@brief Function for initializing serial communication with the target nRF5 Bluetooth slave.
 *
 * @param[in] serial_port The serial port the target nRF5 device is connected to.
 *
 * @return The new transport adapter.
 */
static adapter_t * adapter_init(char * serial_port, uint32_t baud_rate)
{
    physical_layer_t  * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

    phy = sd_rpc_physical_layer_create_uart(serial_port,
                                            baud_rate,
                                            SD_RPC_FLOW_CONTROL_NONE,
                                            SD_RPC_PARITY_NONE);
    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 250);
    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 1500);
    return sd_rpc_adapter_create(transport_layer);
}

/**@brief Function for converting a BLE address to a string.
 *
 * @param[in] address       Bluetooth Low Energy address.
 * @param[out] string_buffer The serial port the target nRF5 device is connected to.
 */
static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer)
{
    const int address_length = 6;
    char      temp_str[3];

    for (int i = address_length - 1; i >= 0; --i)
    {
        sprintf(temp_str, "%02X", address.addr[i]);
        strcat((char *)string_buffer, temp_str);
    }
}

/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

/**@brief Function for searching a given name in the advertisement packets.
 *
 * @details Use this function to parse received advertising data and to find a given
 * name in them either as 'complete_local_name' or as 'short_local_name'.
 *
 * @param[in]   p_adv_report   advertising data to parse.
 * @param[in]   name_to_find   name to search.
 * @return   true if the given name was found, false otherwise.
 */
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, data_t *dev_name)
{
    uint32_t err_code;
    data_t   adv_data;

    // Initialize advertisement report for parsing
    adv_data.p_data     = (uint8_t *)p_adv_report->data.p_data;
    adv_data.data_len   = p_adv_report->data.len;

    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                dev_name);
    if (err_code == NRF_SUCCESS)
    {
        return true;
    }
    else
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                    &adv_data,
                                    dev_name);
        if (err_code != NRF_SUCCESS)
        {
            return false;
        }
        return true;
    }
    return false;
}

/**@brief Function for initializing the BLE stack.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ble_stack_init()
{
    uint32_t            err_code;
    uint32_t *          app_ram_base = NULL;

    err_code = sd_ble_enable(m_adapter, app_ram_base);

    switch (err_code) {
        case NRF_SUCCESS:
            break;
        case NRF_ERROR_INVALID_STATE:
            printf("BLE stack already enabled\n");
            fflush(stdout);
            break;
        default:
            printf("Failed to enable BLE stack. Error code: %d\n", err_code);
            fflush(stdout);
            break;
    }

    return err_code;
}

/**@brief Function for setting configuration for the BLE stack.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ble_cfg_set(uint8_t conn_cfg_tag)
{
    const uint32_t ram_start = 0; // Value is not used by ble-driver
    uint32_t error_code;
    ble_cfg_t ble_cfg;

    // Configure the connection roles.
    memset(&ble_cfg, 0, sizeof(ble_cfg));

    ble_cfg.gap_cfg.role_count_cfg.adv_set_count        = BLE_GAP_ADV_SET_COUNT_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count    = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count   = 1;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count    = 0;

    error_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_GAP_CFG_ROLE_COUNT. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = 1;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_CODED_PHY_MIN;

    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GAP. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    return NRF_SUCCESS;
}

/**@brief Start scanning (GAP Discovery procedure, Observer Procedure).
 * *
 * @return NRF_SUCCESS on successfully initiating scanning procedure, otherwise an error code.
 */
static uint32_t scan_start()
{
    m_adv_report_buffer.p_data = mp_data;
    m_adv_report_buffer.len = sizeof(mp_data);

    uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &m_scan_param
    , &m_adv_report_buffer
    );

    if (error_code != NRF_SUCCESS)
    {
        printf("Scan start failed with error code: %d\n", error_code);
        fflush(stdout);
    }

    return error_code;
}

/** Event functions */
/**@brief Function called on BLE_GAP_EVT_ADV_REPORT event.
 *
 * @details Create a connection if received advertising packet corresponds to desired BLE device.
 *
 * @param[in] p_ble_gap_evt Advertising Report Event.
 */
static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    int8_t rssi_value = 0;
    uint32_t err_code;
    uint8_t  str[STRING_BUFFER_SIZE] = {0};
    char  name[STRING_BUFFER_SIZE] = {0};
    data_t dev_name;

    // Log the Bluetooth device address of advertisement packet received.
    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);
    memcpy(peer_addr, str, STRING_BUFFER_SIZE);
    rssi_value = p_ble_gap_evt->params.adv_report.rssi;
    if(find_adv_name(&p_ble_gap_evt->params.adv_report, &dev_name))
    {
            strncpy(name, (char *)dev_name.p_data, dev_name.data_len);
            name[dev_name.data_len] = '\0';
    }
    else
    {
            strcpy(name, "unknown");
    }
    printf("device address: 0x%s, RSSI: %d device name: %s\n",
           str,
           rssi_value,
           name);
    err_code = sd_ble_gap_scan_start(m_adapter, NULL, &m_adv_report_buffer);

    if (err_code != NRF_SUCCESS)
    {
        printf("Scan start failed with error code: %d\n", err_code);
        fflush(stdout);
    }
}

/** Event dispatcher */

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received an empty BLE event\n");
        fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
            on_adv_report(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_TIMEOUT:
            scan_start();
            break;

        default:
            printf("Received an un-handled event with ID: %d\n", p_ble_evt->header.evt_id);
            fflush(stdout);
            break;
    }
}

static void enable_ext_scan(void)
{
    if(m_scan_param.extended == 0)
    {
        m_scan_param.extended = 1;
        printf("Accept Extended Advertising packet\n");
    }
}

/** Main */

/**@brief Function for application main entry.
 *
 * @param[in] argc Number of arguments (program expects 0 or 1 arguments).
 * @param[in] argv The serial port of the target nRF5 device (Optional).
 */
int main(int argc, char * argv[])
{
    uint32_t error_code;
    char *   serial_port = DEFAULT_UART_PORT_NAME;
    uint32_t baud_rate = DEFAULT_BAUD_RATE;

    const struct option longopts[] = {
        {"active", no_argument, NULL, 'a'},
        {"extadv", no_argument, NULL, 'e'},
        {"help", no_argument, NULL, 'h'},
        {"phy", required_argument, NULL, 'p'},
        {"serial_port", required_argument, NULL, 's'},
        {0, 0, 0, 0},
    };
    int opt, longindex;
    while ((opt = getopt_long(argc, argv, "aehs:p:", longopts, &longindex)) != -1)
    {
        switch (opt)
        {
        case 'a':
            printf("Active scan\n");
            m_scan_param.active = 1;
            break;
        case 'e':
            enable_ext_scan();
            break;
        case 'h':
            printf("Usage: This is example program scan with pc-ble-driver \n");
            printf("        -a: active scan\n");
            printf("        -e: accept extended advertising packet\n");
            printf("        -h: show this message\n");
            printf("        -p: select PHY (1m or coded) (default: coded)\n");
            printf("        -s: select serial port (e.g /dev/ttyACM0) (default: /dev/ttyACM0)\n");
            exit(0);
        case 's':
            if (optarg != NULL)
            {
                serial_port = optarg;
            }
            break;
        case 'p':
            if (strcmp(optarg, "1m") == 0)
            {
                m_scan_param.scan_phys = BLE_GAP_PHY_1MBPS;
            }
            else if (strcmp(optarg, "coded") == 0)
            {
                m_scan_param.scan_phys = BLE_GAP_PHY_CODED;
            }
            break;
        default:
            break;
        }
    }

    switch (m_scan_param.scan_phys)
    {
    case BLE_GAP_PHY_1MBPS:
        printf("phy: 1M_PHY\n");
        break;

    case BLE_GAP_PHY_CODED:
        printf("phy: CODED_PHY\n");
        enable_ext_scan();
        break;
    }

    printf("Serial port used: %s\n", serial_port);
    printf("Baud rate used: %d\n", baud_rate);
    fflush(stdout);

    m_adapter =  adapter_init(serial_port, baud_rate);
    sd_rpc_log_handler_severity_filter_set(m_adapter, SD_RPC_LOG_INFO);
    error_code = sd_rpc_open(m_adapter, NULL, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open nRF BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    ble_cfg_set(m_config_id);

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = scan_start();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    // Endlessly loop.
    for (;;)
    {
        sleep(1);
    }
}
