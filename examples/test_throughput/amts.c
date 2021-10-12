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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "amt.h"
#include "ble.h"
#include "app_util.h"

#define OPCODE_LENGTH 1     /**< Length of opcode inside a notification. */
#define HANDLE_LENGTH 2     /**< Length of handle inside a notification. */

static adapter_t * m_adapter = NULL;
static uint16_t    m_att_mtu_size;

void nrf_ble_amts_init(adapter_t * adapter, uint16_t att_mtu_size, nrf_ble_amts_t * p_ctx, amts_evt_handler_t evt_handler)
{
    uint32_t      err_code;
    uint16_t      service_handle;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t base_uuid = {SERVICE_UUID_BASE};
    m_adapter = adapter;
    m_att_mtu_size = att_mtu_size;

    err_code = sd_ble_uuid_vs_add(m_adapter, &base_uuid, &(p_ctx->uuid_type));
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed sd_ble_uuid_vs_add. Error code: 0x%02X\n", err_code);
        fflush(stdout);
        return;
    }

    BLE_UUID_BLE_ASSIGN(ble_uuid, AMT_SERVICE_UUID);

    // Add service.
    err_code = sd_ble_gatts_service_add(m_adapter, BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &service_handle);
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed sd_ble_gatts_service_add. Error code: 0x%02X\n", err_code);
        fflush(stdout);
        return;
    }
    fflush(stdout);

    // Add AMTS characteristic.
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, AMTS_CHAR_UUID);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = m_att_mtu_size;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = m_att_mtu_size;

    err_code = sd_ble_gatts_characteristic_add(m_adapter, service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &(p_ctx->amts_char_handles));
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to initialize characteristics. Error code: 0x%02X\n", err_code);
        fflush(stdout);
        return;
    }

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    BLE_UUID_BLE_ASSIGN(ble_uuid, AMT_RCV_BYTES_CNT_CHAR_UUID);

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = AMT_RCV_BYTES_CNT_MAX_LEN;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = AMT_RCV_BYTES_CNT_MAX_LEN;

    err_code = sd_ble_gatts_characteristic_add(m_adapter, service_handle,
                                                 &char_md,
                                                 &attr_char_value,
                                                 &(p_ctx->amt_rbc_char_handles));

    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to initialize characteristics rbc. Error code: 0x%02X\n", err_code);
        fflush(stdout);
        return;
    }

    p_ctx->evt_handler = evt_handler;

    return;
}

static void char_notification_send(nrf_ble_amts_t * p_ctx)
{
    uint8_t            data[256];
    uint16_t           payload_len = m_att_mtu_size - OPCODE_LENGTH - HANDLE_LENGTH;
    nrf_ble_amts_evt_t evt;

    ble_gatts_hvx_params_t const hvx_param =
    {
        .type   = BLE_GATT_HVX_NOTIFICATION,
        .handle = p_ctx->amts_char_handles.value_handle,
        .p_data = data,
        .p_len  = &payload_len,
    };

    uint32_t err_code = NRF_SUCCESS;
    while (err_code == NRF_SUCCESS)
    {
        (void)uint32_encode(p_ctx->bytes_sent, data);

        err_code = sd_ble_gatts_hvx(m_adapter, p_ctx->conn_handle, &hvx_param);

        if (err_code == NRF_ERROR_RESOURCES)
        {
            // Wait for BLE_GATTS_EVT_HVN_TX_COMPLETE.
            p_ctx->busy = true;
            break;
        }
        else if (err_code != NRF_SUCCESS)
        {
            printf("sd_ble_gatts_hvx() failed: 0x%x\n", err_code);
            fflush(stdout);
        }
        p_ctx->bytes_sent += payload_len;

        if (p_ctx->kbytes_sent != (p_ctx->bytes_sent / 1024))
        {
            p_ctx->kbytes_sent = (p_ctx->bytes_sent / 1024);

            evt.evt_type             = NRF_BLE_AMTS_EVT_TRANSFER_1KB;
            evt.bytes_transfered_cnt = p_ctx->bytes_sent;
            p_ctx->evt_handler(evt);
        }

        if (p_ctx->bytes_sent >= AMT_BYTE_TRANSFER_CNT_DEFAULT)
        {
            evt.bytes_transfered_cnt = p_ctx->bytes_sent;

            evt.evt_type = NRF_BLE_AMTS_EVT_TRANSFER_FINISHED;
            p_ctx->evt_handler(evt);
            break;
        }
    }
}

void nrf_ble_amts_notif_spam(nrf_ble_amts_t * p_ctx)
{
    p_ctx->kbytes_sent = 0;
    p_ctx->bytes_sent  = 0;
    char_notification_send(p_ctx);
}

void nrf_ble_amts_rbc_set(nrf_ble_amts_t * p_ctx, uint32_t byte_cnt)
{
    uint8_t  data[AMT_RCV_BYTES_CNT_MAX_LEN];
    uint16_t len;

    ble_gatts_value_t value_param;

    memset(&value_param, 0x00, sizeof(value_param));

    len                 = (uint16_t)uint32_encode(byte_cnt, data);
    value_param.len     = len;
    value_param.p_value = data;

    ret_code_t err_code = sd_ble_gatts_value_set(m_adapter,
                                                 p_ctx->conn_handle,
                                                 p_ctx->amt_rbc_char_handles.value_handle,
                                                 &value_param);
    if (err_code != NRF_SUCCESS)
    {
        printf("sd_ble_gatts_value_set() failed: 0x%x\n", err_code);
        fflush(stdout);
    }
}

/**@brief Function for handling the Connect event.
 *
 * @param     p_ctx       Pointer to the AMTS structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_connect(nrf_ble_amts_t * p_ctx, ble_evt_t * p_ble_evt)
{
    p_ctx->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Write event.
 *
 * @param     p_ctx       Pointer to the AMTS structure.
 * @param[in] p_ble_evt   Event received from the BLE stack.
 */
static void on_write(nrf_ble_amts_t * p_ctx, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_ctx->amts_char_handles.cccd_handle) && (p_evt_write->len == 2))
    {
        // CCCD written, call the application event handler.
        nrf_ble_amts_evt_t evt;

        evt.evt_type = NRF_BLE_AMTS_EVT_NOTIF_ENABLED;
        p_ctx->evt_handler(evt);
    }
}

/**@brief Function for handling the TX_COMPLETE event.
 *
 * @param   p_ctx   Pointer to the AMTS structure.
 */
static void on_tx_complete(nrf_ble_amts_t * p_ctx)
{
    if (p_ctx->busy)
    {
        p_ctx->busy = false;
        char_notification_send(p_ctx);
    }
}

void nrf_ble_amts_on_ble_evt(nrf_ble_amts_t * p_ctx, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received an empty AMTS event\n");
        fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ctx, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ctx, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_tx_complete(p_ctx);
            break;

        default:
            break;
    }
}
