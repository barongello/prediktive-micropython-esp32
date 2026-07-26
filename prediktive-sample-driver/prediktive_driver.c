#include "py/runtime.h"
#include "py/obj.h"
#include "py/mpthread.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"

#define LED_PIN GPIO_NUM_32
#define BUTTON_PIN GPIO_NUM_33

static mp_obj_t button_callback = mp_const_none;

MP_REGISTER_ROOT_POINTER(mp_obj_t button_callback);

static mp_obj_t button_callback_run(mp_obj_t arg)
{
  if (button_callback != mp_const_none)
  {
    mp_call_function_1(button_callback, arg);
  }

  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(button_callback_run_obj, button_callback_run);

static void IRAM_ATTR button_isr_handler(void *arg)
{
  mp_sched_schedule(MP_OBJ_FROM_PTR(&button_callback_run_obj), mp_const_none);
}

static mp_obj_t prediktive_driver_init(void)
{
  gpio_config_t led_conf = {
      .pin_bit_mask = (1ULL << LED_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  gpio_config(&led_conf);

  gpio_config_t btn_conf = {
      .pin_bit_mask = (1ULL << BUTTON_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE,
  };

  gpio_config(&btn_conf);

  esp_err_t err = gpio_install_isr_service(0);

  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
  {
    mp_raise_msg_varg(&mp_type_OSError,
                      MP_ERROR_TEXT("failed to install GPIO ISR service: %d"), err);
  }

  gpio_isr_handler_remove(BUTTON_PIN);

  err = gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

  if (err != ESP_OK)
  {
    mp_raise_msg_varg(&mp_type_OSError,
                      MP_ERROR_TEXT("failed to add GPIO ISR handler: %d"), err);
  }

  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_0(prediktive_driver_init_obj, prediktive_driver_init);

static mp_obj_t prediktive_driver_led_set(mp_obj_t value_obj)
{
  int value = mp_obj_get_int(value_obj);

  gpio_set_level(LED_PIN, value ? 1 : 0);

  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(prediktive_driver_led_set_obj, prediktive_driver_led_set);

static mp_obj_t prediktive_driver_button_read(void)
{
  int level = gpio_get_level(BUTTON_PIN);

  return mp_obj_new_bool(level == 0);
}

static MP_DEFINE_CONST_FUN_OBJ_0(prediktive_driver_button_read_obj, prediktive_driver_button_read);

static mp_obj_t prediktive_driver_set_button_callback(mp_obj_t callback_obj)
{
  if (callback_obj != mp_const_none && !mp_obj_is_callable(callback_obj))
  {
    mp_raise_ValueError(MP_ERROR_TEXT("callback must be callable or None"));
  }

  button_callback = callback_obj;

  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(prediktive_driver_set_button_callback_obj, prediktive_driver_set_button_callback);

static const mp_rom_map_elem_t prediktive_driver_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR__prediktive_driver)},
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&prediktive_driver_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_led_set), MP_ROM_PTR(&prediktive_driver_led_set_obj)},
    {MP_ROM_QSTR(MP_QSTR_button_read), MP_ROM_PTR(&prediktive_driver_button_read_obj)},
    {MP_ROM_QSTR(MP_QSTR_set_button_callback), MP_ROM_PTR(&prediktive_driver_set_button_callback_obj)},
};

static MP_DEFINE_CONST_DICT(prediktive_driver_module_globals, prediktive_driver_module_globals_table);

const mp_obj_module_t prediktive_driver_user_cmodule = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&prediktive_driver_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__prediktive_driver, prediktive_driver_user_cmodule);
