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

/*
 * Structure of this file
 * - Includes
 * - Definitions
 * - Global variables
 * - Global functions
 * - Event functions
 * - Event dispatcher
 * - Main
 */
/** Includes */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include "ble.h"
#include "sd_rpc.h"
#include "amt.h"

/** Definitions */
#define DEFAULT_BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */
#define DEFAULT_UART_PORT_NAME "/dev/ttyACM0"

#define BUFFER_SIZE     30                    /**< Sufficiently large buffer for the advertising data.  */
#define DEVICE_NAME     "Armadillo_ATT_MTU" /**< Name device advertises as over Bluetooth. */
#define MAX_PEER_COUNT  1                     /**< Maximum number of peer's application intends to manage. */

#define AMT_SERVICE_UUID             0x1523
#define AMTS_CHAR_UUID               0x1524
#define AMT_RCV_BYTES_CNT_CHAR_UUID  0x1525

#define BLE_UUID_CCCD       0x2902

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

enum
{
    UNIT_0_625_MS = 625,  /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250, /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000 /**< Number of microseconds in 10 milliseconds. */
};

#define SCAN_INTERVAL 0x00A0 /**< Determines scan interval in units of 0.625 milliseconds. */
#define SCAN_WINDOW   0x0050 /**< Determines scan window in units of 0.625 milliseconds. */
#define SCAN_TIMEOUT  0x0    /**< Scan timeout between 0x01 and 0xFFFF in seconds, 0x0 disables timeout. */

#define SLAVE_LATENCY                   0                                /**< Slave Latency in number of connection events. */
#define CONNECTION_SUPERVISION_TIMEOUT  MSEC_TO_UNITS(4000, UNIT_10_MS)  /**< Determines supervision time-out in units of 10 milliseconds. */

#define ADVERTISING_INTERVAL_40_MS 64  /**< 0.625 ms = 40 ms */

#define STRING_BUFFER_SIZE 50

#define PAYLOAD_DATA_1M                     14
#define PAYLOAD_DATA_2M                     15
#define PAYLOAD_TRANSFER_TIME_US_2M         0.5
#define PAYLOAD_TRANSFER_TIME_MIN_US        328
#define BIT_TO_OCT                          8

#define ATT_MTU_SIZE                    247
#define ATT_MTU_SIZE_CODED              24
#define DATA_LENGTH_MAX                 251
#define DATA_LENGTH_MIN                 27
#define CONNECTION_INTERVAL             7.5
#define CONNECTION_INTERVAL_CODED       400
#define GAP_EVENT_LENGTH                400

/** Global variables */
typedef struct
{
    uint8_t *     p_data;   /**< Pointer to data. */
    uint16_t      data_len; /**< Length of data. */
} data_t;

static nrf_ble_amtc_t m_amtc;
static nrf_ble_amts_t m_amts;

static bool m_notif_enabled                     = false;

static uint8_t     m_connected_devices          = 0;
static uint16_t    m_connection_handle          = 0;
static bool        m_connection_is_in_progress  = false;
static adapter_t * m_adapter                    = NULL;

static uint32_t    m_config_id                  = 1;

static bool        m_is_send_spam               = false;
static bool        m_is_cccd_config             = false;

static uint8_t     m_selected_phy               = BLE_GAP_PHY_1MBPS;
static uint16_t    m_att_mtu_size               = ATT_MTU_SIZE;
static uint8_t     m_board_role                 = BLE_GAP_ROLE_CENTRAL;

static uint8_t     mp_data[BLE_GAP_SCAN_BUFFER_EXTENDED_MIN]                 = { 0 };
static ble_data_t  m_adv_report_buffer;
static ble_gap_scan_params_t m_scan_param =
{
    1,                       // Set if accept extended advertising packetets.
    0,                       // Set if report inomplete reports.
    0,                       // Set if active scanning.
    BLE_GAP_SCAN_FP_ACCEPT_ALL,
    BLE_GAP_PHY_1MBPS,
    (uint16_t)SCAN_INTERVAL,
    (uint16_t)SCAN_WINDOW,
    (uint16_t)SCAN_TIMEOUT,
    { 0 }                  // Set chennel mask.
};

