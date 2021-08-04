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

/**@example examples/heart_rate_collector
 *
 * @brief Heart Rate Collector Sample Application main file.
 *
 * This file contains the source code for a sample application that acts as a BLE Central device.
 * This application scans for a Heart Rate Sensor device and reads it's heart rate data.
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.heart_rate.xml
 *
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
#include "ble.h"
#include "sd_rpc.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>


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

enum
{
    UNIT_0_625_MS = 625,  /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250, /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000 /**< Number of microseconds in 10 milliseconds. */
};

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define SCAN_INTERVAL 0x00A0 /**< Determines scan interval in units of 0.625 milliseconds. */
#define SCAN_WINDOW   0x0050 /**< Determines scan window in units of 0.625 milliseconds. */
#define SCAN_TIMEOUT  0x0    /**< Scan timeout between 0x01 and 0xFFFF in seconds, 0x0 disables timeout. */

#define MIN_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY                   0                                /**< Slave Latency in number of connection events. */
#define CONNECTION_SUPERVISION_TIMEOUT  MSEC_TO_UNITS(4000, UNIT_10_MS)  /**< Determines supervision time-out in units of 10 milliseconds. */

#define TARGET_DEV_NAME "test_armadillo" /**< Connect to a peripheral using a given advertising name here. */
#define MAX_PEER_COUNT 8            /**< Maximum number of peer's application intends to manage. */

#define BLE_UUID_HEART_RATE_SERVICE          0x180D /**< Heart Rate service UUID. */
#define BLE_UUID_HEART_RATE_MEASUREMENT_CHAR 0x2A37 /**< Heart Rate Measurement characteristic UUID. */
#define BLE_UUID_CCCD                        0x2902
#define BLE_CCCD_NOTIFY                      0x01

#define STRING_BUFFER_SIZE 50

typedef struct
{
    uint8_t *     p_data;   /**< Pointer to data. */
    uint16_t      data_len; /**< Length of data. */
} data_t;

typedef struct
{
    uint16_t      connection_handle;
    char          peer_addr[STRING_BUFFER_SIZE];
} connect_t;

/** Global variables */
static uint8_t     m_connected_devices          = 0;
static uint16_t    m_connection_handle          = 0;
static uint16_t    m_service_start_handle       = 0;
static uint16_t    m_service_end_handle         = 0;
static uint16_t    m_hrm_char_handle            = 0;
static uint16_t    m_hrm_cccd_handle            = 0;
static bool        m_connection_is_in_progress  = false;
static bool        m_2m_phy_selected            = false;
static bool        is_connected                 = false;
static bool        do_connect                   = false;
static adapter_t * m_adapter                    = NULL;
static connect_t   m_connect[MAX_PEER_COUNT];
static char        peer_addr[STRING_BUFFER_SIZE]= {0};

#if NRF_SD_BLE_API >= 5
static uint32_t    m_config_id                  = 1;
#endif

#if NRF_SD_BLE_API >= 6
static uint8_t     mp_data[BLE_GAP_SCAN_BUFFER_EXTENDED_MIN]                 = { 0 };
static ble_data_t  m_adv_report_buffer;
#endif

static ble_gap_scan_params_t m_scan_param =
{
#if NRF_SD_BLE_API >= 6
    1,                       // Set if accept extended advertising packetets.
    0,                       // Set if report inomplete reports.
#endif
    0,                       // Set if active scanning.
#if NRF_SD_BLE_API < 6
    0,                       // Set if selective scanning.
#endif
#if NRF_SD_BLE_API >= 6
    BLE_GAP_SCAN_FP_ACCEPT_ALL,
    BLE_GAP_PHY_CODED,
#endif
#if NRF_SD_BLE_API == 2
    NULL,                    // Set white-list.
#endif
#if NRF_SD_BLE_API == 3 || NRF_SD_BLE_API == 5
    0,                       // Set adv_dir_report.
#endif
    (uint16_t)SCAN_INTERVAL,
    (uint16_t)SCAN_WINDOW,
    (uint16_t)SCAN_TIMEOUT
#if NRF_SD_BLE_API >= 6
    , { 0 }                  // Set chennel mask.
#endif
};

