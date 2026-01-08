/* cfl.h - one line definition */

/* All Rights Reserved */

#ifndef INC_CTRL_FREAK_LITE_H
#define INC_CTRL_FREAK_LITE_H

/* Includes */

#include <stddef.h>
#include <stdint.h>

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

// clang-format off

#define CFL_SYNC_WORD           (0x0A50)
#define CFL_VERSION             (1)
#define CFL_HEADER_SIZE         (sizeof(cfl_message_t))

#define CFL_F_RQST              (1u << 0)
#define CFL_F_RPLY              (1u << 1)
#define CFL_F_PUSH              (1u << 2)
#define CFL_F_ACK               (1u << 3)
#define CFL_F_NACK              (1u << 4)
#define CFL_F_ENCRYPTED         (1u << 5)
#define CFL_F_COMPRESSED        (1u << 6)
#define CFL_F_RESERVED          (1u << 7)

// clang-format on

/* Types */

typedef struct cfl_message_s
{
    /* Header */
    uint16_t sync;   ///< Sync word
    uint8_t version; ///< Protocol version
    uint8_t flags;   ///< Message flags
    uint16_t id;     ///< Message ID
    uint16_t seq;    ///< Sequence number
    uint16_t length; ///< Length of payload in bytes
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


#ifdef __cplusplus
}
#endif

#endif /* INC_CTRL_FREAK_LITE_H */