static uint8_t                  m_adv_handle = 0;
static ble_gap_adv_params_t     m_adv_params;

struct timeval m_start_time;

/** Global functions */

/**@brief Function for handling error message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] code Error code that the error message is associated with.
 * @param[in] message The error message that the callback is associated with.
 */
static void status_handler(adapter_t * adapter, sd_rpc_app_status_t code, const char * message)
{
    printf("Status: %d, message: %s\n", (uint32_t)code, message);
    fflush(stdout);
}

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
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count    = 1;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count   = 1;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count    = 0;

    error_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_GAP_CFG_ROLE_COUNT. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = m_att_mtu_size;


    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GATT. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = 1;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = GAP_EVENT_LENGTH;

    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GAP. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    return NRF_SUCCESS;
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

/**@brief Function for setting the advertisement data.
 *
 * @details Sets the full device name and its available BLE services in the advertisement data.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t advertisement_data_set()
{
    uint32_t error_code;
    uint8_t  index = 0;
    uint8_t  data_buffer[BUFFER_SIZE];

    const char  * device_name = DEVICE_NAME;
    const uint8_t name_length = (uint8_t)strlen(device_name);
    const uint8_t data_type   = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;

    // Set the device name.
    data_buffer[index++] = name_length + 1; // Device name + data type
    data_buffer[index++] = data_type;
    memcpy((char *)&data_buffer[index], device_name, name_length);
    index += name_length;

    // Set the device's available services.
    data_buffer[index++] = 3;
    data_buffer[index++] = BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE;
    // Store AMT_SERVICE_UUID in little-endian format.
    data_buffer[index++] = AMT_SERVICE_UUID & 0xFF;
    data_buffer[index++] = (AMT_SERVICE_UUID & 0xFF00) >> 8;

    // No scan response.

    ble_gap_adv_properties_t adv_properties;
    adv_properties.type             = BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED;
    adv_properties.anonymous        = 0;
    adv_properties.include_tx_power = 0;

    m_adv_params.properties         = adv_properties;
    m_adv_params.filter_policy      = BLE_GAP_ADV_FP_ANY;
    m_adv_params.duration           = BLE_GAP_ADV_TIMEOUT_LIMITED_MAX;
    m_adv_params.p_peer_addr        = NULL;
    m_adv_params.interval           = ADVERTISING_INTERVAL_40_MS;
    m_adv_params.max_adv_evts       = 0;
    m_adv_params.primary_phy        = BLE_GAP_PHY_1MBPS;
    m_adv_params.secondary_phy      = BLE_GAP_PHY_1MBPS;
    m_adv_params.channel_mask[0]    = 0;
    m_adv_params.channel_mask[1]    = 0;
    m_adv_params.channel_mask[2]    = 0;
    m_adv_params.channel_mask[3]    = 0;
    m_adv_params.channel_mask[4]    = 0;

    ble_gap_adv_data_t m_adv_data;

    ble_data_t adv_data;
    adv_data.p_data = data_buffer;
    adv_data.len    = index;

    ble_data_t scan_rsp_data;
    scan_rsp_data.p_data = NULL;
    scan_rsp_data.len    = 0;

    m_adv_data.adv_data         = adv_data;
    m_adv_data.scan_rsp_data    = scan_rsp_data;

    error_code = sd_ble_gap_adv_set_configure(m_adapter, &m_adv_handle, &m_adv_data, &m_adv_params);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to set advertisement data. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }
    return NRF_SUCCESS;
}

/**@brief Function for initializing the Advertising functionality and starting advertising.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t advertising_start()
{
    uint32_t             error_code;
    ble_gap_adv_params_t adv_params;
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.interval = ADVERTISING_INTERVAL_40_MS;

    error_code = sd_ble_gap_tx_power_set(m_adapter, BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, 8);
    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to set tx_power as advertiser. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }
    error_code = sd_ble_gap_adv_start(m_adapter, m_adv_handle, m_config_id);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to start advertising. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Started advertising\n");
    fflush(stdout);
    return NRF_SUCCESS;
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
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, const char * name_to_find)
{
    uint32_t err_code;
    data_t   adv_data;
    data_t   dev_name;

    // Initialize advertisement report for parsing
    adv_data.p_data     = (uint8_t *)p_adv_report->data.p_data;
    adv_data.data_len   = p_adv_report->data.len;

    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                &dev_name);
    if (err_code == NRF_SUCCESS)
    {
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    else
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                    &adv_data,
                                    &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            return false;
        }
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    return false;
}

/**@brief Start scanning (GAP Discovery procedure, Observer Procedure).
 * *
 * @return NRF_SUCCESS on successfully initiating scanning procedure, otherwise an error code.
 */
