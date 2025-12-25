/* cfl.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include "cfl/cfl.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Imports */


/* Definitions */

#define CFL_CRC_OFFSET  (offsetof(cfl_header_t, crc))

/* Types */


/* Forward Declarations */


/* Variables */


/* Functions */


uint16_t cfl_crc16_continue(uint16_t crc, const uint8_t *data, uint16_t len)
{
    if (data == NULL) {
        return crc;
    }

    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CFL_CRC_POLY;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint16_t cfl_crc16(const uint8_t *data, uint16_t len)
{
    return cfl_crc16_continue(CFL_CRC_INIT, data, len);
}


void cfl_header_init(cfl_header_t *hdr, uint16_t id, uint8_t flags)
{
    if (hdr == NULL) {
        return;
    }

    memset(hdr, 0, sizeof(cfl_header_t));
    hdr->sync    = CFL_SYNC_WORD;
    hdr->version = CFL_VERSION;
    hdr->flags   = flags;
    hdr->id      = id;
    hdr->seq     = 0;
    hdr->length  = 0;
    hdr->crc     = 0;
}

void cfl_header_set_seq(cfl_header_t *hdr, uint16_t seq)
{
    if (hdr == NULL) {
        return;
    }
    hdr->seq = seq;
}

void cfl_header_set_length(cfl_header_t *hdr, uint16_t length)
{
    if (hdr == NULL) {
        return;
    }
    hdr->length = length;
}

void cfl_header_compute_crc(cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len)
{
    uint16_t crc;

    if (hdr == NULL) {
        return;
    }

    /* CRC covers header (excluding crc field) + payload */
    crc = cfl_crc16((const uint8_t *)hdr, CFL_CRC_OFFSET);

    if (payload != NULL && payload_len > 0) {
        crc = cfl_crc16_continue(crc, payload, payload_len);
    }

    hdr->crc = crc;
}

/* Flag Functions */

void cfl_header_set_flags(cfl_header_t *hdr, uint8_t flags)
{
    if (hdr == NULL) {
        return;
    }
    hdr->flags |= flags;
}

void cfl_header_clear_flags(cfl_header_t *hdr, uint8_t flags)
{
    if (hdr == NULL) {
        return;
    }
    hdr->flags &= ~flags;
}

uint8_t cfl_header_has_flag(const cfl_header_t *hdr, uint8_t flag)
{
    if (hdr == NULL) {
        return 0;
    }
    return (hdr->flags & flag) != 0;
}

/* Validation Functions */

cfl_error_t cfl_header_validate(const cfl_header_t *hdr)
{
    if (hdr == NULL) {
        return CFL_ERR_NULL;
    }

    if (hdr->sync != CFL_SYNC_WORD) {
        return CFL_ERR_SYNC;
    }

    if (hdr->version != CFL_VERSION) {
        return CFL_ERR_VERSION;
    }

    if (hdr->length > CFL_MAX_PAYLOAD_SIZE) {
        return CFL_ERR_LENGTH;
    }

    return CFL_OK;
}

cfl_error_t cfl_message_validate(const cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len)
{
    cfl_error_t err;
    uint16_t computed_crc;

    err = cfl_header_validate(hdr);
    if (err != CFL_OK) {
        return err;
    }

    if (hdr->length != payload_len) {
        return CFL_ERR_LENGTH;
    }

    if (hdr->length > 0 && payload == NULL) {
        return CFL_ERR_NULL;
    }

    /* Compute and verify CRC */
    computed_crc = cfl_crc16((const uint8_t *)hdr, CFL_CRC_OFFSET);
    if (payload != NULL && payload_len > 0) {
        computed_crc = cfl_crc16_continue(computed_crc, payload, payload_len);
    }

    if (computed_crc != hdr->crc) {
        return CFL_ERR_CRC;
    }

    return CFL_OK;
}

/* Serialization Functions */

uint16_t cfl_serialize(const cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len,
                       uint8_t *buf, uint16_t buf_size)
{
    uint16_t total_len;

    if (hdr == NULL || buf == NULL) {
        return 0;
    }

    total_len = CFL_HEADER_SIZE + payload_len;

    if (buf_size < total_len) {
        return 0;
    }

    /* Copy header */
    memcpy(buf, hdr, CFL_HEADER_SIZE);

    /* Copy payload if present */
    if (payload != NULL && payload_len > 0) {
        memcpy(buf + CFL_HEADER_SIZE, payload, payload_len);
    }

    return total_len;
}

cfl_error_t cfl_deserialize(const uint8_t *buf, uint16_t buf_len,
                            cfl_header_t *hdr, const uint8_t **payload, uint16_t *payload_len)
{
    cfl_error_t err;

    if (buf == NULL || hdr == NULL) {
        return CFL_ERR_NULL;
    }

    if (buf_len < CFL_HEADER_SIZE) {
        return CFL_ERR_BUFFER;
    }

    /* Extract header */
    memcpy(hdr, buf, CFL_HEADER_SIZE);

    /* Basic header validation */
    err = cfl_header_validate(hdr);
    if (err != CFL_OK) {
        return err;
    }

    /* Check buffer has enough data for payload */
    if (buf_len < CFL_HEADER_SIZE + hdr->length) {
        return CFL_ERR_BUFFER;
    }

    /* Set payload pointer and length */
    if (payload != NULL) {
        *payload = (hdr->length > 0) ? (buf + CFL_HEADER_SIZE) : NULL;
    }

    if (payload_len != NULL) {
        *payload_len = hdr->length;
    }

    return CFL_OK;
}

/* Message Builder Functions */

cfl_error_t cfl_build_request(cfl_header_t *hdr, uint16_t id, uint16_t seq)
{
    if (hdr == NULL) {
        return CFL_ERR_NULL;
    }

    cfl_header_init(hdr, id, CFL_F_RQST);
    hdr->seq = seq;

    return CFL_OK;
}

cfl_error_t cfl_build_reply(cfl_header_t *hdr, const cfl_header_t *request)
{
    if (hdr == NULL || request == NULL) {
        return CFL_ERR_NULL;
    }

    cfl_header_init(hdr, request->id, CFL_F_RPLY);
    hdr->seq = request->seq;

    return CFL_OK;
}

cfl_error_t cfl_build_ack(cfl_header_t *hdr, const cfl_header_t *request)
{
    if (hdr == NULL || request == NULL) {
        return CFL_ERR_NULL;
    }

    cfl_header_init(hdr, request->id, CFL_F_ACK);
    hdr->seq = request->seq;

    return CFL_OK;
}

cfl_error_t cfl_build_nack(cfl_header_t *hdr, const cfl_header_t *request)
{
    if (hdr == NULL || request == NULL) {
        return CFL_ERR_NULL;
    }

    cfl_header_init(hdr, request->id, CFL_F_NACK);
    hdr->seq = request->seq;

    return CFL_OK;
}

cfl_error_t cfl_build_push(cfl_header_t *hdr, uint16_t id, uint16_t seq)
{
    if (hdr == NULL) {
        return CFL_ERR_NULL;
    }

    cfl_header_init(hdr, id, CFL_F_PUSH);
    hdr->seq = seq;

    return CFL_OK;
}

/* Utility Functions */

const char *cfl_error_str(cfl_error_t err)
{
    switch (err) {
        case CFL_OK:          return "OK";
        case CFL_ERR_NULL:    return "Null pointer";
        case CFL_ERR_SYNC:    return "Invalid sync word";
        case CFL_ERR_VERSION: return "Version mismatch";
        case CFL_ERR_CRC:     return "CRC mismatch";
        case CFL_ERR_LENGTH:  return "Invalid length";
        case CFL_ERR_BUFFER:  return "Buffer too small";
        case CFL_ERR_FLAGS:   return "Invalid flags";
        default:              return "Unknown error";
    }
}

void cfl_header_dump(const cfl_header_t *hdr)
{
    if (hdr == NULL) {
        printf("CFL Header: NULL\n");
        return;
    }

    printf("CFL Header:\n");
    printf("  sync:    0x%04X %s\n", hdr->sync, (hdr->sync == CFL_SYNC_WORD) ? "(OK)" : "(INVALID)");
    printf("  version: %u %s\n", hdr->version, (hdr->version == CFL_VERSION) ? "(OK)" : "(MISMATCH)");
    printf("  flags:   0x%02X [", hdr->flags);

    if (hdr->flags & CFL_F_RQST)       printf(" RQST");
    if (hdr->flags & CFL_F_RPLY)       printf(" RPLY");
    if (hdr->flags & CFL_F_PUSH)       printf(" PUSH");
    if (hdr->flags & CFL_F_ACK)        printf(" ACK");
    if (hdr->flags & CFL_F_NACK)       printf(" NACK");
    if (hdr->flags & CFL_F_ENCRYPTED)  printf(" ENC");
    if (hdr->flags & CFL_F_COMPRESSED) printf(" COMP");
    if (hdr->flags & CFL_F_RESERVED)   printf(" RSVD");

    printf(" ]\n");
    printf("  id:      0x%04X (%u)\n", hdr->id, hdr->id);
    printf("  seq:     0x%04X (%u)\n", hdr->seq, hdr->seq);
    printf("  length:  %u bytes\n", hdr->length);
    printf("  crc:     0x%04X\n", hdr->crc);
}
