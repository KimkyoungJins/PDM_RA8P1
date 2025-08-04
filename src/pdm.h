/**
 * @file pdm.h
 * @brief Simple PDM Interface Header File
 * @details Header file for PDM microphone functions
 */

#ifndef PDM_H
#define PDM_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "hal_data.h"
#include "bsp_api.h"
#include "r_pdm.h"
#include <assert.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** PDM Buffer Configuration */
#define PDM_BUFFER_NUM_SAMPLES          (1024U)    /**< Number of samples in buffer */
#define PDM_CALLBACK_NUM_SAMPLES        (PDM_BUFFER_NUM_SAMPLES / 2)  /**< Callback trigger point */

/** PDM Timing Configuration */
#define PDM_MIC_STARTUP_TIME_US         (35000U)   /**< Microphone startup time (35ms) */

/** PDM Filter Settling Time */
#ifdef G_PDM0_FILTER_SETTLING_TIME_US
    #define PDM0_FILTER_SETTLING_TIME_US    G_PDM0_FILTER_SETTLING_TIME_US
#else
    #define PDM0_FILTER_SETTLING_TIME_US    (20000U)  /**< Default 20ms settling time */
#endif

/** Sound Detection Thresholds */
#define PDM_SDE_UPPER_LIMIT             (5000U)            /**< Sound detection upper limit */
#define PDM_SDE_LOWER_LIMIT             (0xFFF80000U)      /**< Sound detection lower limit */

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** PDM data buffer */
extern uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

/** PDM control and configuration (defined in hal_data.c) */
extern pdm_instance_ctrl_t g_pdm0_ctrl;
extern const pdm_cfg_t g_pdm0_cfg;

/***********************************************************************************************************************
 * Function Declarations
 **********************************************************************************************************************/

/**
 * @brief Basic PDM example function
 * @details Demonstrates PDM usage with sound detection
 */
void r_pdm_basic_messaging_core0_example(void);

/**
 * @brief PDM callback function
 * @param[in] p_args    Callback arguments
 */
void pdm0_callback(pdm_callback_args_t * p_args);

/**
 * @brief Initialize PDM interface
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_Open
 */
fsp_err_t pdm_init(void);

/**
 * @brief Start PDM data acquisition
 * @param[in] p_buffer          Data buffer pointer
 * @param[in] buffer_size       Buffer size in bytes
 * @param[in] callback_samples  Samples before callback
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_Start
 */
fsp_err_t pdm_start(void * const p_buffer, size_t const buffer_size, uint32_t const callback_samples);

/**
 * @brief Stop PDM data acquisition
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_Stop
 */
fsp_err_t pdm_stop(void);

/**
 * @brief Enable sound detection
 * @param[in] upper_limit   Upper threshold
 * @param[in] lower_limit   Lower threshold
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_SoundDetectionEnable
 */
fsp_err_t pdm_sound_detection_enable(uint32_t upper_limit, uint32_t lower_limit);

/**
 * @brief Disable sound detection
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_SoundDetectionDisable
 */
fsp_err_t pdm_sound_detection_disable(void);

/**
 * @brief Get PDM status
 * @param[out] p_status Status structure pointer
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_StatusGet
 */
fsp_err_t pdm_status_get(pdm_status_t * const p_status);

/**
 * @brief Close PDM interface
 * @retval FSP_SUCCESS  Success
 * @retval Other        Error codes from R_PDM_Close
 */
fsp_err_t pdm_close(void);

/**
 * @brief Calculate RMS value of PDM data
 * @param[in] p_data        Data buffer pointer
 * @param[in] num_samples   Number of samples
 * @return RMS value
 */
uint32_t pdm_calculate_rms(uint32_t * p_data, uint32_t num_samples);

/**
 * @brief Apply high-pass filter to remove DC bias
 * @param[in,out] p_data    Data buffer pointer
 * @param[in] num_samples   Number of samples
 */
void pdm_apply_highpass_filter(int32_t * p_data, uint32_t num_samples);

/***********************************************************************************************************************
 * Inline Utility Functions
 **********************************************************************************************************************/

/**
 * @brief Convert 20-bit PDM data to signed integer
 * @param[in] raw_data  Raw PDM data
 * @return Signed integer value
 */
static inline int32_t pdm_convert_20bit_to_signed(uint32_t raw_data)
{
    int32_t result = (int32_t)(raw_data & 0x000FFFFF);  /* 20-bit mask */

    /* Sign extend if negative (bit 19 is set) */
    if (result & 0x80000)
    {
        result |= 0xFFF00000;  /* Sign extension */
    }

    return result;
}

/**
 * @brief Convert 16-bit PDM data to signed integer
 * @param[in] raw_data  Raw PDM data
 * @return Signed integer value
 */
static inline int32_t pdm_convert_16bit_to_signed(uint32_t raw_data)
{
    int32_t result = (int32_t)(raw_data & 0x0000FFFF);  /* 16-bit mask */

    /* Sign extend if negative (bit 15 is set) */
    if (result & 0x8000)
    {
        result |= 0xFFFF0000;  /* Sign extension */
    }

    return result;
}

#endif /* PDM_H */
