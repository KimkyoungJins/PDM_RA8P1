/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (3)
#endif
/* ISR prototypes */
void pdm_sdet_isr(void);
void pdm_dat_isr(void);
void pdm_err_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_PDM_SDET ((IRQn_Type) 0) /* PDM SDET (Sound detection interrupt) */
#define PDM_SDET_IRQn          ((IRQn_Type) 0) /* PDM SDET (Sound detection interrupt) */
#define VECTOR_NUMBER_PDM_DAT2 ((IRQn_Type) 1) /* PDM DAT2 (Data reception interrupt channel 2) */
#define PDM_DAT2_IRQn          ((IRQn_Type) 1) /* PDM DAT2 (Data reception interrupt channel 2) */
#define VECTOR_NUMBER_PDM_ERR2 ((IRQn_Type) 2) /* PDM ERR2 (Error detection interrupt channel 2) */
#define PDM_ERR2_IRQn          ((IRQn_Type) 2) /* PDM ERR2 (Error detection interrupt channel 2) */
/* The number of entries required for the ICU vector table. */
#define BSP_ICU_VECTOR_NUM_ENTRIES (3)

#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
