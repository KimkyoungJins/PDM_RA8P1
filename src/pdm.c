#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 4096
#define PDM_CALLBACK_NUM_SAMPLES 1024
#define PDM_MIC_STARTUP_TIME_US 35000 
#define PDM_SDE_UPPER_LIMIT 5000
#define PDM_SDE_LOWER_LIMIT 0xFFF80000
#define PDM0_FILTER_SETTLING_TIME_US (25000U)

// Text output setting
#define ENABLE_AUDIO_TEXT_OUTPUT 1      
#define AUDIO_OUTPUT_INTERVAL 10        
#define SAMPLES_PER_LINE 8              
#define OUTPUT_FORMAT_HEX 1             

// Complete data storage buffer
#define MAX_TOTAL_SAMPLES 160000         // About 10 seconds

// 저장용 버퍼
uint32_t g_all_audio_data[MAX_TOTAL_SAMPLES];
uint32_t g_total_collected_samples = 0;

uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

// Statistics counters
static uint32_t g_sound_detection_count = 0;
static uint32_t g_data_callback_count = 0; 
static uint32_t g_error_count = 0;

// Function declarations
void collect_all_audio_data(uint32_t *buffer, uint32_t sample_count);
void dump_all_collected_data(void);
void analyze_audio_data(uint32_t *buffer, uint32_t sample_count);
void r_pdm_basic_messaging_core0_example(void);


// Main function
void r_pdm_basic_messaging_core0_example(void)
{
    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "\n=== PDM OPTIMIZED RECORDING START ===\n");

    /* PDM initialization */
    fsp_err_t err = R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);
    if (FSP_SUCCESS != err) {
        SEGGER_RTT_printf(0, "PDM Open FAILED: 0x%X\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "PDM Open: SUCCESS\n");

    /* Filter stabilization wait */
    SEGGER_RTT_printf(0, "Filter stabilizing...\n");
    R_BSP_SoftwareDelay(PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US,
                        BSP_DELAY_UNITS_MICROSECONDS);


    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);


    /* PDM start */
    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm0_buffer, sizeof(g_pdm0_buffer), PDM_CALLBACK_NUM_SAMPLES);

    if (FSP_SUCCESS != err) {
        SEGGER_RTT_printf(0, "PDM Start FAILED: 0x%X\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "Recording started! (10 seconds)\n");
    SEGGER_RTT_printf(0, "Progress.... ");

    // EXACTLY 10 seconds wait
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS);

    SEGGER_RTT_printf(0, "\nRecording completed!\n");

    /* PDM stop */
    R_PDM_Stop(&g_pdm0_ctrl);
    R_PDM_Close(&g_pdm0_ctrl);

    // Post-recording analysis
    SEGGER_RTT_printf(0, "\n=== POST-RECORDING ANALYSIS ===\n");
    SEGGER_RTT_printf(0, "Total callbacks: %lu\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Errors occurred: %lu\n", g_error_count);


    // Final data output for Python processing
    SEGGER_RTT_printf(0, "\nStarting data output for Python processing...\n");
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    dump_all_collected_data();
    
    SEGGER_RTT_printf(0, "\n=== ALL TASKS COMPLETED ===\n");
}


// ULTRA-OPTIMIZED callback function for maximum performance
void pdm0_callback(pdm_callback_args_t * p_args)
{
    switch(p_args->event)
    {
        case PDM_EVENT_SOUND_DETECTION:
        {
            g_sound_detection_count++;
            break;
        }

        case PDM_EVENT_DATA:
        {
            g_data_callback_count++;

            // CRITICAL: Only fast data collection!
            collect_all_audio_data(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES);
            
            if (g_data_callback_count % 100 == 0)
            {
                SEGGER_RTT_printf(0, ".");
            }

            break;
        }

        case PDM_EVENT_ERROR:
        {
            g_error_count++;
            // Continue data collection even on error
            collect_all_audio_data(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES);
            break;
        }

        default:
            break;
    }
}

// Collect all audio data into large buffer
void collect_all_audio_data(uint32_t *buffer, uint32_t sample_count)
{
    for (uint32_t i = 0; i < sample_count && g_total_collected_samples < MAX_TOTAL_SAMPLES; i++)
    {
        g_all_audio_data[g_total_collected_samples] = buffer[i];
        g_total_collected_samples++;
    }
}

// Output all collected data in pure format for Python processing
void dump_all_collected_data(void)
{
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }

    SEGGER_RTT_printf(0, "\n");

    SEGGER_RTT_printf(0, "=== COMPLETE AUDIO DATA DUMP ===\n");
    SEGGER_RTT_printf(0, "Total collected samples: %lu\n", g_total_collected_samples);
    SEGGER_RTT_printf(0, "Data format: 32-bit hex (20-bit effective)\n");
    SEGGER_RTT_printf(0, "Sample rate: 16000 Hz\n");
    SEGGER_RTT_printf(0, "Bit depth: 20-bit PDM -> 16-bit PCM\n");
    
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
    
    SEGGER_RTT_printf(0, "\n*** PURE DATA OUTPUT START ***\n");
    SEGGER_RTT_printf(0, " ");


    for (uint32_t i = 0; i < g_total_collected_samples; i++)
    {
        // New line every 16 samples
        if (i % 16 == 0 && i > 0)
        {
            SEGGER_RTT_printf(0, "\n");
        }

        if (i > 0) {
            SEGGER_RTT_printf(0, " ");
        }

        SEGGER_RTT_printf(0, "%08lX", g_all_audio_data[i]);


        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    }

    SEGGER_RTT_printf(0, "\n*** PURE DATA OUTPUT END ***\n");
    
    SEGGER_RTT_printf(0, "\n=== END COMPLETE DATA DUMP ===\n");
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
}

