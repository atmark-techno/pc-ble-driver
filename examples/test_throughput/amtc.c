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

#include <stdio.h>
#include "amt.h"
#include "app_util.h"

#define TX_BUFFER_MASK         0x07                  /**< TX Buffer mask, must be a mask of continuous zeroes, followed by continuous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE         (TX_BUFFER_MASK + 1)  /**< Size of send buffer, which is 1 higher than the mask. */

#define BLE_CCCD_VALUE_LEN     2                     /**< The length of a CCCD value. */
#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */

/** The upper 8 bits of a 16 bit value */
//lint -emacro(572,MSB_16) // Suppress warning 572 "Excessive shift value"
#define MSB_16(a) (((a) & 0xFF00) >> 8)
/** The lower 8 bits (of a 16 bit value) */
#define LSB_16(a) ((a) & 0x00FF)

#define BLE_UUID_CCCD       0x2902
typedef enum
{
    READ_REQ,  /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ  /**< Type identifying that this tx_message is a write request. */
} tx_request_t;

/**@brief Structure for writing a message to the peer, i.e. CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH];  /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                       /**< GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding data to be transmitted to the connected central.
 */
typedef struct
{
    uint16_t     conn_handle;  /**< Connection handle to be used when transmitting this message. */
    tx_request_t type;         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t       read_handle;  /**< Read request message. */
        write_params_t write_req;    /**< Write request message. */
    } req;
} tx_message_t;

static tx_message_t   m_tx_buffer[TX_BUFFER_SIZE];  /**< Transmit buffer for messages to be transmitted to the central. */
static uint32_t       m_tx_insert_index = 0;        /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t       m_tx_index = 0;               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */

static adapter_t * m_adapter = NULL;

static uint16_t     m_service_start_handle       = 0;
static uint16_t     m_service_end_handle         = 0;
static uint16_t     m_char_handle                = 0;

/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
    if (m_tx_index != m_tx_insert_index)
    {
        uint32_t err_code;

        if (m_tx_buffer[m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(m_adapter,
                                         m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        }
        else
        {
            fflush(stdout);
            if (m_tx_buffer[m_tx_index].req.write_req.gattc_params.handle == 0)
            {
                printf("Error. No CCCD handle has been found\n");
                fflush(stdout);
                return;
            }

            err_code = sd_ble_gattc_write(m_adapter,
                                          m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
        }
        fflush(stdout);

        if (err_code == NRF_SUCCESS)
        {
            m_tx_index++;
            m_tx_index &= TX_BUFFER_MASK;
        }
        else
        {
            printf("SD Read/Write API returns error, Error code 0x%X\n", err_code);
            fflush(stdout);
        }
    }
}

/**@brief Function called upon connecting to BLE peripheral.
 *
 * @details Initiates primary service discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t service_discovery_start(nrf_ble_amtc_t * p_ctx)
{
    uint32_t   err_code;
    uint16_t   start_handle = 0x01;
    ble_uuid_t srvc_uuid;

    srvc_uuid.type = BLE_UUID_TYPE_BLE;
    srvc_uuid.uuid = AMT_SERVICE_UUID;

    // Initiate procedure to find the primary AMT_SERVICE_UUID.
    err_code = sd_ble_gattc_primary_services_discover(m_adapter,
                                                      p_ctx->conn_handle, start_handle,
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
 * @details Initiates service's characteristic discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t char_discovery_start(nrf_ble_amtc_t * p_ctx)
{
    ble_gattc_handle_range_t handle_range;

    handle_range.start_handle = m_service_start_handle;
    handle_range.end_handle = m_service_end_handle;

    return sd_ble_gattc_characteristics_discover(m_adapter, p_ctx->conn_handle, &handle_range);
}

/**@brief Function called upon discovering service's characteristics.
 *
 * @details Initiates m_char_handle characteristic's descriptor discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t descr_discovery_start(nrf_ble_amtc_t * p_ctx)
{
    ble_gattc_handle_range_t handle_range;

    if (m_char_handle == 0)
    {
        printf("No characteristic handle found\n");
        fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    handle_range.start_handle   = m_char_handle;
    handle_range.end_handle     = m_service_end_handle;

    return sd_ble_gattc_descriptors_discover(m_adapter, p_ctx->conn_handle, &handle_range);
}

/** Event functions */

/**@brief Function called on BLE_GAP_EVT_CONNECTED event.
 *
 * @details Update connection state and proceed to discovering the peer's GATT services.
 *
 * @param[in] p_ble_gap_evt GAP event.
 */
static void on_connected(nrf_ble_amtc_t * p_ctx, const ble_evt_t * p_ble_evt)
{
    nrf_ble_amtc_evt_t evt;
    p_ctx->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    evt.evt_type = NRF_BLE_AMT_C_EVT_NOT_DISCOVERY_COMPLETE;
    p_ctx->evt_handler(p_ctx, &evt);
}

/**@brief Function called on BLE_GATTC_EVT_DESC_DISC_RSP event.
 *
 * @details Update CCCD descriptor state and proceed to prompting user to toggle notifications.
 *
 * @param[in] p_ble_gattc_evt Descriptor Discovery Response Event.
 */
static void on_descriptor_discovery_response(nrf_ble_amtc_t * p_ctx, const ble_evt_t * p_ble_evt)
{
    int count = p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count;

    if (p_ble_evt->evt.gattc_evt.gatt_status != NRF_SUCCESS)
    {
        printf("Descriptor discovery failed. Error code 0x%X\n", p_ble_evt->evt.gattc_evt.gatt_status);
        fflush(stdout);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        if (p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid == BLE_UUID_CCCD)
        {
            p_ctx->peer_db.amt_cccd_handle =  p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].handle;
        }
        if (p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid == AMTS_CHAR_UUID)
        {
            p_ctx->peer_db.amt_handle = p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].handle;
        }
        if (p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid == AMT_RCV_BYTES_CNT_CHAR_UUID)
        {
            p_ctx->peer_db.amt_rbc_handle = p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].handle;
        }
    }

    nrf_ble_amtc_evt_t evt;
    evt.evt_type = NRF_BLE_AMT_C_EVT_DISCOVERY_COMPLETE;
    p_ctx->evt_handler(p_ctx, &evt);
}

