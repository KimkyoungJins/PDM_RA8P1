#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 2048
#define PDM_CALLBACK_NUM_SAMPLES 512
#define PDM_MIC_STARTUP_TIME_US 35000 
#define PDM_SDE_UPPER_LIMIT (uint32_t)10000  
#define PDM_SDE_LOWER_LIMIT (uint32_t)-10000 
#define PDM0_FILTER_SETTLING_TIME_US (25000U)

// 텍스트 출력 설정
#define ENABLE_AUDIO_TEXT_OUTPUT 1      // 텍스트 출력 활성화
#define AUDIO_OUTPUT_INTERVAL 10        // 10번마다 출력 (성능 고려)
#define SAMPLES_PER_LINE 8              // 한 줄에 출력할 샘플 수
#define OUTPUT_FORMAT_HEX 1             // 1=16진수, 0=10진수

uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

// Statistics counters
static uint32_t g_sound_detection_count = 0;
static uint32_t g_data_callback_count = 0;
static uint32_t g_error_count = 0;
static uint32_t g_total_samples_saved = 0;  // 저장된 총 샘플 수

// Simple counter-based time measurement
static uint32_t g_callback_start_count = 0;
static uint32_t g_callback_end_count = 0;

// ✅ 함수 선언 (Function Prototypes) - 이 부분이 누락되었습니다!
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number);
void save_audio_data_as_csv(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number);

// 오디오 데이터를 텍스트로 출력하는 함수
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number)
{
    if (!ENABLE_AUDIO_TEXT_OUTPUT) return;
    
    // 출력 간격 확인
    if (callback_number % AUDIO_OUTPUT_INTERVAL != 0) return;
    
    SEGGER_RTT_printf(0, "\n=== AUDIO DATA CALLBACK #%lu ===\n", callback_number);
    SEGGER_RTT_printf(0, "Timestamp: %lu ms\n", callback_number * PDM_CALLBACK_NUM_SAMPLES * 1000 / 16000);
    SEGGER_RTT_printf(0, "Samples: %lu (Total: %lu)\n", sample_count, g_total_samples_saved);
    
    // 샘플 데이터 출력 (처음 64개만 출력하여 성능 최적화)
    uint32_t max_samples = (sample_count > 64) ? 64 : sample_count;
    
    for (uint32_t i = 0; i < max_samples; i++)
    {
        // 새 줄 시작
        if (i % SAMPLES_PER_LINE == 0)
        {
            SEGGER_RTT_printf(0, "\nS[%04lu]: ", i);
        }
        
        // 데이터 형식에 따른 출력
        if (OUTPUT_FORMAT_HEX)
        {
            // 16진수 출력 (PDM 원시 데이터)
            SEGGER_RTT_printf(0, "%08lX ", buffer[i]);
        }
        else
        {
            // 10진수 출력 (부호있는 정수)
            int32_t signed_sample = (int32_t)buffer[i];
            SEGGER_RTT_printf(0, "%8ld ", signed_sample);
        }
    }
    
    if (sample_count > 64)
    {
        SEGGER_RTT_printf(0, "\n... (showing first 64 of %lu samples)", sample_count);
    }
    
    SEGGER_RTT_printf(0, "\n=== END AUDIO DATA ===\n\n");
    
    g_total_samples_saved += sample_count;
}

