#include "app_hogpd.h"
#include "app_hogpd_task.h"
#include "user_gamepad.h"
#include "user_hogpd_config.h"
#include "wkupct_quadec.h"
#include "arch_console.h"
#include "app_entry_point.h"
#include "adc.h"

struct gamepad_digitizer_report_t gamepad_digitizer_report __SECTION_ZERO("retention_mem_area0");    //@RETENTION MEMORY
struct gamepad_digitizer_map_joystick_t LS_config, RS_config __SECTION_ZERO("retention_mem_area0");

void user_gamepad_register_button(button_map_t button_index, GPIO_PORT port, GPIO_PIN pin, uint8_t polarity)
{
}

void user_gamepad_button_config(void)
{
}


void user_gamepad_init(void)
{
}

void user_gamepad_send_report(void)
{
}

uint8_t user_get_avlb_mt_index(void)
{
    return 0xFF;
}

bool user_add_mt_index(void)
{
    return false;
}

void user_remove_mt_index(uint8_t index)
{
}

void user_gamepad_button_cb(void)
{
}

void user_gamepad_enable_buttons(void)
{
}

void user_gamepad_set_button_location(button_map_t button_index, struct gamepad_digitizer_map_button_t config)
{
}

void user_gamepad_config_joystick(struct gamepad_digitizer_map_joystick_t *config,
                                  struct gamepad_digitizer_map_joystick_t params)
{
}

void user_gamepad_config_digitizer(void)
{
}

void user_usDelay(uint32_t nof_us)
{
    while(nof_us--) {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}

uint32_t adc_get_raw_sample(adc_input_se_t channel)
{
    uint32_t adc_sample;
    adc_config_t cfg = { .input_mode = ADC_INPUT_MODE_SINGLE_ENDED,
                         .input = channel,
                         .smpl_time_mult = 1,
                         .continuous = false,
                         .input_attenuator = ADC_INPUT_ATTN_4X,
                         .oversampling = 2 };

    adc_init(&cfg);
    adc_sample = adc_get_sample();
    return adc_sample;
}

bool deadzone_check(uint32_t raw_x, uint32_t raw_y)
{
    return true;
}

uint8_t user_sample_conv(uint16_t input, uint16_t cap)
{
    return 0;
}

void user_gamepad_update_joystick(void)
{
}

void user_gamepad_axis_polling_cb(void)
{
}


void user_gamepad_toggle_axis_polling(bool on)
{
}

/**
 ****************************************************************************************
 * HID event handler entrance
 ****************************************************************************************
 */

void app_hid_gamepad_event_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
#if BLE_HID_DEVICE
    app_hogpd_process_handler(msgid, param, dest_id, src_id);
#endif
}
