/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "extmod/machine_i2c.h"
#include "i2c.h"

#if MICROPY_HW_ENABLE_HW_I2C

STATIC const mp_obj_type_t machine_hard_i2c_type;

#if defined(STM32F0) || defined(STM32F4) || defined(STM32F7)

typedef struct _machine_hard_i2c_obj_t {
    mp_obj_base_t base;
    i2c_t *i2c;
    mp_hal_pin_obj_t scl;
    mp_hal_pin_obj_t sda;
} machine_hard_i2c_obj_t;

STATIC const machine_hard_i2c_obj_t machine_hard_i2c_obj[] = {
    #if defined(MICROPY_HW_I2C1_SCL)
    {{&machine_hard_i2c_type}, I2C1, MICROPY_HW_I2C1_SCL, MICROPY_HW_I2C1_SDA},
    #else
    {{NULL}, NULL, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C2_SCL)
    {{&machine_hard_i2c_type}, I2C2, MICROPY_HW_I2C2_SCL, MICROPY_HW_I2C2_SDA},
    #else
    {{NULL}, NULL, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C3_SCL)
    {{&machine_hard_i2c_type}, I2C3, MICROPY_HW_I2C3_SCL, MICROPY_HW_I2C3_SDA},
    #else
    {{NULL}, NULL, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C4_SCL)
    {{&machine_hard_i2c_type}, I2C4, MICROPY_HW_I2C4_SCL, MICROPY_HW_I2C4_SDA},
    #else
    {{NULL}, NULL, NULL, NULL},
    #endif
};

STATIC void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);

    #if defined(STM32F4)

    uint32_t freq = self->i2c->CR2 & 0x3f;
    uint32_t ccr = self->i2c->CCR;
    if (ccr & 0x8000) {
        // Fast mode, assume duty cycle of 16/9
        freq = freq * 40000 / (ccr & 0xfff);
    } else {
        // Standard mode
        freq = freq * 500000 / (ccr & 0xfff);
    }

    mp_printf(print, "I2C(%u, scl=%q, sda=%q, freq=%u)",
        self - &machine_hard_i2c_obj[0] + 1,
        mp_hal_pin_name(self->scl), mp_hal_pin_name(self->sda),
        freq);

    #else

    uint32_t timingr = self->i2c->TIMINGR;
    uint32_t presc = timingr >> 28;
    uint32_t sclh = timingr >> 8 & 0xff;
    uint32_t scll = timingr & 0xff;
    uint32_t freq = HAL_RCC_GetPCLK1Freq() / (presc + 1) / (sclh + scll + 2);
    mp_printf(print, "I2C(%u, scl=%q, sda=%q, freq=%u, timingr=0x%08x)",
        self - &machine_hard_i2c_obj[0] + 1,
        mp_hal_pin_name(self->scl), mp_hal_pin_name(self->sda),
        freq, timingr);

    #endif
}

void machine_hard_i2c_init(machine_hard_i2c_obj_t *self, uint32_t freq, uint32_t timeout) {
    (void)timeout;
    i2c_init(self->i2c, self->scl, self->sda, freq);
}

int machine_hard_i2c_readfrom(mp_obj_base_t *self_in, uint16_t addr, uint8_t *dest, size_t len, bool stop) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return i2c_readfrom(self->i2c, addr, dest, len, stop);
}

int machine_hard_i2c_writeto(mp_obj_base_t *self_in, uint16_t addr, const uint8_t *src, size_t len, bool stop) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return i2c_writeto(self->i2c, addr, src, len, stop);
}

#else

// No hardware I2C driver for this MCU so use the software implementation

typedef mp_machine_soft_i2c_obj_t machine_hard_i2c_obj_t;