/**@brief Function called on BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP event.
 *
 * @details Update service state and proceed to discovering the service's GATT characteristics.
 *
 * @param[in] p_ble_gattc_evt Primary Service Discovery Response Event.
 */
static void on_service_discovery_response(nrf_ble_amtc_t * p_ctx, const ble_evt_t * p_ble_evt)
{
    int count;
    int service_index;
    const ble_gattc_service_t * service;

    if (p_ble_evt->evt.gattc_evt.gatt_status != NRF_SUCCESS)
    {
        printf("Service discovery failed. Error code 0x%X\n", p_ble_evt->evt.gattc_evt.gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count;

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

    service_index = 0; /* We expect to discover only the AMT service as requested. */
    service = &(p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[service_index]);

    if (service->uuid.uuid != AMT_SERVICE_UUID)
    {
        printf("Unknown service discovered with UUID: 0x%04X\n", service->uuid.uuid);
        fflush(stdout);
        return;
    }

    m_service_start_handle  = service->handle_range.start_handle;
    m_service_end_handle    = service->handle_range.end_handle;

    char_discovery_start(p_ctx);
}

/**@brief Function called on BLE_GATTC_EVT_CHAR_DISC_RSP event.
 *
 * @details Update characteristic state and proceed to discovering the characteristicss descriptors.
 *
 * @param[in] p_ble_gattc_evt Characteristic Discovery Response Event.
 */
static void on_characteristic_discovery_response(nrf_ble_amtc_t * p_ctx, const ble_evt_t * p_ble_evt)
{
    int count = p_ble_evt->evt.gattc_evt.params.char_disc_rsp.count;

    if (p_ble_evt->evt.gattc_evt.gatt_status != NRF_SUCCESS)
    {
        printf("Characteristic discovery failed. Error code 0x%X\n", p_ble_evt->evt.gattc_evt.gatt_status);
        fflush(stdout);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        if (p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.uuid ==
            AMTS_CHAR_UUID)
        {
            m_char_handle = p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_decl;
        }
    }

    descr_discovery_start(p_ctx);
}

/**@brief     Function for handling write response event received from the SoftDevice.
 *
 * @details   This function will uses the read response received from the SoftDevice
 *            and checks if it is a write response of the AMT from the peer.
 *
 * @param[in] p_ctx        Pointer to the AMT Client structure.
 * @param[in] p_ble_evt    Pointer to the BLE event received.
 */
static void on_write_response(nrf_ble_amtc_t * p_ctx, ble_evt_t const * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ctx->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }

    // Check if this is a write response on the CCCD.
    if (p_ble_evt->evt.gattc_evt.params.write_rsp.handle == p_ctx->peer_db.amt_cccd_handle)
    {
        nrf_ble_amtc_evt_t amt_c_evt;
        amt_c_evt.evt_type = NRF_BLE_AMT_C_EVT_CCCD_CONFIGURED;
        p_ctx->evt_handler(p_ctx, &amt_c_evt);
    }
}

/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the AMT from the peer.
 *            If it is, this function will send an event to the peer.
 *
 * @param[in] p_ctx        Pointer to the AMT Client structure.
 * @param[in] p_ble_evt    Pointer to the BLE event received.
 */
static void on_hvx(nrf_ble_amtc_t * p_ctx, ble_evt_t const * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ctx->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }

    // Check if this is a AMT notification.
    if (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ctx->peer_db.amt_handle)
    {
        nrf_ble_amtc_evt_t amt_c_evt;
        p_ctx->bytes_rcvd_cnt           += p_ble_evt->evt.gattc_evt.params.hvx.len;
        amt_c_evt.evt_type              = NRF_BLE_AMT_C_EVT_NOTIFICATION;
        amt_c_evt.conn_handle           = p_ble_evt->evt.gattc_evt.conn_handle;
        amt_c_evt.params.hvx.notif_len  = p_ble_evt->evt.gattc_evt.params.hvx.len;
        amt_c_evt.params.hvx.bytes_sent = uint32_decode(p_ble_evt->evt.gattc_evt.params.hvx.data);
        amt_c_evt.params.hvx.bytes_rcvd = p_ctx->bytes_rcvd_cnt;

        p_ctx->evt_handler(p_ctx, &amt_c_evt);
    }
}


