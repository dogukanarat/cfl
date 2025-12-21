/* cfl_service.h - one line definition */

/* All Rights Reserved */

#ifndef INC_CTRL_FREAK_LITE_SERVICE_H
#define INC_CTRL_FREAK_LITE_SERVICE_H

/* Includes */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Configurations */


/* Definitions */


/* Types */

typedef struct cfl_service_config_s {
    uint8_t dummy;
} cfl_service_config_t;

/* External Declarations */

extern int32_t cfl_service_init(const cfl_service_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* INC_CTRL_FREAK_LITE_SERVICE_H */
