#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 4096
#define PDM_CALLBACK_NUM_SAMPLES 1024
#define PDM_MIC_STARTUP_TIME_US 35000 
#define PDM_SDE_UPPER_LIMIT (uint32_t)10000  
#define PDM_SDE_LOWER_LIMIT (uint32_t)-10000 
#define PDM0_FILTER_SETTLING_TIME_US (25000U)

// Text output settings
#define ENABLE_AUDIO_TEXT_OUTPUT 1      
#define AUDIO_OUTPUT_INTERVAL 10        
#define SAMPLES_PER_LINE 8              
#define OUTPUT_FORMAT_HEX 1             

// Complete data storage buffer
#define MAX_TOTAL_SAMPLES 160000         // About 10 seconds
uint32_t g_all_audio_data[MAX_TOTAL_SAMPLES];
uint32_t g_total_collected_samples = 0;

uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

// Statistics counters
static uint32_t g_sound_detection_count = 0;
static uint32_t g_data_callback_count = 0; 
static uint32_t g_error_count = 0;
static uint32_t g_total_samples_saved = 0;

// Simple counter-based time measurement
static uint32_t g_callback_start_count = 0;
static uint32_t g_callback_end_count = 0;

// Function declarations
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number);
void collect_all_audio_data(uint32_t *buffer, uint32_t sample_count);
void dump_all_collected_data(void);
void analyze_audio_data(uint32_t *buffer, uint32_t sample_count);
void r_pdm_basic_messaging_core0_example(void);

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
    SEGGER_RTT_printf(0, "Recording duration: %lu.%03lu seconds\n", 
                     g_total_collected_samples * 1000 / 16000 / 1000,
                     (g_total_collected_samples * 1000 / 16000) % 1000);
    SEGGER_RTT_printf(0, "Data format: 32-bit hex (20-bit effective)\n");
    SEGGER_RTT_printf(0, "Sample rate: 16000 Hz\n");
    SEGGER_RTT_printf(0, "Bit depth: 20-bit PDM -> 16-bit PCM\n");
    
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
    
    SEGGER_RTT_printf(0, "\n*** PURE DATA OUTPUT START ***\n");

    // Pure data output (no addresses or prefixes) - Python-friendly format
    for (uint32_t i = 0; i < g_total_collected_samples; i++)
    {
        // New line every 16 samples
        if (i % 16 == 0 && i > 0)
        {
            SEGGER_RTT_printf(0, "\n");
        }
        // Space every 8 samples for readability
        else if (i % 8 == 0 && i > 0)
        {
            SEGGER_RTT_printf(0, "  ");
        }
        
        // Add space before data (except first)
        if (i > 0) {
            SEGGER_RTT_printf(0, " ");
        }

        SEGGER_RTT_printf(0, "%08lX", g_all_audio_data[i]);

        // Progress indicator for large data
        if ((i + 1) % 16000 == 0)
        {
            SEGGER_RTT_printf(0, "\n... Progress: %lu / %lu samples (%d%%) ...\n",
                             i + 1, g_total_collected_samples, 
                             (int)((i + 1) * 100 / g_total_collected_samples));
        }
    }

    SEGGER_RTT_printf(0, "\n*** PURE DATA OUTPUT END ***\n");
    
    SEGGER_RTT_printf(0, "\n=== END COMPLETE DATA DUMP ===\n");
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
}

// Audio data analysis (statistics)
void analyze_audio_data(uint32_t *buffer, uint32_t sample_count)
{
    uint32_t min_val = 0xFFFFFFFF;
    uint32_t max_val = 0;
    uint64_t sum = 0;  // 64-bit for large number handling
    uint32_t zero_count = 0;
    uint32_t negative_count = 0;

    for (uint32_t i = 0; i < sample_count; i++)
    {
        uint32_t sample = buffer[i];
        
        if (sample < min_val) min_val = sample;
        if (sample > max_val) max_val = sample;
        sum += sample;
        
        if (sample == 0) zero_count++;
        if (sample & 0x80000) negative_count++;
    }

    // Safe average calculation
    double average = (sample_count > 0) ? ((double)sum / (double)sample_count) : 0.0;
    
    // Safe percentage calculation
    double zero_pct = (sample_count > 0) ? ((double)zero_count * 100.0 / (double)sample_count) : 0.0;
    double neg_pct = (sample_count > 0) ? ((double)negative_count * 100.0 / (double)sample_count) : 0.0;
    
    SEGGER_RTT_printf(0, "\n=== AUDIO DATA ANALYSIS ===\n");
    SEGGER_RTT_printf(0, "Sample count: %lu\n", sample_count);
    SEGGER_RTT_printf(0, "Min value: 0x%08lX (%ld)\n", min_val, (int32_t)min_val);
    SEGGER_RTT_printf(0, "Max value: 0x%08lX (%ld)\n", max_val, (int32_t)max_val);
    SEGGER_RTT_printf(0, "Average: %.1f\n", average);
    SEGGER_RTT_printf(0, "Zero samples: %lu (%.1f%%)\n", zero_count, zero_pct);
    SEGGER_RTT_printf(0, "Negative samples: %lu (%.1f%%)\n", negative_count, neg_pct);
    SEGGER_RTT_printf(0, "Dynamic range: %lu\n", max_val - min_val);
    SEGGER_RTT_printf(0, "Signal quality: %s\n",
                     (max_val - min_val > 1000) ? "Good" : "Low");
}