/**@brief     Function for handling read response event received from the SoftDevice.
 *
 * @details   This function will uses the read response received from the SoftDevice
 *            and checks if it is a read response of the AMT from the peer.
 *            If it is, this function will send an event to the peer.
 *
 * @param[in] p_ctx        Pointer to the AMT Client structure.
 * @param[in] p_ble_evt    Pointer to the BLE event received.
 */
static void on_read_response(nrf_ble_amtc_t * p_ctx, ble_evt_t const * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ctx->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }

    // Check if this is a AMT RCB read response.
    if (p_ble_evt->evt.gattc_evt.params.read_rsp.handle == p_ctx->peer_db.amt_rbc_handle)
    {
        nrf_ble_amtc_evt_t amt_c_evt;
        amt_c_evt.evt_type             = NRF_BLE_AMT_C_EVT_RCB_READ_RSP;
        amt_c_evt.conn_handle          = p_ble_evt->evt.gattc_evt.conn_handle;
        amt_c_evt.params.rcv_bytes_cnt = uint32_decode(p_ble_evt->evt.gattc_evt.params.read_rsp.data);
        p_ctx->evt_handler(p_ctx, &amt_c_evt);
    }
}

void nrf_ble_amtc_on_ble_evt(nrf_ble_amtc_t * p_ctx, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received an empty AMTC event\n");
        fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connected(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            on_service_discovery_response(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            on_characteristic_discovery_response(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            on_descriptor_discovery_response(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_response(p_ctx, p_ble_evt);
            break;

        case BLE_GATTC_EVT_READ_RSP:
            fflush(stdout);
            on_read_response(p_ctx, p_ble_evt);
            break;

        default:
            break;
    }

    // Check if the event if on the link for this instance
    if (p_ctx->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        printf("on the link for this instance\n");
        fflush(stdout);
        return;
    }

    // Check if there is any message to be sent across to the peer and send it.
    tx_buffer_process();
}

ret_code_t nrf_ble_amtc_init(adapter_t * adapter, nrf_ble_amtc_t * p_ctx, nrf_ble_amtc_evt_handler_t evt_handler)
{
    m_adapter = adapter;
    if(p_ctx==NULL)
    {
        return NRF_ERROR_NULL;
    }
    if(evt_handler==NULL)
    {
        return NRF_ERROR_NULL;
    }

    ble_uuid128_t base_uuid = {SERVICE_UUID_BASE};

    ret_code_t err_code = sd_ble_uuid_vs_add(m_adapter, &base_uuid, &p_ctx->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        printf("sd_ble_uuid_vs_add() Failed, reason %d\n", err_code);
        fflush(stdout);
        return NRF_ERROR_NULL;
    }

    p_ctx->evt_handler                  = evt_handler;
    p_ctx->bytes_rcvd_cnt               = 0;
    p_ctx->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_ctx->peer_db.amt_cccd_handle      = BLE_GATT_HANDLE_INVALID;
    p_ctx->peer_db.amt_handle           = BLE_GATT_HANDLE_INVALID;
    p_ctx->peer_db.amt_rbc_handle       = BLE_GATT_HANDLE_INVALID;

    return NRF_SUCCESS;
}

ret_code_t nrf_ble_amtc_handles_assign(nrf_ble_amtc_t   * p_ctx,
                                      nrf_ble_amtc_db_t * p_peer_handles)
{
    if(p_ctx==NULL)
    {
        return NRF_ERROR_NULL;
    }
    if (p_peer_handles != NULL)
    {
        p_ctx->peer_db = *p_peer_handles;
    }

    return NRF_SUCCESS;
}

/**@brief Function for creating a message for writing to the CCCD.
 */
static ret_code_t cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable)
{
    if (handle_cccd == 0)
    {
        printf("Error. No CCCD handle has been found\n");
        fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();

    return NRF_SUCCESS;
}

ret_code_t nrf_ble_amtc_notif_enable(nrf_ble_amtc_t * p_ctx)
{
    if(p_ctx==NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (p_ctx->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    return cccd_configure(p_ctx->conn_handle, p_ctx->peer_db.amt_cccd_handle, true);
}

ret_code_t nrf_ble_amtc_rcb_read(nrf_ble_amtc_t * p_ctx)
{
    if (p_ctx==NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (p_ctx->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    tx_message_t * p_msg = &m_tx_buffer[m_tx_insert_index++];

    p_msg->req.read_handle = p_ctx->peer_db.amt_rbc_handle;
    p_msg->conn_handle     = p_ctx->conn_handle;
    p_msg->type            = READ_REQ;

    m_tx_insert_index &= TX_BUFFER_MASK;
    tx_buffer_process();

    return NRF_SUCCESS;
}
