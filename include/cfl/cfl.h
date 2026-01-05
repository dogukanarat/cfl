/* cfl.h - one line definition */

/* All Rights Reserved */

#ifndef INC_CTRL_FREAK_LITE_H
#define INC_CTRL_FREAK_LITE_H

/* Includes */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif


/* Configurations */


/* Definitions */

#ifndef __packed
#if defined(__GNUC__)
#define __packed __attribute__((__packed__))
#else
#define __packed
#endif
#endif

#define CFL_SYNC_WORD (0x0A50)
#define CFL_VERSION (1)
#define CFL_HEADER_SIZE (sizeof(cfl_message_t))

#define CFL_F_RQST (1 << 0)
#define CFL_F_RPLY (1 << 1)
#define CFL_F_PUSH (1 << 2)
#define CFL_F_ACK (1 << 3)
#define CFL_F_NACK (1 << 4)
#define CFL_F_ENCRYPTED (1 << 5)
#define CFL_F_COMPRESSED (1 << 6)
#define CFL_F_RESERVED (1 << 7)

#define CFL_MAX_PAYLOAD_SIZE (4096)
#define CFL_CRC_INIT (0xFFFF)
#define CFL_CRC_POLY (0x1021) /* CRC-16-CCITT */

/* Types */

typedef enum cfl_status_e
{
    CFL_OK = 0,
    CFL_ERR_NULL = -1,
    CFL_ERR_SYNC = -2,
    CFL_ERR_VERSION = -3,
    CFL_ERR_CRC = -4,
    CFL_ERR_LENGTH = -5,
    CFL_ERR_BUFFER = -6,
    CFL_ERR_FLAGS = -7,
} cfl_status_t;

typedef struct cfl_message_s
{
    /* Header */
    uint16_t sync;    ///< Sync word
    uint8_t version;  ///< Protocol version
    uint8_t flags;    ///< Message flags
    uint16_t id;      ///< Message ID
    uint16_t seq;     ///< Sequence number
    uint16_t length;  ///< Length of payload in bytes
    uint16_t crc;     ///< CRC16 of header and payload except crc field
    /* Payload */
    union
    {
        uint8_t data[0];
        uint16_t data16[0];
        uint32_t data32[0];
        uint64_t data64[0];
    };
} cfl_message_t;

/* External Declarations */

uint16_t cfl_crc16(const uint8_t *data, uint16_t len);
uint16_t cfl_crc16_continue(uint16_t crc, const uint8_t *data, uint16_t len);

/* Header Functions */

void cfl_message_init(cfl_message_t *msg, uint16_t id, uint8_t flags);
void cfl_message_set_seq(cfl_message_t *msg, uint16_t seq);
void cfl_message_set_length(cfl_message_t *msg, uint16_t length);
void cfl_message_compute_crc(cfl_message_t *msg);

/* Flag Functions */

void cfl_message_set_flags(cfl_message_t *msg, uint8_t flags);
void cfl_message_clear_flags(cfl_message_t *msg, uint8_t flags);
uint8_t cfl_message_has_flag(const cfl_message_t *msg, uint8_t flag);

/* Validation Functions */

cfl_status_t cfl_message_validate(const cfl_message_t *msg, size_t len);

/* Utility Functions */

const char *cfl_error_str(cfl_status_t err);
void cfl_header_dump(const cfl_message_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* INC_CTRL_FREAK_LITE_H */