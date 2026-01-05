/* cfl.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include "cfl/cfl.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Imports */


/* Definitions */

#define CFL_CRC_OFFSET (offsetof(cfl_message_t, crc))

/* Types */


/* Forward Declarations */


/* Variables */


/* Functions */


uint16_t cfl_crc16_continue(uint16_t crc, const uint8_t *data, uint16_t len)
{
    if (data == NULL)
    {
        return crc;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CFL_CRC_POLY;
            }
            else
            {
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

void cfl_message_init(cfl_message_t *msg, uint16_t id, uint8_t flags)
{
    if (msg == NULL)
    {
        return;
    }

    memset(msg, 0, sizeof(cfl_message_t));
    msg->sync = CFL_SYNC_WORD;
    msg->version = CFL_VERSION;
    msg->flags = flags;
    msg->id = id;
    msg->seq = 0;
    msg->length = 0;
    msg->crc = 0;
}

void cfl_message_set_seq(cfl_message_t *msg, uint16_t seq)
{
    if (msg == NULL)
    {
        return;
    }
    msg->seq = seq;
}

void cfl_message_set_length(cfl_message_t *msg, uint16_t length)
{
    if (msg == NULL)
    {
        return;
    }
    msg->length = length;
}

void cfl_message_compute_crc(cfl_message_t *msg)
{
    uint16_t crc;

    if (msg == NULL)
    {
        return;
    }

    /* CRC covers header (excluding crc field) + payload */
    crc = cfl_crc16((const uint8_t *)msg, CFL_CRC_OFFSET);

    if (msg->length > 0)
    {
        crc = cfl_crc16_continue(crc, msg->data, msg->length);
    }

    msg->crc = crc;
}

/* Flag Functions */

void cfl_message_set_flags(cfl_message_t *msg, uint8_t flags)
{
    if (msg == NULL)
    {
        return;
    }
    msg->flags |= flags;
}

void cfl_message_clear_flags(cfl_message_t *msg, uint8_t flags)
{
    if (msg == NULL)
    {
        return;
    }
    msg->flags &= ~flags;
}

uint8_t cfl_message_has_flag(const cfl_message_t *msg, uint8_t flag)
{
    if (msg == NULL)
    {
        return 0;
    }
    return (msg->flags & flag) != 0;
}

/* Validation Functions */

static cfl_status_t cfl_header_validate(const cfl_message_t *msg)
{
    if (msg == NULL)
    {
        return CFL_ERR_NULL;
    }

    if (msg->sync != CFL_SYNC_WORD)
    {
        return CFL_ERR_SYNC;
    }

    if (msg->version != CFL_VERSION)
    {
        return CFL_ERR_VERSION;
    }

    if (msg->length > CFL_MAX_PAYLOAD_SIZE)
    {
        return CFL_ERR_LENGTH;
    }

    return CFL_OK;
}

cfl_status_t cfl_message_validate(const cfl_message_t *msg, size_t len)
{
    cfl_status_t err;
    uint16_t computed_crc;

    err = cfl_header_validate(msg);
    if (err != CFL_OK)
    {
        return err;
    }

    if (len != CFL_HEADER_SIZE + msg->length)
    {
        return CFL_ERR_BUFFER;
    }

    /* Compute and verify CRC */
    computed_crc = cfl_crc16((const uint8_t *)msg, CFL_CRC_OFFSET);
    if (msg->length > 0)
    {
        computed_crc = cfl_crc16_continue(computed_crc, msg->data, msg->length);
    }

    if (computed_crc != msg->crc)
    {
        return CFL_ERR_CRC;
    }

    return CFL_OK;
}

/* Utility Functions */

const char *cfl_error_str(cfl_status_t err)
{
    switch (err)
    {
    case CFL_OK:
        return "OK";
    case CFL_ERR_NULL:
        return "Null pointer";
    case CFL_ERR_SYNC:
        return "Invalid sync word";
    case CFL_ERR_VERSION:
        return "Version mismatch";
    case CFL_ERR_CRC:
        return "CRC mismatch";
    case CFL_ERR_LENGTH:
        return "Invalid length";
    case CFL_ERR_BUFFER:
        return "Buffer too small";
    case CFL_ERR_FLAGS:
        return "Invalid flags";
    default:
        return "Unknown error";
    }
}

void cfl_header_dump(const cfl_message_t *msg)
{
    if (msg == NULL)
    {
        printf("CFL Header: NULL\n");
        return;
    }

    printf("CFL Header:\n");
    printf(
        "  sync:    0x%04X %s\n",
        msg->sync,
        (msg->sync == CFL_SYNC_WORD) ? "(OK)" : "(INVALID)");
    printf(
        "  version: %u %s\n",
        msg->version,
        (msg->version == CFL_VERSION) ? "(OK)" : "(MISMATCH)");
    printf("  flags:   0x%02X [", msg->flags);

    if (msg->flags & CFL_F_RQST)
        printf(" RQST");
    if (msg->flags & CFL_F_RPLY)
        printf(" RPLY");
    if (msg->flags & CFL_F_PUSH)
        printf(" PUSH");
    if (msg->flags & CFL_F_ACK)
        printf(" ACK");
    if (msg->flags & CFL_F_NACK)
        printf(" NACK");
    if (msg->flags & CFL_F_ENCRYPTED)
        printf(" ENC");
    if (msg->flags & CFL_F_COMPRESSED)
        printf(" COMP");
    if (msg->flags & CFL_F_RESERVED)
        printf(" RSVD");

    printf(" ]\n");
    printf("  id:      0x%04X (%u)\n", msg->id, msg->id);
    printf("  seq:     0x%04X (%u)\n", msg->seq, msg->seq);
    printf("  length:  %u bytes\n", msg->length);
    printf("  crc:     0x%04X\n", msg->crc);
}