static uint32_t scan_start()
{
    m_adv_report_buffer.p_data = mp_data;
    m_adv_report_buffer.len = sizeof(mp_data);

    uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &m_scan_param, &m_adv_report_buffer);

    if (error_code != NRF_SUCCESS)
    {
        printf("Scan start failed with error code: %d\n", error_code);
        fflush(stdout);
    }

    return error_code;
}

/**@brief AMT Client Handler.
 */
void amtc_evt_handler(nrf_ble_amtc_t * p_amt_c, nrf_ble_amtc_evt_t * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_type)
    {
        case NRF_BLE_AMT_C_EVT_NOT_DISCOVERY_COMPLETE:
        {
            err_code = nrf_ble_amtc_handles_assign(p_amt_c,
                                                   &p_evt->params.peer_db);
            if (err_code != NRF_SUCCESS)
            {
                printf("nrf_ble_amtc_handles_assign() Failed, reason %d\n", err_code);
                fflush(stdout);
                return;
            }

            service_discovery_start(p_amt_c);
        } break;
        case NRF_BLE_AMT_C_EVT_DISCOVERY_COMPLETE:
        {
            err_code = nrf_ble_amtc_notif_enable(p_amt_c);
            if (err_code != NRF_SUCCESS)
            {
                printf("nrf_ble_amtc_notif_enable() Failed, reason %d\n", err_code);
                fflush(stdout);
                return;
            }
        } break;

        case NRF_BLE_AMT_C_EVT_NOTIFICATION:
        {
            static uint32_t bytes_cnt  = 0;
            static uint32_t kbytes_cnt = 0;

            if (p_evt->params.hvx.bytes_sent == 0)
            {
                bytes_cnt  = 0;
                kbytes_cnt = 0;
            }

            bytes_cnt += p_evt->params.hvx.notif_len;

            if (bytes_cnt > 1024)
            {
                bytes_cnt -= 1024;
                kbytes_cnt++;

                printf("Received %u kbytes\n", kbytes_cnt);
                fflush(stdout);

                nrf_ble_amts_rbc_set(&m_amts, p_evt->params.hvx.bytes_rcvd);
            }

            if (p_evt->params.hvx.bytes_rcvd >= AMT_BYTE_TRANSFER_CNT_DEFAULT)
            {
                printf("AMT Transfer complete, received %u bytes.\r\n",
                             p_evt->params.hvx.bytes_rcvd);
                fflush(stdout);

                nrf_ble_amts_rbc_set(&m_amts, p_evt->params.hvx.bytes_rcvd);
            }

        } break;

        case NRF_BLE_AMT_C_EVT_RCB_READ_RSP:
        {
            //wait transfer complete
            if ((p_evt->params.rcv_bytes_cnt) < AMT_BYTE_TRANSFER_CNT_DEFAULT)
            {
                uint32_t error_code;
                error_code = nrf_ble_amtc_rcb_read(&m_amtc);
                if (error_code != NRF_SUCCESS)
                {
                    printf("nrf_ble_amtc_rcb_read() failed: 0x%x.", error_code);
                }
            }
            else
            {
                printf("AMT peer received %u bytes (%u KBytes).\n",
                            (p_evt->params.rcv_bytes_cnt), (p_evt->params.rcv_bytes_cnt / 1024));
                fflush(stdout);

                struct timeval end_time;
                gettimeofday(&end_time,NULL);
                float elapsed_time = (end_time.tv_sec - m_start_time.tv_sec)+
                                    (end_time.tv_usec - m_start_time.tv_usec)*1.0E-6;
                float throughput_kbps = p_evt->params.rcv_bytes_cnt * 8 / elapsed_time / 1000.f;

                printf("Done.\n");
                printf("=============================\n");
                printf("Time: %.2lf seconds elapsed.\n", elapsed_time);
                printf("Throughput: %.2fKbps.\n",throughput_kbps);
                printf("=============================\n");
                printf("Sent %u bytes of ATT payload.\n", p_evt->params.rcv_bytes_cnt);
                printf("Retrieving amount of bytes received from peer...\n");
                fflush(stdout);
            }
        }break;

        case NRF_BLE_AMT_C_EVT_CCCD_CONFIGURED:
            m_is_cccd_config = true;
            break;

        default:
            break;
    }
}