// Legacy function (simplified)
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number)
{
    if (!ENABLE_AUDIO_TEXT_OUTPUT) return;
    if (callback_number % AUDIO_OUTPUT_INTERVAL != 0) return;
    
    SEGGER_RTT_printf(0, "\n=== AUDIO DATA CALLBACK #%lu ===\n", callback_number);
    
    // Show only first 32 samples for real-time output (performance optimization)
    uint32_t display_count = (sample_count > 32) ? 32 : sample_count;
    
    for (uint32_t i = 0; i < display_count; i++)
    {
        if (i % SAMPLES_PER_LINE == 0)
        {
            SEGGER_RTT_printf(0, "\nS[%04lu]: ", i);
        }
        SEGGER_RTT_printf(0, "%08lX ", buffer[i]);
    }
    
    if (sample_count > 32)
    {
        SEGGER_RTT_printf(0, "\n... (showing first 32 of %lu samples)", sample_count);
    }
    
    SEGGER_RTT_printf(0, "\n=== END CALLBACK DATA ===\n\n");
    g_total_samples_saved += sample_count;
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
            
            // REMOVED: No real-time text output (causes callback delay)
            // REMOVED: No complex analysis in callback (causes timing issues)

            // Minimal progress indication - ONLY every 100 callbacks (6.4 seconds)
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

    /* Sound detection enable */
    pdm_sound_detection_setting_t sound_detection_setting = {
        .sound_detection_lower_limit = PDM_SDE_LOWER_LIMIT,
        .sound_detection_upper_limit = PDM_SDE_UPPER_LIMIT
    };
    R_PDM_SoundDetectionEnable(&g_pdm0_ctrl, sound_detection_setting);

    /* PDM start */
    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm0_buffer, sizeof(g_pdm0_buffer), PDM_CALLBACK_NUM_SAMPLES);
    if (FSP_SUCCESS != err) {
        SEGGER_RTT_printf(0, "PDM Start FAILED: 0x%X\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "Recording started! (10 seconds)\n");
    SEGGER_RTT_printf(0, "Progress: ");

    // EXACTLY 10 seconds wait
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS);

    SEGGER_RTT_printf(0, "\nRecording completed!\n");

    /* PDM stop */
    R_PDM_Stop(&g_pdm0_ctrl);
    R_PDM_Close(&g_pdm0_ctrl);

    // Post-recording analysis
    SEGGER_RTT_printf(0, "\n=== POST-RECORDING ANALYSIS ===\n");
    SEGGER_RTT_printf(0, "Total callbacks: %lu\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Expected callbacks: %.1f\n", 10000.0f / 64.0f);
    SEGGER_RTT_printf(0, "Success rate: %.1f%%\n",
                     (float)g_data_callback_count * 100.0f / (10000.0f / 64.0f));
    SEGGER_RTT_printf(0, "Errors occurred: %lu\n", g_error_count);
    SEGGER_RTT_printf(0, "Actual recording time: %.2f seconds\n",
                     (float)g_total_collected_samples / 16000.0f);

    if (g_total_collected_samples > 0) {
        analyze_audio_data(g_all_audio_data, g_total_collected_samples);
    }

    // Final data output for Python processing
    SEGGER_RTT_printf(0, "\nStarting data output for Python processing...\n");
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    dump_all_collected_data();
    
    SEGGER_RTT_printf(0, "\n=== ALL TASKS COMPLETED ===\n");
}