// CSV 형식으로 저장하는 함수 (선택사항)
void save_audio_data_as_csv(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number)
{
    if (callback_number == 1)
    {
        // CSV 헤더 출력
        SEGGER_RTT_printf(0, "Callback,Sample_Index,Timestamp_ms,Raw_Value,Signed_Value\n");
    }
    
    // 성능을 위해 처음 16개 샘플만 CSV로 출력
    uint32_t max_samples = (sample_count > 16) ? 16 : sample_count;
    
    for (uint32_t i = 0; i < max_samples; i++)
    {
        uint32_t global_sample_index = (callback_number - 1) * PDM_CALLBACK_NUM_SAMPLES + i;
        uint32_t timestamp_ms = global_sample_index * 1000 / 16000; // 16kHz 기준
        int32_t signed_value = (int32_t)buffer[i];
        
        SEGGER_RTT_printf(0, "%lu,%lu,%lu,0x%08lX,%ld\n",
                         callback_number, global_sample_index, timestamp_ms, buffer[i], signed_value);
    }
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
                SEGGER_RTT_printf(0, "First callback started! Starting audio data logging...\n");
            }

            // ✅ 오디오 데이터를 텍스트로 저장
            save_audio_data_as_text(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES, g_data_callback_count);
            
            // 또는 CSV 형식으로 저장 (선택) - 주석 처리하여 성능 향상
            // save_audio_data_as_csv(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES, g_data_callback_count);

            // Periodic status output (every 50 times - 성능 최적화)
            if (g_data_callback_count % 50 == 0)
            {
                SEGGER_RTT_printf(0, "\n[STATUS] Callback #%lu - Progress Report\n", g_data_callback_count);
                SEGGER_RTT_printf(0, "   Total samples processed: %lu\n", g_total_samples_saved);
                SEGGER_RTT_printf(0, "   Estimated time: %lu.%03lu seconds\n", 
                                 g_total_samples_saved * 1000 / 16000 / 1000,
                                 (g_total_samples_saved * 1000 / 16000) % 1000);
                SEGGER_RTT_printf(0, "   Recent sample values: [0]=%08lx, [1]=%08lx, [2]=%08lx, [3]=%08lx\n",
                                 g_pdm0_buffer[0], g_pdm0_buffer[1], g_pdm0_buffer[2], g_pdm0_buffer[3]);
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
                
                // 오류 발생시에도 현재 버퍼 데이터 저장
                save_audio_data_as_text(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES, g_data_callback_count);
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
        SEGGER_RTT_printf(0, "Audio text logging: %s (Every %d callbacks)\n", 
                         ENABLE_AUDIO_TEXT_OUTPUT ? "ENABLED" : "DISABLED", AUDIO_OUTPUT_INTERVAL);
        SEGGER_RTT_printf(0, "Listening for audio...\n");
        SEGGER_RTT_printf(0, "================================================\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "PDM Start: FAILED (Error: 0x%X)\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "Recording for 100 seconds...\n");

    // Wait for 100 seconds
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_SECONDS); 

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

    // ✅ 개선된 최종 통계
    SEGGER_RTT_printf(0, "\n=== FINAL AUDIO DATA SUMMARY ===\n");
    SEGGER_RTT_printf(0, "Total callbacks processed: %lu\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Total samples saved: %lu\n", g_total_samples_saved);
    SEGGER_RTT_printf(0, "Total recording time: %lu.%03lu seconds\n", 
                     g_total_samples_saved * 1000 / 16000 / 1000,
                     (g_total_samples_saved * 1000 / 16000) % 1000);
    SEGGER_RTT_printf(0, "Average samples per callback: %lu\n", 
                     g_total_samples_saved / (g_data_callback_count > 0 ? g_data_callback_count : 1));
    SEGGER_RTT_printf(0, "Data integrity: %s\n", 
                     g_error_count == 0 ? "PERFECT" : "SOME ERRORS OCCURRED");
    SEGGER_RTT_printf(0, "Sound detections: %lu times\n", g_sound_detection_count);
    SEGGER_RTT_printf(0, "Errors: %lu times\n", g_error_count);
    
    // 데이터 파일 정보
    SEGGER_RTT_printf(0, "\n=== SAVED DATA INFO ===\n");
    SEGGER_RTT_printf(0, "Format: %s\n", OUTPUT_FORMAT_HEX ? "Hexadecimal" : "Decimal");
    SEGGER_RTT_printf(0, "Samples per line: %d\n", SAMPLES_PER_LINE);
    SEGGER_RTT_printf(0, "Output interval: Every %d callback(s)\n", AUDIO_OUTPUT_INTERVAL);
    SEGGER_RTT_printf(0, "Text logging: %s\n", ENABLE_AUDIO_TEXT_OUTPUT ? "ENABLED" : "DISABLED");
    SEGGER_RTT_printf(0, "=====================================\n");
}