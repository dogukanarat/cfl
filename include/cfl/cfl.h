/* cfl.h - one line definition */

/* All Rights Reserved */

#ifndef INC_CTRL_FREAK_LITE_H
#define INC_CTRL_FREAK_LITE_H

/* Includes */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
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

#define CFL_SYNC_WORD      (0x0A50)
#define CFL_VERSION        (1)
#define CFL_HEADER_SIZE    (sizeof(cfl_header_t))

#define CFL_F_RQST         (1 << 0)
#define CFL_F_RPLY         (1 << 1)
#define CFL_F_PUSH         (1 << 2)
#define CFL_F_ACK          (1 << 3)
#define CFL_F_NACK         (1 << 4)
#define CFL_F_ENCRYPTED    (1 << 5)
#define CFL_F_COMPRESSED   (1 << 6)
#define CFL_F_RESERVED     (1 << 7)

#define CFL_MAX_PAYLOAD_SIZE   (4096)
#define CFL_CRC_INIT           (0xFFFF)
#define CFL_CRC_POLY           (0x1021)  /* CRC-16-CCITT */

/* Types */

typedef enum cfl_error_e {
    CFL_OK              =  0,
    CFL_ERR_NULL        = -1,
    CFL_ERR_SYNC        = -2,
    CFL_ERR_VERSION     = -3,
    CFL_ERR_CRC         = -4,
    CFL_ERR_LENGTH      = -5,
    CFL_ERR_BUFFER      = -6,
    CFL_ERR_FLAGS       = -7,
} cfl_error_t;

typedef struct __packed cfl_header_s {
    uint16_t sync;
    uint8_t version;
    uint8_t flags;
    uint16_t id;
    uint16_t seq;
    uint16_t length;
    uint16_t crc;
} cfl_header_t;

typedef struct cfl_message_s {
    cfl_header_t header;
    uint8_t     *payload;
    uint16_t     payload_size;
} cfl_message_t;

/* External Declarations */

uint16_t cfl_crc16(const uint8_t *data, uint16_t len);
uint16_t cfl_crc16_continue(uint16_t crc, const uint8_t *data, uint16_t len);

/* Header Functions */

void cfl_header_init(cfl_header_t *hdr, uint16_t id, uint8_t flags);
void cfl_header_set_seq(cfl_header_t *hdr, uint16_t seq);
void cfl_header_set_length(cfl_header_t *hdr, uint16_t length);
void cfl_header_compute_crc(cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len);

/* Flag Functions */

void cfl_header_set_flags(cfl_header_t *hdr, uint8_t flags);
void cfl_header_clear_flags(cfl_header_t *hdr, uint8_t flags);
uint8_t cfl_header_has_flag(const cfl_header_t *hdr, uint8_t flag);

/* Validation Functions */

cfl_error_t cfl_header_validate(const cfl_header_t *hdr);
cfl_error_t cfl_message_validate(const cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len);

/* Serialization Functions */

uint16_t cfl_serialize(const cfl_header_t *hdr, const uint8_t *payload, uint16_t payload_len,
                       uint8_t *buf, uint16_t buf_size);
cfl_error_t cfl_deserialize(const uint8_t *buf, uint16_t buf_len,
                            cfl_header_t *hdr, const uint8_t **payload, uint16_t *payload_len);

/* Message Builder Functions */

cfl_error_t cfl_build_request(cfl_header_t *hdr, uint16_t id, uint16_t seq);
cfl_error_t cfl_build_reply(cfl_header_t *hdr, const cfl_header_t *request);
cfl_error_t cfl_build_ack(cfl_header_t *hdr, const cfl_header_t *request);
cfl_error_t cfl_build_nack(cfl_header_t *hdr, const cfl_header_t *request);
cfl_error_t cfl_build_push(cfl_header_t *hdr, uint16_t id, uint16_t seq);

/* Utility Functions */

const char *cfl_error_str(cfl_error_t err);
void cfl_header_dump(const cfl_header_t *hdr);

#ifdef __cplusplus
}
#endif

#endif /* INC_CTRL_FREAK_LITE_H */