static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,
    (uint16_t)MAX_CONNECTION_INTERVAL,
    (uint16_t)SLAVE_LATENCY,
    (uint16_t)CONNECTION_SUPERVISION_TIMEOUT
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
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, const char * name_to_find)
{
    uint32_t err_code;
    data_t   adv_data;
    data_t   dev_name;

    // Initialize advertisement report for parsing
#if NRF_SD_BLE_API >= 6
    adv_data.p_data     = (uint8_t *)p_adv_report->data.p_data;
    adv_data.data_len   = p_adv_report->data.len;
#else
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;
#endif

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

/**@brief Function for initializing the BLE stack.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ble_stack_init()
{
    uint32_t            err_code;
    uint32_t *          app_ram_base = NULL;

#if NRF_SD_BLE_API <= 3
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#endif

#if NRF_SD_BLE_API == 3
    ble_enable_params.gatt_enable_params.att_mtu = GATT_MTU_SIZE_DEFAULT;
#elif NRF_SD_BLE_API < 3
    ble_enable_params.gatts_enable_params.attr_tab_size     = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed   = false;
    ble_enable_params.common_enable_params.p_conn_bw_counts = NULL;
    ble_enable_params.common_enable_params.vs_uuid_count    = 1;
#endif

#if NRF_SD_BLE_API <= 3
    ble_enable_params.gap_enable_params.periph_conn_count   = 1;
    ble_enable_params.gap_enable_params.central_conn_count  = 1;
    ble_enable_params.gap_enable_params.central_sec_count   = 1;

    err_code = sd_ble_enable(m_adapter, &ble_enable_params, app_ram_base);
#else
    err_code = sd_ble_enable(m_adapter, app_ram_base);
#endif

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

#if NRF_SD_BLE_API < 5
/**@brief Set BLE option for the BLE role and connection bandwidth.
 *
 * @return NRF_SUCCESS on option set successfully, otherwise an error code.
 */
static uint32_t ble_options_set()
{
#if NRF_SD_BLE_API <= 3
    ble_opt_t        opt;
    ble_common_opt_t common_opt;

    common_opt.conn_bw.role                 = BLE_GAP_ROLE_CENTRAL;
    common_opt.conn_bw.conn_bw.conn_bw_rx   = BLE_CONN_BW_HIGH;
    common_opt.conn_bw.conn_bw.conn_bw_tx   = BLE_CONN_BW_HIGH;
    opt.common_opt                          = common_opt;

    return sd_ble_opt_set(m_adapter, BLE_COMMON_OPT_CONN_BW, &opt);
#else
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}
#endif

#if NRF_SD_BLE_API >= 5
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

#if NRF_SD_BLE_API >= 6
    ble_cfg.gap_cfg.role_count_cfg.adv_set_count        = BLE_GAP_ADV_SET_COUNT_DEFAULT;
#endif
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count    = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count   = MAX_PEER_COUNT;
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
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = 150;

    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GATT. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

#if NRF_SD_BLE_API >= 6
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = MAX_PEER_COUNT;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_CODED_PHY_MIN;

    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    if (error_code != NRF_SUCCESS)
    {
        printf("sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GAP. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }
#endif

    return NRF_SUCCESS;
}
#endif

/**@brief Start scanning (GAP Discovery procedure, Observer Procedure).
 * *
 * @return NRF_SUCCESS on successfully initiating scanning procedure, otherwise an error code.
 */
static uint32_t scan_start()
{
#if NRF_SD_BLE_API >= 6
    m_adv_report_buffer.p_data = mp_data;
    m_adv_report_buffer.len = sizeof(mp_data);
#endif

    uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &m_scan_param
#if NRF_SD_BLE_API >= 6
    , &m_adv_report_buffer
#endif
    );

    if (error_code != NRF_SUCCESS)
    {
        printf("Scan start failed with error code: %d\n", error_code);
        fflush(stdout);
    }

    return error_code;
}

/**@brief Function called upon connecting to BLE peripheral.
 *
 * @details Initiates primary service discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t service_discovery_start()
{
    uint32_t   err_code;
    uint16_t   start_handle = 0x01;
    ble_uuid_t srvc_uuid;

    srvc_uuid.type = BLE_UUID_TYPE_BLE;
    srvc_uuid.uuid = BLE_UUID_HEART_RATE_SERVICE;

    // Initiate procedure to find the primary BLE_UUID_HEART_RATE_SERVICE.
    err_code = sd_ble_gattc_primary_services_discover(m_adapter,
                                                      m_connection_handle, start_handle,
                                                      &srvc_uuid);
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to initiate or continue a GATT Primary Service Discovery procedure\n");
        fflush(stdout);
    }

    return err_code;
}

/**@brief Function called upon discovering a BLE peripheral's primary service(s).
 *
 * @details Initiates service's (m_service) characteristic discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t char_discovery_start()
{
    ble_gattc_handle_range_t handle_range;

    handle_range.start_handle = m_service_start_handle;
    handle_range.end_handle = m_service_end_handle;

    return sd_ble_gattc_characteristics_discover(m_adapter, m_connection_handle, &handle_range);
}

/**@brief Function called upon discovering service's characteristics.
 *
 * @details Initiates heart rate monitor (m_hrm_char_handle) characteristic's descriptor discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t descr_discovery_start()
{
    ble_gattc_handle_range_t handle_range;

    if (m_hrm_char_handle == 0)
    {
        printf("No heart rate measurement characteristic handle found\n");
        fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    handle_range.start_handle   = m_hrm_char_handle;
    handle_range.end_handle     = m_service_end_handle;

    return sd_ble_gattc_descriptors_discover(m_adapter, m_connection_handle, &handle_range);
}

static uint32_t rssi_measurements_start()
{
    uint8_t threshold    = 2;
    uint8_t skip_count   = 10;
    uint32_t err_code;

    err_code = sd_ble_gap_rssi_start(m_adapter, m_connection_handle, threshold, skip_count);
    if (err_code != NRF_SUCCESS)
    {
        printf("RSSI start failed with error code: 0x%X\n", err_code);
        fflush(stdout);
    }

    return err_code;
}

static void update_phy_to_2M(uint16_t conn_handle)
{
    uint32_t err_code;
    ble_gap_phys_t const phys =
    {
        .rx_phys = BLE_GAP_PHY_2MBPS,
        .tx_phys = BLE_GAP_PHY_2MBPS,
    };
    printf("Update PHY to 2M\n");
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
    if (m_2m_phy_selected)
    {
        update_phy_to_2M(p_ble_gap_evt->conn_handle);
    }

    m_connected_devices++;
    m_connection_handle         = p_ble_gap_evt->conn_handle;
    m_connect[m_connection_handle].connection_handle = p_ble_gap_evt->conn_handle;
    strcpy(m_connect[p_ble_gap_evt->conn_handle].peer_addr, peer_addr);
    m_connection_is_in_progress = false;

    service_discovery_start();
    rssi_measurements_start();
    if (m_connected_devices < MAX_PEER_COUNT)
    {
        scan_start();
    }
}

static void on_disconnected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    printf("peripheral handle 0x%x disconnected (reason: 0x%x)\n",
            p_ble_gap_evt->conn_handle,
            p_ble_gap_evt->params.disconnected.reason);
    fflush(stdout);
    m_connected_devices--;
    fflush(stdout);
    m_connect[p_ble_gap_evt->conn_handle].connection_handle = 0;
    m_connect[p_ble_gap_evt->conn_handle].peer_addr[0] = '\0';
}


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

    // Log the Bluetooth device address of advertisement packet received.
    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);
    memcpy(peer_addr, str, STRING_BUFFER_SIZE);
    if (find_adv_name(&p_ble_gap_evt->params.adv_report, TARGET_DEV_NAME))
    {
        rssi_value = p_ble_gap_evt->params.adv_report.rssi;
        printf("Received advertisement report with device address: 0x%s, RSSI: %d\n", str, rssi_value);
        if (m_connected_devices >= MAX_PEER_COUNT || m_connection_is_in_progress)
        {
            return;
        }

        if (do_connect)
        {
            err_code = sd_ble_gap_connect(m_adapter,
                                          &(p_ble_gap_evt->params.adv_report.peer_addr),
                                          &m_scan_param,
                                          &m_connection_param
#if NRF_SD_BLE_API >= 5
                                         , m_config_id
#endif
                                         );

            if (err_code != NRF_SUCCESS)
            {
                printf("Connection Request Failed, reason %d\n", err_code);
                fflush(stdout);
                return;
            }

            m_connection_is_in_progress = true;
        }
        else
        {
            err_code = sd_ble_gap_scan_start(m_adapter, NULL, &m_adv_report_buffer);

            if (err_code != NRF_SUCCESS)
            {
                printf("Scan start failed with error code: %d\n", err_code);
                fflush(stdout);
            }
        }
    }
#if NRF_SD_BLE_API >= 6
    else {
        err_code = sd_ble_gap_scan_start(m_adapter, NULL, &m_adv_report_buffer);

        if (err_code != NRF_SUCCESS)
        {
            printf("Scan start failed with error code: %d\n", err_code);
            fflush(stdout);
        }
    }
#endif

}

/**@brief Function called on BLE_GAP_EVT_TIMEOUT event.
 *
 * @param[in] ble_gap_evt_t Timeout Event.
 */
static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt)
{
    if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
    {
        m_connection_is_in_progress = false;
    }
    else if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
    {
        scan_start();
    }
}

