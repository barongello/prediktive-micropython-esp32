#include "py/runtime.h"
#include "py/obj.h"

int add(int a, int b)
{
  return a + b;
}

static mp_obj_t prediktive_library_add(mp_obj_t a_obj, mp_obj_t b_obj)
{
  int a = mp_obj_get_int(a_obj);
  int b = mp_obj_get_int(b_obj);
  int result = add(a, b);

  return mp_obj_new_int(result);
}

static MP_DEFINE_CONST_FUN_OBJ_2(prediktive_library_add_obj, prediktive_library_add);

static const mp_rom_map_elem_t prediktive_library_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR__prediktive_library)},
    {MP_ROM_QSTR(MP_QSTR_add), MP_ROM_PTR(&prediktive_library_add_obj)},
};

static MP_DEFINE_CONST_DICT(prediktive_library_module_globals, prediktive_library_module_globals_table);

const mp_obj_module_t prediktive_library_user_cmodule = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&prediktive_library_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__prediktive_library, prediktive_library_user_cmodule);