/**@brief AMT Service Handler.
 */
static void amts_evt_handler(nrf_ble_amts_evt_t evt)
{
    switch (evt.evt_type)
    {
        case NRF_BLE_AMTS_EVT_NOTIF_ENABLED:
        {
            m_notif_enabled = true;
        } break;

        case NRF_BLE_AMTS_EVT_TRANSFER_1KB:
        {
            printf("Sent %u KBytes\n", (evt.bytes_transfered_cnt / 1024));
            fflush(stdout);
        } break;

        case NRF_BLE_AMTS_EVT_TRANSFER_FINISHED:
        {
            uint32_t error_code;
            error_code = nrf_ble_amtc_rcb_read(&m_amtc);
            if (error_code != NRF_SUCCESS)
            {
                printf("nrf_ble_amtc_rcb_read() failed: 0x%x.", error_code);
            }
        } break;

        default:
        {
            printf("SERVICE_EVT default\n");
            fflush(stdout);
        }
    }
}

static void client_init(void)
{
    ret_code_t err_code = nrf_ble_amtc_init(m_adapter, &m_amtc, amtc_evt_handler);
    fflush(stdout);

    if (err_code != NRF_SUCCESS)
    {
        printf("nrf_ble_amtc_init() Failed, reason %d\n", err_code);
        fflush(stdout);
        return;
    }
}

static void server_init(void)
{
    nrf_ble_amts_init(m_adapter, m_att_mtu_size, &m_amts, amts_evt_handler);
}

static void test_begin(void)
{
    if (m_board_role == BLE_GAP_ROLE_CENTRAL)
    {
        scan_start();
    }
    if (m_board_role == BLE_GAP_ROLE_PERIPH)
    {
        advertising_start();
    }
}

static bool is_test_ready()
{
    return (     m_notif_enabled
              && m_is_cccd_config
              && !m_is_send_spam);
}

static void test_run(void)
{
    m_is_send_spam = true;
    gettimeofday(&m_start_time,NULL);
    nrf_ble_amts_notif_spam(&m_amts);
}

