/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_pdm_api.h"
#include "r_pdm.h"
FSP_HEADER
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;

#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
#define PDM2_CALCULATED_SINCRNG_VALUE (1)
#define PDM2_CALCULATED_SINCDEC_VALUE (249)
#define PDM2_FILTER_SETTLING_TIME_US  (3319)

/** PDM Instance. */
extern const pdm_instance_t g_pdm0;

/** Access the PDM instance using these structures when calling API functions directly (::p_api is not used). */
extern pdm_instance_ctrl_t g_pdm0_ctrl;
extern const pdm_cfg_t g_pdm0_cfg;

#ifndef pdm0_callback
void pdm0_callback(pdm_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
