#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 2048
#define PDM_CALLBACK_NUM_SAMPLES 512
#define PDM_MIC_STARTUP_TIME_US 35000 
#define PDM_SDE_UPPER_LIMIT (uint32_t)10000  
#define PDM_SDE_LOWER_LIMIT (uint32_t)-10000 
#define PDM0_FILTER_SETTLING_TIME_US (25000U)

// 텍스트 출력 설정
#define ENABLE_AUDIO_TEXT_OUTPUT 1      
#define AUDIO_OUTPUT_INTERVAL 10        
#define SAMPLES_PER_LINE 8              
#define OUTPUT_FORMAT_HEX 1             

// ✅ 전체 데이터 저장을 위한 큰 버퍼
#define MAX_TOTAL_SAMPLES 65536         // 64K 샘플 저장 (약 4초분)
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

// 함수 선언
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number);
void collect_all_audio_data(uint32_t *buffer, uint32_t sample_count);
void dump_all_collected_data(void);
void analyze_audio_data(uint32_t *buffer, uint32_t sample_count);

// ✅ 모든 오디오 데이터를 큰 버퍼에 수집
void collect_all_audio_data(uint32_t *buffer, uint32_t sample_count)
{
    for (uint32_t i = 0; i < sample_count && g_total_collected_samples < MAX_TOTAL_SAMPLES; i++)
    {
        g_all_audio_data[g_total_collected_samples] = buffer[i];
        g_total_collected_samples++;
    }
}

// ✅ 수집된 모든 데이터를 보기 좋게 출력
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
    
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
    

    // 전체 데이터 출력
    for (uint32_t i = 0; i < g_total_collected_samples; i++)
    {
        // 새로운 줄 시작 (16개씩)
        if (i % 16 == 0)
        {
            SEGGER_RTT_printf(0, "\n[%06lu]: ", i);
        }
        // 8개마다 공백 추가 (가독성)
        else if (i % 8 == 0)
        {
            SEGGER_RTT_printf(0, "  ");
        }

        SEGGER_RTT_printf(0, "%08lX ", g_all_audio_data[i]);

        // 1000줄마다 진행 상황 표시 (대용량 데이터 처리)
        if ((i + 1) % 16000 == 0)
        {
            SEGGER_RTT_printf(0, "\n... Progress: %lu / %lu samples (%d%%) ...\n", 
                             i + 1, g_total_collected_samples, 
                             (int)((i + 1) * 100 / g_total_collected_samples));
        }
    }

    SEGGER_RTT_printf(0, "\n\n=== END COMPLETE DATA DUMP ===\n");
    SEGGER_RTT_printf(0, "\n");
    for(int i = 0; i < 60; i++){
        SEGGER_RTT_printf(0, "=");
    }
    SEGGER_RTT_printf(0, "\n");
}

// ✅ 오디오 데이터 분석 (통계)
void analyze_audio_data(uint32_t *buffer, uint32_t sample_count)
{
    uint32_t min_val = 0xFFFFFFFF;
    uint32_t max_val = 0;
    uint64_t sum = 0;
    uint32_t zero_count = 0;
    uint32_t negative_count = 0;

    for (uint32_t i = 0; i < sample_count; i++)
    {
        uint32_t sample = buffer[i];
        
        if (sample < min_val) min_val = sample;
        if (sample > max_val) max_val = sample;
        sum += sample;
        
        if (sample == 0) zero_count++;
        if (sample & 0x80000) negative_count++;  // 20비트에서 음수 판별
    }

    uint32_t average = (uint32_t)(sum / sample_count);
    
    SEGGER_RTT_printf(0, "\n=== AUDIO DATA ANALYSIS ===\n");
    SEGGER_RTT_printf(0, "Sample count: %lu\n", sample_count);
    SEGGER_RTT_printf(0, "Min value: 0x%08lX (%ld)\n", min_val, (int32_t)min_val);
    SEGGER_RTT_printf(0, "Max value: 0x%08lX (%ld)\n", max_val, (int32_t)max_val);
    SEGGER_RTT_printf(0, "Average: 0x%08lX (%ld)\n", average, (int32_t)average);
    SEGGER_RTT_printf(0, "Zero samples: %lu (%.1f%%)\n", zero_count, (float)zero_count * 100.0f / sample_count);
    SEGGER_RTT_printf(0, "Negative samples: %lu (%.1f%%)\n", negative_count, (float)negative_count * 100.0f / sample_count);
    SEGGER_RTT_printf(0, "Dynamic range: %lu\n", max_val - min_val);
    SEGGER_RTT_printf(0, "==========================\n\n");
}