/**@brief Function called on BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP event.
 *
 * @details Update service state and proceed to discovering the service's GATT characteristics.
 *
 * @param[in] p_ble_gattc_evt Primary Service Discovery Response Event.
 */
static void on_service_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count;
    int service_index;
    const ble_gattc_service_t * service;

    printf("Received service discovery response\n");
    fflush(stdout);

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Service discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_gattc_evt->params.prim_srvc_disc_rsp.count;

    if (count == 0)
    {
        printf("Service not found\n");
        fflush(stdout);
        return;
    }

    if (count > 1)
    {
        printf("Warning, discovered multiple primary services. Ignoring all but the first\n");
    }

    service_index = 0; /* We expect to discover only the Heart Rate service as requested. */
    service = &(p_ble_gattc_evt->params.prim_srvc_disc_rsp.services[service_index]);

    if (service->uuid.uuid != BLE_UUID_HEART_RATE_SERVICE)
    {
        printf("Unknown service discovered with UUID: 0x%04X\n", service->uuid.uuid);
        fflush(stdout);
        return;
    }

    m_service_start_handle  = service->handle_range.start_handle;
    m_service_end_handle    = service->handle_range.end_handle;

    printf("Discovered heart rate service. UUID: 0x%04X, "
                   "start handle: 0x%04X, end handle: 0x%04X\n",
        service->uuid.uuid, m_service_start_handle, m_service_end_handle);
    fflush(stdout);

    char_discovery_start();
}

