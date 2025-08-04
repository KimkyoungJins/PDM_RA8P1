/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_pdm_api.h"
#include "r_pdm.h"
FSP_HEADER
#define PDM2_CALCULATED_SINCRNG_VALUE (5)
#define PDM2_CALCULATED_SINCDEC_VALUE (124)
#define PDM2_FILTER_SETTLING_TIME_US  (1662)

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