void data_length_update()
{
    uint32_t error_code;
    ble_gap_data_length_params_t dl_params;

    memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
    if (m_selected_phy == BLE_GAP_PHY_CODED)
    {
        dl_params.max_rx_octets = DATA_LENGTH_MIN;
        dl_params.max_tx_octets = DATA_LENGTH_MIN;
    }
    else
    {
        dl_params.max_rx_octets = DATA_LENGTH_MIN;
        dl_params.max_tx_octets = DATA_LENGTH_MAX;
    }

    if (m_selected_phy == BLE_GAP_PHY_CODED)
    {
        dl_params.max_rx_time_us = BLE_GAP_DATA_LENGTH_AUTO;
        dl_params.max_tx_time_us = BLE_GAP_DATA_LENGTH_AUTO;
    }
    else
    {
        if (m_selected_phy == BLE_GAP_PHY_1MBPS)
        {
            dl_params.max_rx_time_us = (dl_params.max_rx_octets + PAYLOAD_DATA_1M) * BIT_TO_OCT;
            dl_params.max_tx_time_us = (dl_params.max_tx_octets + PAYLOAD_DATA_1M) * BIT_TO_OCT;
        }
        if (m_selected_phy == BLE_GAP_PHY_2MBPS)
        {
            dl_params.max_rx_time_us = (dl_params.max_rx_octets + PAYLOAD_DATA_2M) * BIT_TO_OCT * PAYLOAD_TRANSFER_TIME_US_2M;
            dl_params.max_tx_time_us = (dl_params.max_tx_octets + PAYLOAD_DATA_2M) * BIT_TO_OCT * PAYLOAD_TRANSFER_TIME_US_2M;
        }

        if(dl_params.max_rx_time_us < PAYLOAD_TRANSFER_TIME_MIN_US)
        {
            dl_params.max_rx_time_us = PAYLOAD_TRANSFER_TIME_MIN_US;
        }
        if(dl_params.max_tx_time_us < PAYLOAD_TRANSFER_TIME_MIN_US)
        {
            dl_params.max_tx_time_us = PAYLOAD_TRANSFER_TIME_MIN_US;
        }
    }

    error_code = sd_ble_gap_data_length_update(m_adapter, m_connection_handle, &dl_params, NULL);

    if (error_code != NRF_SUCCESS)
    {
        printf("Data length update error: %x\n", error_code);
        fflush(stdout);
        return;
    }
}

static void exchange_mtu()
{
    uint32_t err_code = sd_ble_gattc_exchange_mtu_request(
        m_adapter,
        m_connection_handle,
        m_att_mtu_size);

    if (err_code != NRF_SUCCESS)
    {
        printf("MTU exchange request failed, err_code %d\n", err_code);
        fflush(stdout);
    }
}

static void update_phy(uint16_t conn_handle)
{
    uint32_t err_code;
    ble_gap_phys_t const phys =
    {
        .rx_phys = m_selected_phy,
        .tx_phys = m_selected_phy,
    };
    if (m_selected_phy == BLE_GAP_PHY_2MBPS)
    {
        printf("Update PHY to 2M\n");
    }
    if (m_selected_phy == BLE_GAP_PHY_CODED)
    {
        printf("Update PHY to Coded\n");
    }
    fflush(stdout);

    err_code = sd_ble_gap_phy_update(m_adapter, conn_handle, &phys);
    if (err_code != NRF_SUCCESS)
    {
        printf("Update PHY Failed, reason %d\n", err_code);
        fflush(stdout);
        return;
    }
}

/** Event functions */

/**@brief Function called on BLE_GAP_EVT_CONNECTED event.
 *
 * @details Update connection state and proceed to discovering the peer's GATT services.
 *
 * @param[in] p_ble_gap_evt GAP event.
 */
static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    m_connected_devices++;
    m_connection_handle         = p_ble_gap_evt->conn_handle;
    m_connection_is_in_progress = false;

    // Stop scanning and advertising.
    sd_ble_gap_scan_stop(m_adapter);
    sd_ble_gap_adv_stop(m_adapter, m_adv_handle);

    if (m_selected_phy != BLE_GAP_PHY_1MBPS)
    {
        update_phy(p_ble_gap_evt->conn_handle);
    }

    if (m_board_role == BLE_GAP_ROLE_CENTRAL)
    {
        if(m_selected_phy != BLE_GAP_PHY_CODED)
        {
            data_length_update();
        }
        printf("Connected as a central\n");
        fflush(stdout);
    }
    else
    {
        printf("Connected as a peripheral\n");
        fflush(stdout);
    }
}

