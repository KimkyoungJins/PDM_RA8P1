/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = pdm_sdet_isr, /* PDM SDET (Sound detection interrupt) */
            [1] = pdm_dat_isr, /* PDM DAT2 (Data reception interrupt channel 2) */
            [2] = pdm_err_isr, /* PDM ERR2 (Error detection interrupt channel 2) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_PDM_SDET,GROUP0), /* PDM SDET (Sound detection interrupt) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_PDM_DAT2,GROUP1), /* PDM DAT2 (Data reception interrupt channel 2) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_PDM_ERR2,GROUP2), /* PDM ERR2 (Error detection interrupt channel 2) */
        };
        #endif
        #endif
