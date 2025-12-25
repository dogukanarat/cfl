/* cfl_service_danp.h - CFL Service over DANP Transport */

/* All Rights Reserved */

#ifndef INC_CFL_SERVICE_DANP_H
#define INC_CFL_SERVICE_DANP_H

/* Includes */

#include <stdint.h>
#include "danp/danp.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Configurations */

#ifndef CFL_DANP_RX_TASK_STACK_SIZE
#define CFL_DANP_RX_TASK_STACK_SIZE    (2048)
#endif

#ifndef CFL_DANP_RX_TASK_PRIORITY
#define CFL_DANP_RX_TASK_PRIORITY      (osPriorityNormal)
#endif

#ifndef CFL_DANP_MAX_HANDLERS
#define CFL_DANP_MAX_HANDLERS          (32)
#endif

#ifndef CFL_DANP_RX_TIMEOUT_MS
#define CFL_DANP_RX_TIMEOUT_MS         (100)
#endif

/* Definitions */

#define CFL_ERR_ALREADY_INIT   (-10)
#define CFL_ERR_NOT_INIT       (-11)
#define CFL_ERR_TRANSPORT      (-12)
#define CFL_ERR_NO_RESOURCE    (-13)
#define CFL_ERR_EXISTS         (-14)
#define CFL_ERR_NOT_FOUND      (-15)

/* Types */

typedef struct cfl_service_danp_config_s {
    uint16_t port_id;
} cfl_service_danp_config_t;

typedef enum cfl_response_type_e {
    CFL_RESP_NONE  = 0,
    CFL_RESP_REPLY = 1,
    CFL_RESP_ACK   = 2,
    CFL_RESP_NACK  = 3,
} cfl_response_type_t;

typedef struct cfl_response_s {
    cfl_response_type_t type;
    uint8_t            *payload;
    uint16_t            payload_len;
    uint8_t             flags;
} cfl_response_t;

/* Forward declaration */
struct cfl_header_s;

/**
 * @brief Request handler callback type
 * 
 * @param src_node   Source node address
 * @param src_port   Source port
 * @param hdr        Pointer to received CFL header
 * @param payload    Pointer to payload data (NULL if no payload)
 * @param payload_len Length of payload in bytes
 * @param response   Pointer to response structure to fill (NULL for PUSH messages)
 * @param user_data  User context passed during handler registration
 */
typedef void (*cfl_request_handler_t)(uint16_t src_node,
                                       uint16_t src_port,
                                       const struct cfl_header_s *hdr,
                                       const uint8_t *payload,
                                       uint16_t payload_len,
                                       cfl_response_t *response,
                                       void *user_data);

/* External Declarations */

/**
 * @brief Initialize CFL service over DANP transport
 * @param config Service configuration
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_init(const cfl_service_danp_config_t *config);

/**
 * @brief Deinitialize CFL service
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_deinit(void);

/**
 * @brief Register a handler for a specific message ID
 * @param id        Message ID to handle
 * @param handler   Callback function
 * @param user_data User context passed to handler
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_register_handler(uint16_t id,
                                                  cfl_request_handler_t handler,
                                                  void *user_data);

/**
 * @brief Unregister a handler for a specific message ID
 * @param id Message ID to unregister
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_unregister_handler(uint16_t id);

/**
 * @brief Send a request message
 * @param dst_node    Destination node address
 * @param dst_port    Destination port
 * @param id          Message ID
 * @param payload     Payload data (can be NULL if payload_len is 0)
 * @param payload_len Payload length in bytes
 * @param seq_out     Optional output for sequence number used
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_send_request(uint16_t dst_node,
                                              uint16_t dst_port,
                                              uint16_t id,
                                              const uint8_t *payload,
                                              uint16_t payload_len,
                                              uint16_t *seq_out);

/**
 * @brief Send a push message (no response expected)
 * @param dst_node    Destination node address
 * @param dst_port    Destination port
 * @param id          Message ID
 * @param payload     Payload data (can be NULL if payload_len is 0)
 * @param payload_len Payload length in bytes
 * @return CFL_OK on success, negative error code on failure
 */
extern int32_t cfl_service_danp_send_push(uint16_t dst_node,
                                           uint16_t dst_port,
                                           uint16_t id,
                                           const uint8_t *payload,
                                           uint16_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /* INC_CFL_SERVICE_DANP_H */