/**@brief Function called on BLE_GAP_EVT_ADV_REPORT event.
 *
 * @details Create a connection if received advertising packet corresponds to desired BLE device.
 *
 * @param[in] p_ble_gap_evt Advertising Report Event.
 */
static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code;
    uint8_t  str[STRING_BUFFER_SIZE] = {0};
    uint16_t connection_interval;

    if (m_selected_phy == BLE_GAP_PHY_CODED)
    {
        connection_interval = MSEC_TO_UNITS(CONNECTION_INTERVAL_CODED, UNIT_1_25_MS);
    }
    else
    {
        connection_interval = MSEC_TO_UNITS(CONNECTION_INTERVAL, UNIT_1_25_MS);
    }

    ble_gap_conn_params_t connection_param =
    {
        connection_interval,
        connection_interval,
        (uint16_t)SLAVE_LATENCY,
        (uint16_t)CONNECTION_SUPERVISION_TIMEOUT
    };

    // Log the Bluetooth device address of advertisement packet received.
    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);

    if (find_adv_name(&p_ble_gap_evt->params.adv_report, DEVICE_NAME))
    {
        if (m_connected_devices >= MAX_PEER_COUNT || m_connection_is_in_progress)
        {
            return;
        }

        printf("Received advertisement report with device address: 0x%s\n", str);
        fflush(stdout);

        // Stop advertising.
        sd_ble_gap_adv_stop(m_adapter, m_adv_handle);

        err_code = sd_ble_gap_connect(m_adapter,
                                      &(p_ble_gap_evt->params.adv_report.peer_addr),
                                      &m_scan_param,
                                      &connection_param
                                     , m_config_id
                                     );
        if (err_code != NRF_SUCCESS)
        {
            printf("Connection Request Failed, reason %d\n", err_code);
            fflush(stdout);
            return;
        }
        m_connection_is_in_progress = true;
    }
    else {
        err_code = sd_ble_gap_scan_start(m_adapter, NULL, &m_adv_report_buffer);

        if (err_code != NRF_SUCCESS)
        {
            printf("Scan start failed with error code: %d\n", err_code);
            fflush(stdout);
        }
    }
}

/** Event dispatcher */
static void on_ble_evt(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    ble_gap_data_length_params_t dl_params;

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

        case BLE_GAP_EVT_CONNECTED:
            on_connected(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
            printf("Received BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST.\n");
            fflush(stdout);
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));

            err_code = sd_ble_gap_data_length_update(adapter, p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            if (err_code != NRF_SUCCESS)
            {
                printf("Data len update error: %x\n", err_code);
                fflush(stdout);
            }
            break;

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
            if(m_board_role == BLE_GAP_ROLE_PERIPH)
            {
                printf("Received BLE_GAP_EVT_DATA_LENGTH_UPDATE (max_rx_octets:%d, max_rx_time_us:%d, max_tx_octets:%d, max_tx_time_us:%d)\n",
                        p_ble_evt->evt.gap_evt.params.data_length_update.effective_params.max_rx_octets,
                        p_ble_evt->evt.gap_evt.params.data_length_update.effective_params.max_rx_time_us,
                        p_ble_evt->evt.gap_evt.params.data_length_update.effective_params.max_tx_octets,
                        p_ble_evt->evt.gap_evt.params.data_length_update.effective_params.max_tx_time_us);
                fflush(stdout);
            }
            break;

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(adapter, m_connection_handle,
                                                       m_att_mtu_size);

            if (err_code != NRF_SUCCESS)
            {
                printf("MTU exchange request reply failed. Error code: 0x%02X\n", err_code);
                fflush(stdout);
            }
            printf("MTU exchange %d\n", m_att_mtu_size);
            fflush(stdout);
            break;

        case BLE_GAP_EVT_PHY_UPDATE:
            printf("Received BLE_GAP_EVT_PHY_UPDATE (RX:%d, TX:%d, status:%d)\n",
                    p_ble_evt->evt.gap_evt.params.phy_update.rx_phy,
                    p_ble_evt->evt.gap_evt.params.phy_update.tx_phy,
                    p_ble_evt->evt.gap_evt.params.phy_update.status);
            fflush(stdout);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            printf("Received BLE_GAP_EVT_PHY_UPDATE_REQUEST.\n");
	        fflush(stdout);

            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(adapter, p_ble_evt->evt.gap_evt.conn_handle, &phys);
	        break;

        default:
            break;
    }
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    on_ble_evt(adapter, p_ble_evt);
    nrf_ble_amts_on_ble_evt(&m_amts, p_ble_evt);
    nrf_ble_amtc_on_ble_evt(&m_amtc, p_ble_evt);
}