/**@brief Function called on BLE_GATTC_EVT_CHAR_DISC_RSP event.
 *
 * @details Update characteristic state and proceed to discovering the characteristicss descriptors.
 *
 * @param[in] p_ble_gattc_evt Characteristic Discovery Response Event.
 */
static void on_characteristic_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count = p_ble_gattc_evt->params.char_disc_rsp.count;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Characteristic discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    printf("Received characteristic discovery response, characteristics count: %d\n", count);
    fflush(stdout);

    for (int i = 0; i < count; i++)
    {
        printf("Characteristic handle: 0x%04X, UUID: 0x%04X\n",
               p_ble_gattc_evt->params.char_disc_rsp.chars[i].handle_decl,
               p_ble_gattc_evt->params.char_disc_rsp.chars[i].uuid.uuid);
        fflush(stdout);

        if (p_ble_gattc_evt->params.char_disc_rsp.chars[i].uuid.uuid ==
            BLE_UUID_HEART_RATE_MEASUREMENT_CHAR)
        {
            m_hrm_char_handle = p_ble_gattc_evt->params.char_disc_rsp.chars[i].handle_decl;
        }
    }

    descr_discovery_start();
}

/**@brief Function called on BLE_GATTC_EVT_DESC_DISC_RSP event.
 *
 * @details Update CCCD descriptor state and proceed to prompting user to toggle notifications.
 *
 * @param[in] p_ble_gattc_evt Descriptor Discovery Response Event.
 */
