/* cfl_service_danp.c - CFL Service over DANP Transport */

/* All Rights Reserved */

/* Includes */

#include "cfl/services/cfl_service_danp.h"
#include "cfl/cfl.h"
#include <string.h>
#include <stdbool.h>

/* Private Definitions */

#define CFL_DANP_RX_BUF_SIZE (CFL_HEADER_SIZE + CFL_MAX_PAYLOAD_SIZE)
#define CFL_DANP_TX_BUF_SIZE (CFL_HEADER_SIZE + CFL_MAX_PAYLOAD_SIZE)

/* Private Types */

typedef struct cfl_handler_entry_s {
    uint16_t id;
    cfl_request_handler_t handler;
    void *user_data;
    bool in_use;
} cfl_handler_entry_t;

typedef struct cfl_service_danp_ctx_s {
    bool initialized;
    bool running;
    uint16_t local_port;
    danp_socket_t *socket;
    osalThreadHandle_t rx_task_handle;
    cfl_handler_entry_t handlers[CFL_DANP_MAX_HANDLERS];
    uint16_t next_seq;
    uint8_t rx_buf[CFL_DANP_RX_BUF_SIZE];
    uint8_t tx_buf[CFL_DANP_TX_BUF_SIZE];
} cfl_service_danp_ctx_t;

/* Private Variables */

static cfl_service_danp_ctx_t g_ctx;

/* Private Function Declarations */

static void cfl_danp_rx_task(void *arg);
static void cfl_process_message(uint16_t src_node, uint16_t src_port, const uint8_t *data,
                                uint16_t len);
static cfl_handler_entry_t *cfl_find_handler(uint16_t id);
static int32_t cfl_send_raw(uint16_t dst_node, uint16_t dst_port, const cfl_header_t *hdr,
                            const uint8_t *payload, uint16_t payload_len);

/* Public Functions */

int32_t cfl_service_danp_init(const cfl_service_danp_config_t *config)
{
    int32_t ret;

    if (config == NULL) {
        return CFL_ERR_NULL;
    }

    if (g_ctx.initialized) {
        return CFL_ERR_ALREADY_INIT;
    }

    memset(&g_ctx, 0, sizeof(g_ctx));

    g_ctx.local_port = config->port_id;
    g_ctx.next_seq = 0;

    /* Create DGRAM socket - CFL handles its own reliability via ACK/NACK */
    g_ctx.socket = danp_socket(DANP_TYPE_DGRAM);
    if (g_ctx.socket == NULL) {
        return CFL_ERR_TRANSPORT;
    }

    /* Bind to local port */
    ret = danp_bind(g_ctx.socket, g_ctx.local_port);
    if (ret < 0) {
        danp_close(g_ctx.socket);
        g_ctx.socket = NULL;
        return CFL_ERR_TRANSPORT;
    }

    g_ctx.running = true;

    /* Create RX task */
    osalThreadAttr_t task_attr = {
        .name = "cfl_rx",
        .priority = CFL_DANP_RX_TASK_PRIORITY,
        .stack_size = CFL_DANP_RX_TASK_STACK_SIZE,
    };

    g_ctx.rx_task_handle = osalThreadNew(cfl_danp_rx_task, &g_ctx, &task_attr);
    if (g_ctx.rx_task_handle == NULL) {
        danp_close(g_ctx.socket);
        g_ctx.socket = NULL;
        g_ctx.running = false;
        return CFL_ERR_NO_RESOURCE;
    }

    g_ctx.initialized = true;

    return CFL_OK;
}

int32_t cfl_service_danp_deinit(void)
{
    if (!g_ctx.initialized) {
        return CFL_ERR_NOT_INIT;
    }

    /* Signal RX task to stop */
    g_ctx.running = false;

    /* Wait for task to exit (it will timeout on recv and check running flag) */
    osalDelay(CFL_DANP_RX_TIMEOUT_MS * 2);

    /* Close socket */
    if (g_ctx.socket != NULL) {
        danp_close(g_ctx.socket);
        g_ctx.socket = NULL;
    }

    memset(&g_ctx, 0, sizeof(g_ctx));

    return CFL_OK;
}

