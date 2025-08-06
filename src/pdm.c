#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 2048 
#define PDM_CALLBACK_NUM_SAMPLES PDM_BUFFER_NUM_SAMPLES / 2 
#define PDM_MIC_STARTUP_TIME_US 35000 
#define PDM_SDE_UPPER_LIMIT (uint32_t)10000  
#define PDM_SDE_LOWER_LIMIT (uint32_t)-10000 
#define PDM0_FILTER_SETTLING_TIME_US (25000U) 

uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

// Statistics counters
static uint32_t g_sound_detection_count = 0;
static uint32_t g_data_callback_count = 0;
static uint32_t g_error_count = 0;

// Simple counter-based time measurement
static uint32_t g_callback_start_count = 0;
static uint32_t g_callback_end_count = 0;

void r_pdm_basic_messaging_core0_example(void)
{
    // RTT initialization
    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "\n=== PDM Microphone Test Start ===\n");

    /* Open PDM instance. */
    fsp_err_t err = R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "PDM Open: SUCCESS\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "PDM Open: FAILED (Error: 0x%X)\n", err);
        return;
    }

    /* Wait for PDM filters to settle. */
    SEGGER_RTT_printf(0, "Waiting for filter settling and mic startup (%d ms)...\n",
                     (PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US) / 1000);

    R_BSP_SoftwareDelay(PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US, BSP_DELAY_UNITS_MICROSECONDS);

    SEGGER_RTT_printf(0, "Filter and Mic ready!\n\n");

    /* Enable sound detection */
    pdm_sound_detection_setting_t sound_detection_setting =
    {
        .sound_detection_lower_limit = PDM_SDE_LOWER_LIMIT,
        .sound_detection_upper_limit = PDM_SDE_UPPER_LIMIT
    };

    err = R_PDM_SoundDetectionEnable(&g_pdm0_ctrl, sound_detection_setting);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "Sound Detection: ENABLED (Lower: 0x%X, Upper: %d)\n",
                         PDM_SDE_LOWER_LIMIT, PDM_SDE_UPPER_LIMIT);
    }
    else
    {
        SEGGER_RTT_printf(0, "Sound Detection Enable: FAILED (Error: 0x%X)\n", err);
    }

    /* Start receiving PDM data. */
    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm0_buffer, sizeof(g_pdm0_buffer), PDM_CALLBACK_NUM_SAMPLES);
    
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "PDM Recording: STARTED (Buffer: %d samples, Callback every: %d samples)\n",
                         PDM_BUFFER_NUM_SAMPLES, PDM_CALLBACK_NUM_SAMPLES);
        SEGGER_RTT_printf(0, "Listening for audio...\n");
        SEGGER_RTT_printf(0, "================================================\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "PDM Start: FAILED (Error: 0x%X)\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "Recording for 10 seconds...\n");

    // Wait for 10 seconds
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS); 

    SEGGER_RTT_printf(0, "\n================================================\n");

    /* Stop receiving PDM data. */
    err = R_PDM_Stop(&g_pdm0_ctrl);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "PDM Stop: SUCCESS\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "PDM Stop: FAILED (Error: 0x%X)\n", err);
    }

    /* Close PDM instance. */
    err = R_PDM_Close(&g_pdm0_ctrl);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "PDM Close: SUCCESS\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "PDM Close: FAILED (Error: 0x%X)\n", err);
    }

    // Simple time calculation using integer math only
    if (g_callback_end_count > g_callback_start_count)
    {
        uint32_t total_samples = g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES;
        
        SEGGER_RTT_printf(0, "\n=== Time Analysis Results ===\n");
        SEGGER_RTT_printf(0, "Total callbacks: %lu\n", g_data_callback_count);
        SEGGER_RTT_printf(0, "Total samples: %lu\n", total_samples);
        
        // Calculate time using integer math (avoiding float)
        // For 16kHz: time_ms = samples * 1000 / 16000 = samples / 16
        uint32_t time_16k_ms = total_samples * 1000 / 16000;  // milliseconds for 16kHz
        uint32_t time_48k_ms = total_samples * 1000 / 48000;  // milliseconds for 48kHz
        
        SEGGER_RTT_printf(0, "Estimated time (16kHz): %lu.%03lu seconds\n", 
                         time_16k_ms / 1000, time_16k_ms % 1000);
        SEGGER_RTT_printf(0, "Estimated time (48kHz): %lu.%03lu seconds\n", 
                         time_48k_ms / 1000, time_48k_ms % 1000);
        
        // Most likely sampling rate calculation
        SEGGER_RTT_printf(0, "Most likely actual runtime: %lu seconds\n", time_16k_ms / 1000);
        
        // Calculate actual sampling frequency (assuming 4.5 seconds actual runtime)
        uint32_t estimated_freq = total_samples / 4; // Assuming 4 seconds
        SEGGER_RTT_printf(0, "Estimated sampling frequency: ~%lu Hz\n", estimated_freq);
    }

    // Final statistics output
    SEGGER_RTT_printf(0, "\n=== Final Statistics ===\n");
    SEGGER_RTT_printf(0, "Sound Detections: %lu times\n", g_sound_detection_count);
    SEGGER_RTT_printf(0, "Data Callbacks: %lu times\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Errors: %lu times\n", g_error_count);
    SEGGER_RTT_printf(0, "Total Audio Samples Processed: %lu\n", g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES);
    SEGGER_RTT_printf(0, "========================\n");
}