STATIC machine_hard_i2c_obj_t machine_hard_i2c_obj[] = {
    #if defined(MICROPY_HW_I2C1_SCL)
    {{&machine_hard_i2c_type}, 1, 500, MICROPY_HW_I2C1_SCL, MICROPY_HW_I2C1_SDA},
    #else
    {{NULL}, 0, 0, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C2_SCL)
    {{&machine_hard_i2c_type}, 1, 500, MICROPY_HW_I2C2_SCL, MICROPY_HW_I2C2_SDA},
    #else
    {{NULL}, 0, 0, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C3_SCL)
    {{&machine_hard_i2c_type}, 1, 500, MICROPY_HW_I2C3_SCL, MICROPY_HW_I2C3_SDA},
    #else
    {{NULL}, 0, 0, NULL, NULL},
    #endif
    #if defined(MICROPY_HW_I2C4_SCL)
    {{&machine_hard_i2c_type}, 1, 500, MICROPY_HW_I2C4_SCL, MICROPY_HW_I2C4_SDA},
    #else
    {{NULL}, 0, 0, NULL, NULL},
    #endif
};

STATIC void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "I2C(%u, scl=%q, sda=%q, freq=%u, timeout=%u)",
        self - &machine_hard_i2c_obj[0] + 1,
        self->scl->name, self->sda->name, 500000 / self->us_delay, self->us_timeout);
}

STATIC void machine_hard_i2c_init(machine_hard_i2c_obj_t *self, uint32_t freq, uint32_t timeout) {
    // set parameters
    if (freq >= 1000000) {
        // allow fastest possible bit-bang rate
        self->us_delay = 0;
    } else {
        self->us_delay = 500000 / freq;
        if (self->us_delay == 0) {
            self->us_delay = 1;
        }
    }

    self->us_timeout = timeout;

    // init pins
    mp_hal_pin_open_drain(self->scl);
    mp_hal_pin_open_drain(self->sda);
}

#define machine_hard_i2c_readfrom mp_machine_soft_i2c_readfrom
#define machine_hard_i2c_writeto mp_machine_soft_i2c_writeto

#endif

/******************************************************************************/
/* MicroPython bindings for machine API                                       */

mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // parse args
    enum { ARG_id, ARG_scl, ARG_sda, ARG_freq, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_scl, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_sda, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 400000} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // work out i2c bus
    int i2c_id = 0;
    if (MP_OBJ_IS_STR(args[ARG_id].u_obj)) {
        const char *port = mp_obj_str_get_str(args[ARG_id].u_obj);
        if (0) {
        #ifdef MICROPY_HW_I2C1_NAME
        } else if (strcmp(port, MICROPY_HW_I2C1_NAME) == 0) {
            i2c_id = 1;
        #endif
        #ifdef MICROPY_HW_I2C2_NAME
        } else if (strcmp(port, MICROPY_HW_I2C2_NAME) == 0) {
            i2c_id = 2;
        #endif
        #ifdef MICROPY_HW_I2C3_NAME
        } else if (strcmp(port, MICROPY_HW_I2C3_NAME) == 0) {
            i2c_id = 3;
        #endif
        } else {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "I2C(%s) doesn't exist", port));
        }
    } else {
        i2c_id = mp_obj_get_int(args[ARG_id].u_obj);
        if (i2c_id < 1 || i2c_id > MP_ARRAY_SIZE(machine_hard_i2c_obj)
            || machine_hard_i2c_obj[i2c_id - 1].base.type == NULL) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "I2C(%d) doesn't exist", i2c_id));
        }
    }

    // get static peripheral object
    machine_hard_i2c_obj_t *self = (machine_hard_i2c_obj_t*)&machine_hard_i2c_obj[i2c_id - 1];

    // here we would check the scl/sda pins and configure them, but it's not implemented
    if (args[ARG_scl].u_obj != MP_OBJ_NULL || args[ARG_sda].u_obj != MP_OBJ_NULL) {
        mp_raise_ValueError("explicit choice of scl/sda is not implemented");
    }

    // initialise the I2C peripheral
    machine_hard_i2c_init(self, args[ARG_freq].u_int, args[ARG_timeout].u_int);

    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .readfrom = machine_hard_i2c_readfrom,
    .writeto = machine_hard_i2c_writeto,
};

STATIC const mp_obj_type_t machine_hard_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .print = machine_hard_i2c_print,
    .make_new = machine_hard_i2c_make_new,
    .protocol = &machine_hard_i2c_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_soft_i2c_locals_dict,
};

#endif // MICROPY_HW_ENABLE_HW_I2C