// 기존 함수 (간소화됨)
void save_audio_data_as_text(uint32_t *buffer, uint32_t sample_count, uint32_t callback_number)
{
    if (!ENABLE_AUDIO_TEXT_OUTPUT) return;
    if (callback_number % AUDIO_OUTPUT_INTERVAL != 0) return;
    
    SEGGER_RTT_printf(0, "\n=== AUDIO DATA CALLBACK #%lu ===\n", callback_number);
    
    // 처음 32개 샘플만 실시간 출력 (성능 최적화)
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

void pdm0_callback(pdm_callback_args_t * p_args)
{
    switch(p_args->event)
    {
        case PDM_EVENT_SOUND_DETECTION:
        {
            g_sound_detection_count++;
            SEGGER_RTT_printf(0, "SOUND DETECTED! (#%lu)\n", g_sound_detection_count);
            break;
        }

        case PDM_EVENT_DATA:
        {
            g_data_callback_count++;

            if (g_data_callback_count == 1)
            {
                g_callback_start_count = g_data_callback_count;
                SEGGER_RTT_printf(0, "First callback started! Data collection begins...\n");
            }

            // ✅ 모든 데이터를 큰 버퍼에 수집
            collect_all_audio_data(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES);
            
            // 실시간 샘플 출력
            save_audio_data_as_text(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES, g_data_callback_count);

            // 주기적 상태 보고
            if (g_data_callback_count % 100 == 0)
            {
                SEGGER_RTT_printf(0, "\n[PROGRESS] Callback #%lu\n", g_data_callback_count);
                SEGGER_RTT_printf(0, "   Collected: %lu / %lu samples (%.1f%%)\n", 
                                 g_total_collected_samples, MAX_TOTAL_SAMPLES,
                                 (float)g_total_collected_samples * 100.0f / MAX_TOTAL_SAMPLES);
                                 
                // 현재 데이터 분석
                analyze_audio_data(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES);
            }

            g_callback_end_count = g_data_callback_count;

            // 버퍼가 다 찬 경우 조기 종료
            if (g_total_collected_samples >= MAX_TOTAL_SAMPLES - PDM_CALLBACK_NUM_SAMPLES)
            {
                SEGGER_RTT_printf(0, "\n*** COLLECTION BUFFER FULL - STOPPING EARLY ***\n");
                // 여기서 PDM을 중단하거나 플래그를 설정할 수 있음
            }

            break;
        }

        case PDM_EVENT_ERROR:
        {
            g_error_count++;
            SEGGER_RTT_printf(0, "\n[ERROR #%lu] PDM Error (Flags: 0x%lX)\n",
                             g_error_count, (uint32_t)p_args->error);

            if (p_args->error & PDM_ERROR_BUFFER_OVERWRITE)
            {
                SEGGER_RTT_printf(0, "   BUFFER OVERWRITE - Data may be corrupted!\n");
                // 에러 발생시에도 데이터 수집 계속
                collect_all_audio_data(g_pdm0_buffer, PDM_CALLBACK_NUM_SAMPLES);
            }
            break;
        }

        default:
            SEGGER_RTT_printf(0, "[UNKNOWN] PDM event: %d\n", p_args->event);
            break;
    }
}

void r_pdm_basic_messaging_core0_example(void)
{
    // RTT initialization
    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "\n=== PDM Microphone Complete Data Collection ===\n");
    SEGGER_RTT_printf(0, "Max collection capacity: %lu samples (%.1f seconds)\n", 
                     MAX_TOTAL_SAMPLES, (float)MAX_TOTAL_SAMPLES / 16000.0f);

    /* Open PDM instance. */
    fsp_err_t err = R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);
    if (FSP_SUCCESS != err)
    {
        SEGGER_RTT_printf(0, "PDM Open FAILED: 0x%X\n", err);
        return;
    }
    SEGGER_RTT_printf(0, "PDM Open: SUCCESS\n");

    /* Wait for PDM filters to settle. */
    SEGGER_RTT_printf(0, "Filter settling (%d ms)...\n",
                     (PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US) / 1000);
    R_BSP_SoftwareDelay(PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US, 
                        BSP_DELAY_UNITS_MICROSECONDS);

    /* Enable sound detection */
    pdm_sound_detection_setting_t sound_detection_setting = {
        .sound_detection_lower_limit = PDM_SDE_LOWER_LIMIT,
        .sound_detection_upper_limit = PDM_SDE_UPPER_LIMIT
    };
    R_PDM_SoundDetectionEnable(&g_pdm0_ctrl, sound_detection_setting);

    /* Start receiving PDM data. */
    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm0_buffer, sizeof(g_pdm0_buffer), PDM_CALLBACK_NUM_SAMPLES);
    if (FSP_SUCCESS != err)
    {
        SEGGER_RTT_printf(0, "PDM Start FAILED: 0x%X\n", err);
        return;
    }

    SEGGER_RTT_printf(0, "Recording started! Collecting all data...\n");
    SEGGER_RTT_printf(0, "================================================\n");

    // 더 짧은 시간 (10초) - 큰 데이터량을 고려
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS); 

    SEGGER_RTT_printf(0, "\n================================================\n");
    SEGGER_RTT_printf(0, "Recording finished. Processing data...\n");

    /* Stop PDM */
    R_PDM_Stop(&g_pdm0_ctrl);
    R_PDM_Close(&g_pdm0_ctrl);

    // ✅ 최종 분석
    if (g_total_collected_samples > 0)
    {
        SEGGER_RTT_printf(0, "\n=== FINAL ANALYSIS ===\n");
        analyze_audio_data(g_all_audio_data, g_total_collected_samples);
    }

    // ✅ 모든 수집된 데이터 출력
    SEGGER_RTT_printf(0, "Starting complete data dump...\n");
    SEGGER_RTT_printf(0, "*** WARNING: Large data output ahead! ***\n");
    R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_SECONDS); // 2초 대기
    
    dump_all_collected_data();

    SEGGER_RTT_printf(0, "\n=== COLLECTION COMPLETE ===\n");
    SEGGER_RTT_printf(0, "Total callbacks: %lu\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Total samples collected: %lu\n", g_total_collected_samples);
    SEGGER_RTT_printf(0, "Errors encountered: %lu\n", g_error_count);
    SEGGER_RTT_printf(0, "Collection efficiency: %.1f%%\n", 
                     (float)g_total_collected_samples * 100.0f / 
                     (g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES));
}