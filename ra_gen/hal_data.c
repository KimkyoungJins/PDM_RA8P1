/* generated HAL source file - do not edit */
#include "hal_data.h"
dmac_instance_ctrl_t g_transfer0_ctrl;
transfer_info_t g_transfer0_info =
{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
  .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
  .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE,
  .transfer_settings_word_b.mode = TRANSFER_MODE_REPEAT_BLOCK,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 4,
  .length = 1024, };
const dmac_extended_cfg_t g_transfer0_extend =
{ .offset = 1, .src_buffer_size = 1,
#if defined(VECTOR_NUMBER_DMAC0_INT)
    .irq                 = VECTOR_NUMBER_DMAC0_INT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
  .ipl = (5),
  .channel = 0, .p_callback = NULL, .p_context = NULL, .activation_source = ELC_EVENT_PDM_DAT2, };
const transfer_cfg_t g_transfer0_cfg =
{ .p_info = &g_transfer0_info, .p_extend = &g_transfer0_extend, };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 =
{ .p_ctrl = &g_transfer0_ctrl, .p_cfg = &g_transfer0_cfg, .p_api = &g_transfer_on_dmac };
pdm_instance_ctrl_t g_pdm0_ctrl;

/** PDM instance configuration */
const pdm_extended_cfg_t g_pdm0_cfg_extend =
{ .clock_div = PDM_CLOCK_DIV_2,

  /** Function Settings. */
  .short_circuit_detection_enable = PDM_SHORT_CIRCUIT_DISABLED,
  .over_voltage_lower_limit_detection_enable = PDM_OVERVOLTAGE_LOWER_LIMIT_DISABLED,
  .over_voltage_upper_limit_detection_enable = PDM_OVERVOLTAGE_UPPER_LIMIT_DISABLED,
  .buffer_overwrite_detection_enable = PDM_BUFFER_OVERWRITE_DETECTION_ENABLED,

  /** Filter Settings. */
  .moving_average_mode = PDM_MOVING_AVERAGE_MODE_1_ORDER,
  .low_pass_filter_shift = PDM_LPF_RIGHT_SHIFT_0,
  .compensation_filter_shift = PDM_COMPENSATION_FILTER_RIGHT_SHIFT_0,
  .high_pass_filter_shift = PDM_HPF_RIGHT_SHIFT_0,
  .sinc_filter_mode = PDM_SINC_FILTER_MODE_4,
  .sincrng = PDM2_CALCULATED_SINCRNG_VALUE,
  .sincdec = PDM2_CALCULATED_SINCDEC_VALUE,
  .hpf_coefficient_s0 = 0x3F61,
  .hpf_coefficient_k1 = 0x3EC1,
  .hpf_coefficient_h =
  { 0x4000, 0xC000 },
  .compensation_filter_coefficient_h =
  { 0x1FE8, 0x0039, 0x003C, 0x1E56, 0x01DC, 0x06E1, 0x01DC, 0x1E56, 0x003C, 0x0039, 0x1FE8 },
  .lpf_coefficient_h0 = 0x0400,
  .lpf_coefficient_h1 =
  { 0x1FF8,
    0x000A,
    0x1FF0,
    0x0018,
    0x1FDC,
    0x0034,
    0x1FB3,
    0x0076,
    0x1F2E,
    0x0289,
    0x0289,
    0x1F2E,
    0x0076,
    0x1FB3,
    0x0034,
    0x1FDC,
    0x0018,
    0x1FF0,
    0x000A,
    0x1FF8 },

  /** Data Threshold. */
  .interrupt_threshold = PDM_INTERRUPT_THRESHOLD_16,

  /** Short-Circuit Detection. */
  .short_circuit_count_h = 0,
  .short_circuit_count_l = 0,

  /** Overvoltage Detection. */
  .overvoltage_detection_lower_limit = 0xE0C0,
  .overvoltage_detection_upper_limit = 8000,

};

/** PDM interface configuration */
const pdm_cfg_t g_pdm0_cfg =
{ .unit = 0, .channel = 2, .pcm_width = PDM_PCM_WIDTH_20_BITS_0_18, .pcm_edge = PDM_INPUT_DATA_EDGE_RISE,

#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx                         = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .p_callback = pdm0_callback,
  .p_context = NULL, .p_extend = &g_pdm0_cfg_extend,

#if defined(VECTOR_NUMBER_PDM_SDET)
                .sdet_irq                              = PDM_SDET_IRQn,
#else
  .sdet_irq = FSP_INVALID_VECTOR,
#endif
  .sdet_ipl = (1),

#if defined(VECTOR_NUMBER_PDM_DAT2)
                .dat_irq                               = PDM_DAT2_IRQn,
#else
  .dat_irq = FSP_INVALID_VECTOR,
#endif
  .dat_ipl = (2),

#if defined(VECTOR_NUMBER_PDM_ERR2)
                .err_irq                               = PDM_ERR2_IRQn,
#else
  .err_irq = FSP_INVALID_VECTOR,
#endif
  .err_ipl = (3), };

/* Instance structure to use this module. */
const pdm_instance_t g_pdm0 =
{ .p_ctrl = &g_pdm0_ctrl, .p_cfg = &g_pdm0_cfg, .p_api = &g_pdm_on_pdm };
void g_hal_init(void)
{
    g_common_init ();
}