void pdm0_callback(pdm_callback_args_t * p_args)
{
    switch(p_args->event)
    {
        case PDM_EVENT_SOUND_DETECTION:
        {
            g_sound_detection_count++;
            SEGGER_RTT_printf(0, "SOUND DETECTED! (#%lu) - Loud sound threshold exceeded\n",
                             g_sound_detection_count);
            break;
        }

        case PDM_EVENT_DATA:
        {
            g_data_callback_count++;

            // Record first callback
            if (g_data_callback_count == 1)
            {
                g_callback_start_count = g_data_callback_count;
                SEGGER_RTT_printf(0, "First callback started!\n");
            }

            // Periodic detailed output (every 10 times)
            if (g_data_callback_count % 10 == 0)
            {
                SEGGER_RTT_printf(0, "[DATA] Callback #%lu - %d samples ready (Total: %lu samples)\n",
                                 g_data_callback_count,
                                 PDM_CALLBACK_NUM_SAMPLES,
                                 g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES);

                // Sample values output
                SEGGER_RTT_printf(0, "   Sample values: [0]=%08lx, [1]=%08lx, [2]=%08lx, [3]=%08lx\n",
                                 g_pdm0_buffer[0], g_pdm0_buffer[1], g_pdm0_buffer[2], g_pdm0_buffer[3]);
                
                // Estimated elapsed time using integer math
                uint32_t total_samples = g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES;
                uint32_t estimated_time_ms = total_samples * 1000 / 16000; // Assume 16kHz
                
                SEGGER_RTT_printf(0, "   Estimated elapsed time: %lu.%03lu seconds\n", 
                                 estimated_time_ms / 1000, estimated_time_ms % 1000);
            }

            // Update last callback number every time callback ends
            g_callback_end_count = g_data_callback_count;

            break;
        }

        case PDM_EVENT_ERROR:
        {
            g_error_count++;

            SEGGER_RTT_printf(0, "\n[ERROR #%lu] PDM Error Detected! (Flags: 0x%lX)\n",
                             g_error_count, (uint32_t)p_args->error);

            if (p_args->error & PDM_ERROR_SHORT_CIRCUIT)
            {
                SEGGER_RTT_printf(0, "   SHORT CIRCUIT detected\n");
            }

            if (p_args->error & PDM_ERROR_OVERVOLTAGE_LOWER)
            {
                SEGGER_RTT_printf(0, "   UNDER-VOLTAGE detected\n");
            }

            if (p_args->error & PDM_ERROR_OVERVOLTAGE_UPPER)
            {
                SEGGER_RTT_printf(0, "   OVER-VOLTAGE detected\n");
            }

            if (p_args->error & PDM_ERROR_BUFFER_OVERWRITE)
            {
                SEGGER_RTT_printf(0, "   BUFFER OVERWRITE occurred - Data loss possible!\n");
                SEGGER_RTT_printf(0, "   Suggestion: Process data faster or increase buffer size\n");
            }

            break;
        }

        default:
        {
            SEGGER_RTT_printf(0, "[UNKNOWN] Unexpected PDM event: %d\n", p_args->event);
            break;
        }
    }
}