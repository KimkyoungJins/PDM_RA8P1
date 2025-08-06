#include "hal_data.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#define PDM_BUFFER_NUM_SAMPLES 1024 // Number of samples to store in data buffer
#define PDM_CALLBACK_NUM_SAMPLES PDM_BUFFER_NUM_SAMPLES / 2 // Number of samples to store in data buffer before getting a callback
#define PDM_MIC_STARTUP_TIME_US 35000 // PDM microphone startup time, RA8P1 EK mic startup time is 35ms.
#define PDM_SDE_UPPER_LIMIT 8000  // 임계값 설정 바꿈
#define PDM_SDE_LOWER_LIMIT 0xE0C0  // -80000
#define PDM0_FILTER_SETTLING_TIME_US (25000U) // 25ms 안전한 값으로 

uint32_t g_pdm0_buffer[PDM_BUFFER_NUM_SAMPLES];

// 통계를 위한 카운터들
static uint32_t g_sound_detection_count = 0;
static uint32_t g_data_callback_count = 0;
static uint32_t g_error_count = 0;

void r_pdm_basic_messaging_core0_example(void)
{
    // RTT 초기화
    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "\n=== PDM Microphone Test Start ===\n");

    /* Open PDM instance. */
    fsp_err_t err = R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "✅ PDM Open: SUCCESS\n");
    }

    else
    {
        SEGGER_RTT_printf(0, "❌ PDM Open: FAILED (Error: 0x%X)\n", err);
        return;
    }

    /* Wait for PDM filters to settle. Use settling time calculated by e2 studio.
     * Also wait for PDM mic to startup. */
    SEGGER_RTT_printf(0, "⏳ Waiting for filter settling and mic startup (%d ms)...\n",
                     (PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US) / 1000);

    R_BSP_SoftwareDelay(PDM0_FILTER_SETTLING_TIME_US + PDM_MIC_STARTUP_TIME_US, BSP_DELAY_UNITS_MICROSECONDS);


    SEGGER_RTT_printf(0, "✅ Filter and Mic ready!\n");
    SEGGER_RTT_printf(0, "\n");
    SEGGER_RTT_printf(0, "\n");


    /* Enable sound detection (if desired). */
    pdm_sound_detection_setting_t sound_detection_setting =
    {
        .sound_detection_lower_limit = PDM_SDE_LOWER_LIMIT,
        .sound_detection_upper_limit = PDM_SDE_UPPER_LIMIT
    };

    err = R_PDM_SoundDetectionEnable(&g_pdm0_ctrl, sound_detection_setting);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "🔊 Sound Detection: ENABLED (Lower: 0x%X, Upper: %d)\n",
                         PDM_SDE_LOWER_LIMIT, PDM_SDE_UPPER_LIMIT);
    }
    else
    {
        SEGGER_RTT_printf(0, "❌ Sound Detection Enable: FAILED (Error: 0x%X)\n", err);
    }

    /* Start receiving PDM data. */
    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm0_buffer, sizeof(g_pdm0_buffer), PDM_CALLBACK_NUM_SAMPLES);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "🎬 PDM Recording: STARTED (Buffer: %d samples, Callback every: %d samples)\n",
                         PDM_BUFFER_NUM_SAMPLES, PDM_CALLBACK_NUM_SAMPLES);
        SEGGER_RTT_printf(0, "📊 Listening for audio... (Press any key to stop)\n");
        SEGGER_RTT_printf(0, "================================================\n");
    }
    
    else
    {
        SEGGER_RTT_printf(0, "❌ PDM Start: FAILED (Error: 0x%X)\n", err);
    }

    // 여기서 실제로는 사용자 입력이나 타이머를 기다릴 수 있습니다
    // 예시를 위해 10초 후 자동 종료
    SEGGER_RTT_printf(0, "🕐 Recording for 10 seconds...\n");

    // 10초를 기다린다 10초 동안 PDM이 실행되기 때문에 그 동안 콜백함수가 계속 호출됨
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS); 

    SEGGER_RTT_printf(0, "\n================================================\n");

    /* Stop receiving PDM data. */
    err = R_PDM_Stop(&g_pdm0_ctrl);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "✅ PDM Stop: SUCCESS\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "❌ PDM Stop: FAILED (Error: 0x%X)\n", err);
    }

    /* Close PDM instance. */
    err = R_PDM_Close(&g_pdm0_ctrl);
    if (FSP_SUCCESS == err)
    {
        SEGGER_RTT_printf(0, "✅ PDM Close: SUCCESS\n");
    }
    else
    {
        SEGGER_RTT_printf(0, "❌ PDM Close: FAILED (Error: 0x%X)\n", err);
    }

    // 최종 통계 출력
    SEGGER_RTT_printf(0, "\n=== Final Statistics ===\n");
    SEGGER_RTT_printf(0, "🔊 Sound Detections: %lu times\n", g_sound_detection_count);
    SEGGER_RTT_printf(0, "📦 Data Callbacks: %lu times\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "❌ Errors: %lu times\n", g_error_count);
    SEGGER_RTT_printf(0, "📊 Total Audio Samples Processed: %lu\n", g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES);
    SEGGER_RTT_printf(0, "========================\n");
}