static void on_descriptor_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count = p_ble_gattc_evt->params.desc_disc_rsp.count;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Descriptor discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    printf("Received descriptor discovery response, descriptor count: %d\n", count);
    fflush(stdout);

    for (int i = 0; i < count; i++)
    {
        printf("Descriptor handle: 0x%04X, UUID: 0x%04X\n",
               p_ble_gattc_evt->params.desc_disc_rsp.descs[i].handle,
               p_ble_gattc_evt->params.desc_disc_rsp.descs[i].uuid.uuid);
        fflush(stdout);

        if (p_ble_gattc_evt->params.desc_disc_rsp.descs[i].uuid.uuid == BLE_UUID_CCCD)
        {
            m_hrm_cccd_handle = p_ble_gattc_evt->params.desc_disc_rsp.descs[i].handle;
            is_connected = true;
        }
    }
}

/**@brief Function called on BLE_GATTC_EVT_WRITE_RSP event.
 *
 * @param[in] p_ble_gattc_evt Write Response Event.
 */
static void on_write_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    printf("Received write response.\n");
    fflush(stdout);

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Write operation failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
    }
}

/**@brief Function called on BLE_GATTC_EVT_HVX event.
 *
 * @details Logs the received heart rate measurement.
 *
 * @param[in] p_ble_gattc_evt Handle Value Notification/Indication Event.
 */
static void on_hvx(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    if (p_ble_gattc_evt->params.hvx.handle >= m_hrm_char_handle ||
            p_ble_gattc_evt->params.hvx.handle <= m_hrm_cccd_handle) // Heart rate measurement.
    {
        // We know the heart rate reading is encoded as 2 bytes [flag, value].
        printf("Received heart rate measurement: %d\n", p_ble_gattc_evt->params.hvx.data[1]);
    }
    else // Unknown data.
    {
        printf("Un-parsed data received on handle: %04X\n", p_ble_gattc_evt->params.hvx.handle);
    }

    fflush(stdout);
}

/**@brief Function called on BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST event.
 *
 * @details Update GAP connection parameters.
 *
 * @param[in] p_ble_gap_evt Connection Parameter Update Event.
 */
static void on_conn_params_update_request(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code = sd_ble_gap_conn_param_update(m_adapter, m_connection_handle,
                                            &(p_ble_gap_evt->
                                                    params.conn_param_update_request.conn_params));
    if (err_code != NRF_SUCCESS)
    {
        printf("Conn params update failed, err_code %d\n", err_code);
        fflush(stdout);
    }
}

#if NRF_SD_BLE_API >= 3
/**@brief Function called on BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event.
 *
 * @details Replies to an ATT_MTU exchange request by sending an Exchange MTU Response to the client.
 *
 * @param[in] p_ble_gatts_evt Exchange MTU Request Event.
 */
static void on_exchange_mtu_request(const ble_gatts_evt_t * const p_ble_gatts_evt)
{
    uint32_t err_code = sd_ble_gatts_exchange_mtu_reply(
        m_adapter,
        m_connection_handle,
#if NRF_SD_BLE_API < 5
        GATT_MTU_SIZE_DEFAULT);
#else
        BLE_GATT_ATT_MTU_DEFAULT);
#endif

    if (err_code != NRF_SUCCESS)
    {
        printf("MTU exchange request reply failed, err_code %d\n", err_code);
        fflush(stdout);
    }
}