/** Main */
int main(int argc, char * argv[])
{
    char *          serial_port = DEFAULT_UART_PORT_NAME;
    uint32_t        baud_rate = DEFAULT_BAUD_RATE;
    uint32_t        error_code;

    const struct option longopts[] = {
        {"serial_port", optional_argument, NULL, 's'},
        {"role", optional_argument, NULL, 'r'},
        {"phy", optional_argument, NULL, 'p'},
        {0, 0, 0, 0},
    };
    int opt, longindex;
    while ((opt = getopt_long(argc, argv, "s::r::p::", longopts, &longindex)) != -1)
    {
        switch(opt)
        {
            case 's':
                if (optarg != NULL)
                {
                    serial_port = optarg;
                }
                break;
            case 'r':
                if (strcmp(optarg, "cent") == 0)
                {
                    m_board_role = BLE_GAP_ROLE_CENTRAL;
                }
                else if (strcmp(optarg, "peri") == 0)
                {
                    m_board_role = BLE_GAP_ROLE_PERIPH;
                }
                break;
            case 'p':
            if (strcmp(optarg, "1m") == 0)
            {
                m_selected_phy = BLE_GAP_PHY_1MBPS;
                printf("phy: 1M_PHY\n");
            }
            else if (strcmp(optarg, "2m") == 0)
            {
                m_selected_phy = BLE_GAP_PHY_2MBPS;
                printf("phy: 2M_PHY\n");
            }
            else if (strcmp(optarg, "coded") == 0)
            {
                m_selected_phy = BLE_GAP_PHY_CODED;
                printf("phy: Coded_PHY\n");
            }
            break;
        default:
            break;
        }
        fflush(stdout);
    }

    printf("Serial port used: %s\n", serial_port);
    printf("Baud rate used: %d\n", baud_rate);
    fflush(stdout);

    m_adapter =  adapter_init(serial_port, baud_rate);
    sd_rpc_log_handler_severity_filter_set(m_adapter, SD_RPC_LOG_INFO);

    error_code = sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open nRF BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return -1;
    }

    if (m_selected_phy == BLE_GAP_PHY_CODED)
    {
        m_att_mtu_size = ATT_MTU_SIZE_CODED;
    }

    error_code = ble_cfg_set(m_config_id);

    if (error_code != NRF_SUCCESS)
    {
        return -1;
    }

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return -1;
    }

    error_code = advertisement_data_set();

    if (error_code != NRF_SUCCESS)
    {
        return -1;
    }

    server_init();
    client_init();

    test_begin();

    switch (m_board_role)
    {
        case BLE_GAP_ROLE_CENTRAL:
            for(;;)
            {
                if (is_test_ready())
                {
                    exchange_mtu();
                    printf("Press enter to start throughput test\n");
                    fflush(stdout);
                    getchar();
                    test_run();
                    break;
                }
            }

        case BLE_GAP_ROLE_PERIPH:
            for(;;)
            {
                sleep(1);
            }
    }
}