void pdm0_callback(pdm_callback_args_t * p_args)
{

    switch(p_args->event)
    {
        case PDM_EVENT_SOUND_DETECTION:
        {
            g_sound_detection_count++;
            
            if(g_sound_detection_count % 1000 == 0){

                SEGGER_RTT_printf(0, "🔔 SOUND DETECTED! (#%lu) - Loud sound threshold exceeded\n",
                g_sound_detection_count);
            }

            break;
        }

        case PDM_EVENT_DATA:
        {
            g_data_callback_count++;

            // 주기적으로 상세 정보 출력 (10번마다)
            if (g_data_callback_count % 10 == 0)
            {
                SEGGER_RTT_printf(0, "📦 [DATA] Callback #%lu - %d samples ready (Total: %lu samples)\n",
                                 g_data_callback_count,
                                 PDM_CALLBACK_NUM_SAMPLES,
                                 g_data_callback_count * PDM_CALLBACK_NUM_SAMPLES);

                // 첫 몇 개 샘플 값 출력 (디버깅용)
                // 데이터 핵사값으로 바꾸어서 출력하기 
                SEGGER_RTT_printf(0, "   📊 Sample values: [0]=%08x, [1]=%08x, [2]=%08x, [3]=%08x\n",
                                 g_pdm0_buffer[0], g_pdm0_buffer[1], g_pdm0_buffer[2], g_pdm0_buffer[3]);
            }
            else if (g_data_callback_count % 50 == 0)
            {
                // 50번마다 점 출력으로 활동 표시
                SEGGER_RTT_printf(0, ".");
            }

            break;
        }

        case PDM_EVENT_ERROR:
        {
            g_error_count++;

            SEGGER_RTT_printf(0, "\n🚨 [ERROR #%lu] PDM Error Detected! (Flags: 0x%X)\n",
                             g_error_count, p_args->error);

            if (p_args->error & PDM_ERROR_SHORT_CIRCUIT)
            {
                SEGGER_RTT_printf(0, "   ⚡ SHORT CIRCUIT detected based on configured counts\n");
            }

            if (p_args->error & PDM_ERROR_OVERVOLTAGE_LOWER)
            {
                SEGGER_RTT_printf(0, "   ⬇️  UNDER-VOLTAGE detected (below lower limit: 0x%X)\n", PDM_SDE_LOWER_LIMIT);
                SEGGER_RTT_printf(0, "adsf", PDM_SDE_LOWER_LIMIT);
            }

            if (p_args->error & PDM_ERROR_OVERVOLTAGE_UPPER)
            {
                SEGGER_RTT_printf(0, "   ⬆️  OVER-VOLTAGE detected (above upper limit: %d)\n", PDM_SDE_UPPER_LIMIT);
            }

            if (p_args->error & PDM_ERROR_BUFFER_OVERWRITE)
            {
                SEGGER_RTT_printf(0, "   💥 BUFFER OVERWRITE occurred - Data loss possible!\n");
                SEGGER_RTT_printf(0, "   💡 Suggestion: Process data faster or increase buffer size\n");
            }

            // 심각한 에러의 경우 추가 정보
            if (p_args->error & (PDM_ERROR_SHORT_CIRCUIT | PDM_ERROR_BUFFER_OVERWRITE))
            {
                SEGGER_RTT_printf(0, "   🔥 CRITICAL ERROR - Consider restarting PDM\n");
            }

            break;
        }

        default:
        {
            SEGGER_RTT_printf(0, "❓ [UNKNOWN] Unexpected PDM event: %d\n", p_args->event);
            break;
        }
    }
}

// 추가 유틸리티 함수들
void pdm_print_status(void)
{
    SEGGER_RTT_printf(0, "\n=== PDM Status ===\n");
    SEGGER_RTT_printf(0, "Sound Detections: %lu\n", g_sound_detection_count);
    SEGGER_RTT_printf(0, "Data Callbacks: %lu\n", g_data_callback_count);
    SEGGER_RTT_printf(0, "Errors: %lu\n", g_error_count);
    SEGGER_RTT_printf(0, "================\n");
}

void pdm_reset_counters(void)
{
    g_sound_detection_count = 0;
    g_data_callback_count = 0;
    g_error_count = 0;
    SEGGER_RTT_printf(0, "🔄 Counters reset\n");
}