int32_t cfl_service_danp_register_handler(uint16_t id, cfl_request_handler_t handler,
                                          void *user_data)
{
    if (!g_ctx.initialized) {
        return CFL_ERR_NOT_INIT;
    }

    if (handler == NULL) {
        return CFL_ERR_NULL;
    }

    /* Check if handler already exists for this ID */
    cfl_handler_entry_t *entry = cfl_find_handler(id);
    if (entry != NULL) {
        return CFL_ERR_EXISTS;
    }

    /* Find free slot */
    for (uint8_t i = 0; i < CFL_DANP_MAX_HANDLERS; i++) {
        if (!g_ctx.handlers[i].in_use) {
            g_ctx.handlers[i].id = id;
            g_ctx.handlers[i].handler = handler;
            g_ctx.handlers[i].user_data = user_data;
            g_ctx.handlers[i].in_use = true;
            return CFL_OK;
        }
    }

    return CFL_ERR_NO_RESOURCE;
}

int32_t cfl_service_danp_unregister_handler(uint16_t id)
{
    if (!g_ctx.initialized) {
        return CFL_ERR_NOT_INIT;
    }

    cfl_handler_entry_t *entry = cfl_find_handler(id);
    if (entry == NULL) {
        return CFL_ERR_NOT_FOUND;
    }

    memset(entry, 0, sizeof(cfl_handler_entry_t));

    return CFL_OK;
}

int32_t cfl_service_danp_send_request(uint16_t dst_node, uint16_t dst_port, uint16_t id,
                                      const uint8_t *payload, uint16_t payload_len,
                                      uint16_t *seq_out)
{
    cfl_header_t hdr;

    if (!g_ctx.initialized) {
        return CFL_ERR_NOT_INIT;
    }

    if (payload_len > 0 && payload == NULL) {
        return CFL_ERR_NULL;
    }

    if (payload_len > CFL_MAX_PAYLOAD_SIZE) {
        return CFL_ERR_LENGTH;
    }

    /* Build request header */
    cfl_build_request(&hdr, id, g_ctx.next_seq);
    hdr.length = payload_len;
    cfl_header_compute_crc(&hdr, payload, payload_len);

    if (seq_out != NULL) {
        *seq_out = g_ctx.next_seq;
    }
    g_ctx.next_seq++;

    return cfl_send_raw(dst_node, dst_port, &hdr, payload, payload_len);
}

int32_t cfl_service_danp_send_push(uint16_t dst_node, uint16_t dst_port, uint16_t id,
                                   const uint8_t *payload, uint16_t payload_len)
{
    cfl_header_t hdr;

    if (!g_ctx.initialized) {
        return CFL_ERR_NOT_INIT;
    }

    if (payload_len > 0 && payload == NULL) {
        return CFL_ERR_NULL;
    }

    if (payload_len > CFL_MAX_PAYLOAD_SIZE) {
        return CFL_ERR_LENGTH;
    }

    /* Build push header */
    cfl_build_push(&hdr, id, g_ctx.next_seq++);
    hdr.length = payload_len;
    cfl_header_compute_crc(&hdr, payload, payload_len);

    return cfl_send_raw(dst_node, dst_port, &hdr, payload, payload_len);
}

/* Private Functions */

static cfl_handler_entry_t *cfl_find_handler(uint16_t id)
{
    for (uint8_t i = 0; i < CFL_DANP_MAX_HANDLERS; i++) {
        if (g_ctx.handlers[i].in_use && g_ctx.handlers[i].id == id) {
            return &g_ctx.handlers[i];
        }
    }
    return NULL;
}

static int32_t cfl_send_raw(uint16_t dst_node, uint16_t dst_port, const cfl_header_t *hdr,
                            const uint8_t *payload, uint16_t payload_len)
{
    uint16_t total_len;
    int32_t ret;

    /* Serialize to TX buffer */
    total_len = cfl_serialize(hdr, payload, payload_len, g_ctx.tx_buf, sizeof(g_ctx.tx_buf));
    if (total_len == 0) {
        return CFL_ERR_BUFFER;
    }

    /* Send via DANP */
    ret = danp_send_to(g_ctx.socket, g_ctx.tx_buf, total_len, dst_node, dst_port);
    if (ret < 0) {
        return CFL_ERR_TRANSPORT;
    }

    return CFL_OK;
}