static void on_rssi_changed(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code;
    int8_t p_rssi;
    uint8_t p_ch_index;

    for(uint8_t i=0;i<MAX_PEER_COUNT;i++)
    {
        if (m_connect[i].peer_addr[0] != '\0')
        {
            err_code = sd_ble_gap_rssi_get(m_adapter, m_connect[i].connection_handle, &p_rssi, &p_ch_index);
            if (err_code != NRF_SUCCESS)
            {
                printf("RSSI start failed with error code: 0x%X\n", err_code);
                fflush(stdout);
            }
            printf("RSSI: %d, ch_index: %2d addr: 0x%s\n", p_rssi, p_ch_index, m_connect[i].peer_addr);
            fflush(stdout);
        }
    }
}

/**@brief Function called on BLE_GATTC_EVT_EXCHANGE_MTU_RSP event.
 *
 * @details Logs the new BLE server RX MTU size.
 *
 * @param[in] p_ble_gattc_evt Exchange MTU Response Event.
 */
static void on_exchange_mtu_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    uint16_t server_rx_mtu = p_ble_gattc_evt->params.exchange_mtu_rsp.server_rx_mtu;

    printf("MTU response received. New ATT_MTU is %d\n", server_rx_mtu);
    fflush(stdout);
}
#endif


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
        case BLE_GAP_EVT_CONNECTED:
            on_connected(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_ADV_REPORT:
            on_adv_report(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_TIMEOUT:
            on_timeout(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            on_service_discovery_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            on_characteristic_discovery_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            on_descriptor_discovery_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_HVX:
            on_hvx(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            on_conn_params_update_request(&(p_ble_evt->evt.gap_evt));
            break;
        case BLE_GAP_EVT_PHY_UPDATE:
            printf("Received BLE_GAP_EVT_PHY_UPDATE (RX:%d, TX:%d, status:%d)\n",
                    p_ble_evt->evt.gap_evt.params.phy_update.rx_phy,
                    p_ble_evt->evt.gap_evt.params.phy_update.tx_phy,
                    p_ble_evt->evt.gap_evt.params.phy_update.status);

            fflush(stdout);
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
            on_rssi_changed(&(p_ble_evt->evt.gap_evt));
            break;

    #if NRF_SD_BLE_API >= 3
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            on_exchange_mtu_request(&(p_ble_evt->evt.gatts_evt));
            break;

        case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
            on_exchange_mtu_response(&(p_ble_evt->evt.gattc_evt));
            break;
    #endif

        default:
            printf("Received an un-handled event with ID: %d\n", p_ble_evt->header.evt_id);
            fflush(stdout);
            break;
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
        {"serial_port", optional_argument, NULL, 's'},
        {"phy", optional_argument, NULL, 'p'},
        {"connection", no_argument, NULL, 'c'},
        {0, 0, 0, 0},
    };
    int opt, longindex;
    while ((opt = getopt_long(argc, argv, "s::p::c", longopts, &longindex)) != -1)
    {
        switch (opt)
        {
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
            else if (strcmp(optarg, "2m") == 0)
            {
                m_scan_param.scan_phys = BLE_GAP_PHY_1MBPS;
                m_2m_phy_selected = true;
            }
            else if (strcmp(optarg, "coded") == 0)
            {
                m_scan_param.scan_phys = BLE_GAP_PHY_CODED;
            }
            break;
        case 'c':
            do_connect = true;
            break;
        default:
            break;
        }
    }

    if (do_connect)
    {
        printf("test with connection\n");
    }
    else
    {
        printf("test without connection\n");
    }
    switch (m_scan_param.scan_phys)
    {
    case BLE_GAP_PHY_1MBPS:
        if (m_2m_phy_selected)
        {
            printf("phy(scanning)  : 1M_PHY\nphy(connection): 2M_PHY\n");
        }
        else
        {
            printf("phy: 1M_PHY\n");
        }
        break;

    case BLE_GAP_PHY_CODED:
        printf("phy: CODED_PHY\n");
        break;
    }

    m_connect->connection_handle = 0;
    m_connect->peer_addr[0] = '\0';

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

#if NRF_SD_BLE_API >= 5
    ble_cfg_set(m_config_id);
#endif

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

#if NRF_SD_BLE_API < 5
    error_code = ble_options_set();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }
#endif

    error_code = scan_start();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    // Endlessly loop.
    for (;;)
    {
    }
}