static void cfl_danp_rx_task(void *arg)
{
    cfl_service_danp_ctx_t *ctx = (cfl_service_danp_ctx_t *)arg;
    int32_t len;
    uint16_t src_node;
    uint16_t src_port;

    while (ctx->running) {
        len = danp_recv_from(ctx->socket, ctx->rx_buf, sizeof(ctx->rx_buf), &src_node, &src_port,
                             CFL_DANP_RX_TIMEOUT_MS);

        if (len > 0) {
            cfl_process_message(src_node, src_port, ctx->rx_buf, (uint16_t)len);
        }
        /* On timeout or error, loop continues and checks running flag */
    }

    /* Task cleanup - terminate self */
    osalThreadExit();
}

static void cfl_process_message(uint16_t src_node, uint16_t src_port, const uint8_t *data,
                                uint16_t len)
{
    cfl_header_t hdr;
    const uint8_t *payload;
    uint16_t payload_len;
    cfl_error_t err;
    cfl_handler_entry_t *entry;
    cfl_response_t response;

    if (data == NULL || len < CFL_HEADER_SIZE) {
        return;
    }

    /* Deserialize header */
    err = cfl_deserialize(data, len, &hdr, &payload, &payload_len);
    if (err != CFL_OK) {
        return;
    }

    /* Validate complete message including CRC */
    err = cfl_message_validate(&hdr, payload, payload_len);
    if (err != CFL_OK) {
        return;
    }

    /* Handle based on message type */
    if (cfl_header_has_flag(&hdr, CFL_F_RQST)) {
        /* Find handler for this request ID */
        entry = cfl_find_handler(hdr.id);
        if (entry == NULL) {
            /* No handler - send NACK */
            cfl_header_t nack_hdr;
            cfl_build_nack(&nack_hdr, &hdr);
            cfl_header_compute_crc(&nack_hdr, NULL, 0);
            cfl_send_raw(src_node, src_port, &nack_hdr, NULL, 0);
            return;
        }

        /* Initialize response */
        memset(&response, 0, sizeof(response));
        response.type = CFL_RESP_NONE;

        /* Call handler */
        entry->handler(src_node, src_port, &hdr, payload, payload_len, &response, entry->user_data);

        /* Send response based on handler result */
        cfl_header_t resp_hdr;

        switch (response.type) {
        case CFL_RESP_REPLY:
            cfl_build_reply(&resp_hdr, &hdr);
            resp_hdr.length = response.payload_len;
            if (response.flags != 0) {
                cfl_header_set_flags(&resp_hdr, response.flags);
            }
            cfl_header_compute_crc(&resp_hdr, response.payload, response.payload_len);
            cfl_send_raw(src_node, src_port, &resp_hdr, response.payload, response.payload_len);
            break;

        case CFL_RESP_ACK:
            cfl_build_ack(&resp_hdr, &hdr);
            cfl_header_compute_crc(&resp_hdr, NULL, 0);
            cfl_send_raw(src_node, src_port, &resp_hdr, NULL, 0);
            break;

        case CFL_RESP_NACK:
            cfl_build_nack(&resp_hdr, &hdr);
            cfl_header_compute_crc(&resp_hdr, NULL, 0);
            cfl_send_raw(src_node, src_port, &resp_hdr, NULL, 0);
            break;

        case CFL_RESP_NONE:
        default:
            /* No response requested */
            break;
        }
    } else if (cfl_header_has_flag(&hdr, CFL_F_PUSH)) {
        /* Push messages - find handler and call without response */
        entry = cfl_find_handler(hdr.id);
        if (entry != NULL) {
            entry->handler(src_node, src_port, &hdr, payload, payload_len, NULL, entry->user_data);
        }
    }
    /* TODO: Handle ACK/NACK/REPLY for async request tracking if needed */
}
