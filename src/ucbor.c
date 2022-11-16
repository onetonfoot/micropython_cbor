/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Jeremy Herbert
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
#include "py/dynruntime.h"
#include "cbor.h"

// Automatically detect if this module should include double-precision code.
// If double precision is supported by the target architecture then it can
// be used in native module regardless of what float setting the target
// MicroPython runtime uses (being none, float or double).
#if defined(__i386__) || defined(__x86_64__) || (defined(__ARM_FP) && (__ARM_FP & 8))

// #include <string.h>

#define USE_DOUBLE 1
#else
#define USE_DOUBLE 0
#endif

int memcmp(const void *str1, const void *str2, size_t count) {
    const unsigned char *s1 = (const unsigned char *) str1;
    const unsigned char *s2 = (const unsigned char *) str2;

    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

unsigned int strlen(const char *s)
{
    unsigned int count = 0;
    while(*s!='\0')
    {
        count++;
        s++;
    }
    return count;
}

void *memset(void *s, int c,  unsigned int len)
{
    unsigned char* p=s;
    while(len--)
    {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

void *malloc(size_t n) {
    void *ptr = m_malloc(n);
    return ptr;
}

void free(void *ptr) {
    m_free(ptr);
}

STATIC mp_obj_t cbor_it_to_mp_obj_recursive(CborValue *it, mp_obj_t parent_obj) {
    bool dict_value_next = false;
    mp_obj_t dict_key = mp_const_none;
    CborError err;

    // need to use a smaller switch statement
    // https://git.furworks.de/coreboot-mirror/chrome-ec/commit/270e4248192253d934f0a4d66d6cf67972b68c7f

    while (!cbor_value_at_end(it)) {

        CborType type = cbor_value_get_type(it);

        mp_obj_t next_element = mp_const_none;

        if (type == CborIntegerType){
            int64_t val;
            cbor_value_get_int64(it, &val);
            next_element = mp_obj_new_int(val);
        } else if(type == CborByteStringType) {
            uint8_t *buf;
            size_t n;
            err = cbor_value_dup_byte_string(it, &buf, &n, it);
            if (err)
                mp_raise_ValueError("parse bytestring failed");
            next_element = mp_obj_new_bytes(buf, n);
            m_free(buf);
        } else if (type == CborTextStringType){
            char *buf;
            size_t n;
            err = cbor_value_dup_text_string(it, &buf, &n, it);
            if (err)
                mp_raise_ValueError("parse string failed");
            next_element = mp_obj_new_str(buf, n);
            m_free(buf);

        } else if (type == CborTagType) {
            mp_raise_ValueError("unknown tag present");
        } else if (type == CborSimpleType) {
            mp_raise_ValueError("unknown simple value present");
        } else if (type == CborNullType) {
            next_element = mp_const_none;
        } else if (type == CborUndefinedType) {
            mp_raise_ValueError("undefined type encountered");
        } else if (type == CborBooleanType) {
            bool val;
            cbor_value_get_boolean(it, &val);
            next_element = mp_obj_new_bool(val);
        } else if (type  == CborDoubleType) {
            double val;
            cbor_value_get_double(it, &val);
            next_element = mp_obj_new_float_from_d(val);
        } else if (type == CborFloatType ) {
            float val;
            cbor_value_get_float(it, &val);
            next_element = mp_obj_new_float_from_f(val);
        } else if (type == CborHalfFloatType ){
            mp_raise_NotImplementedError("half float type not supported");
        } else if (type == CborInvalidType) {
            mp_raise_ValueError("invalid type encountered");
        } else if (type == CborArrayType || type == CborMapType) {
            CborValue recursed;
            assert(cbor_value_is_container(it));
            err = cbor_value_enter_container(it, &recursed);
            if (err)
                mp_raise_ValueError("parse error");

            if (type == CborArrayType) {
                next_element = mp_obj_new_list(0, NULL);
            } else {
                next_element = mp_obj_new_dict(0);
            }

            cbor_it_to_mp_obj_recursive(&recursed, next_element);
            err = cbor_value_leave_container(it, &recursed);
            if (err)
                mp_raise_ValueError("parse error");

        } else {
            assert(false); // should never happen
        }

        if (parent_obj == mp_const_none) {
            return next_element;
        } else {
            const mp_obj_type_t *parent_type = mp_obj_get_type(parent_obj);

            if (parent_type == &mp_type_list) {
                mp_obj_list_append(parent_obj, next_element);
            } else if (parent_type == &mp_type_dict) {
                if (dict_value_next) {
                    mp_obj_dict_store(parent_obj, dict_key, next_element);
                    dict_value_next = false;
                    dict_key = mp_const_none;
                } else {
                    dict_key = next_element;
                    dict_value_next = true;
                }
            } else {
                assert(false); // should never happen
            }
        }

        // some element types can have a variable length, so the iterator is advanced during processing.
        // here we advance it for all other types
        if (type != CborArrayType &&
            type != CborMapType &&
            type != CborTextStringType &&
            type != CborByteStringType) {
            err = cbor_value_advance_fixed(it);
            if (err)
                mp_raise_ValueError("parse error");
        }
    }

    if (dict_value_next)
        mp_raise_ValueError("key with no value in map");

    return parent_obj;
}

STATIC mp_obj_t cbor_loads(mp_obj_t buf_obj) {
    // get underlying buffer info, and make sure it contains bytes
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_obj, &bufinfo, MP_BUFFER_READ);
    if (bufinfo.typecode != 'B' && bufinfo.typecode != 'b') {
        mp_raise_ValueError("expecting bytes or bytearray");
    }

    CborParser parser;
    CborValue it;
    CborError err = cbor_parser_init(bufinfo.buf, bufinfo.len, CborValidateStrictMode, &parser, &it);
    if (err != CborNoError) {
        mp_raise_ValueError("tinycbor init failed");
    }

    mp_obj_t result = cbor_it_to_mp_obj_recursive(&it, mp_const_none);

    return result;
}

STATIC uint8_t *mp_obj_to_cbor_text_recursive(mp_obj_t x_obj, CborEncoder *parent_enc, size_t *encoded_len) {
    CborEncoder new_enc;
    CborEncoder *enc;

    if (parent_enc == NULL) {
        enc = &new_enc;
    } else {
        enc = parent_enc;
    }

    CborError err = CborNoError;
    const mp_obj_type_t *parent_type = mp_obj_get_type(x_obj);

    size_t bufsize = 64;
    uint8_t *buf = NULL;

    // this uses either 1 or 2 pass encoding. If the first pass runs out of memory, it continues to encode in order to
    // count the final encoded size. Then it reallocates the buffer to the appropriate size and encodes again.
    while (1) {
        if (parent_enc == NULL) {
            buf = m_realloc(buf, bufsize);
            cbor_encoder_init(enc, buf, bufsize, CborValidateStrictMode);
        }

        if (parent_type == &mp_type_NoneType) {
            err = cbor_encode_null(enc);
        } else if (parent_type == &mp_type_int) {
            err = cbor_encode_int(enc, mp_obj_get_int(x_obj));
        } else if (parent_type->name == MP_QSTR_float) {
            #if MICROPY_FLOAT_IMPL == MICROPY_FLOAT_IMPL_DOUBLE
                err = cbor_encode_double(enc, mp_obj_get_float_to_d(x_obj));
            #else
                err = cbor_encode_float(enc, mp_obj_get_float_to_f(x_obj));
            #endif
        } else if (parent_type == &mp_type_str) {
            mp_obj_t len_in = mp_obj_len(x_obj);
            size_t len = mp_obj_get_int(len_in);
            err = cbor_encode_text_string(enc, mp_obj_str_get_str(x_obj), len);
        } else if (parent_type == &mp_type_bytes) {
            mp_buffer_info_t bufinfo;
            // This be used to get at the data for a bytestring/bytearray/string.
            mp_get_buffer_raise(x_obj, &bufinfo, MP_BUFFER_READ);
            err = cbor_encode_byte_string(enc, bufinfo.buf, bufinfo.len);
        } else if (parent_type == &mp_type_list || parent_type == &mp_type_tuple) {
            size_t list_len = mp_obj_get_int(mp_obj_len(x_obj));

            CborEncoder list_enc;
            err = cbor_encoder_create_array(enc, &list_enc, list_len);
            if (err != CborNoError && err != CborErrorOutOfMemory) {
                mp_raise_ValueError("Failed to encode array");
            }
            for (size_t i = 0; i < list_len; i++) {
                mp_obj_t inner_obj = mp_obj_subscr(x_obj, mp_obj_new_int(i), MP_OBJ_SENTINEL);
                mp_obj_to_cbor_text_recursive(inner_obj, &list_enc, NULL);
            }
            err = cbor_encoder_close_container(enc, &list_enc);
        } else if (parent_type == &mp_type_dict) {
            size_t dict_len = mp_obj_get_int(mp_obj_len(x_obj));

            CborEncoder dict_enc;
            err = cbor_encoder_create_map(enc, &dict_enc, dict_len);
            if (err != CborNoError && err != CborErrorOutOfMemory) {
                mp_raise_ValueError("Failed to encode array");
            }
            mp_obj_t dest[2];
            mp_fun_table.load_method(x_obj, MP_QSTR_items, dest);
            mp_obj_t dict_iter = mp_fun_table.call_method_n_kw(0, 0, dest);
            mp_obj_iter_buf_t iter_buf;
            mp_fun_table.getiter(dict_iter, &iter_buf);
            mp_obj_t item;
            while ((item = mp_fun_table.iternext(&iter_buf)) != MP_OBJ_NULL) {
                // item is a tuple with structure: (key, val)
                mp_obj_to_cbor_text_recursive(mp_obj_subscr(item, mp_obj_new_int(0), MP_OBJ_SENTINEL),
                                              &dict_enc, NULL);

                mp_obj_to_cbor_text_recursive(mp_obj_subscr(item, mp_obj_new_int(1), MP_OBJ_SENTINEL),
                                              &dict_enc, NULL);
            }
            err = cbor_encoder_close_container(enc, &dict_enc);
        } else {
            mp_raise_ValueError("Found object which cannot be encoded");
        }

        if (err == CborNoError) {
            break;
        } else if (err == CborErrorOutOfMemory) {
            if (parent_enc == NULL) {
                bufsize += cbor_encoder_get_extra_bytes_needed(enc);
                continue;
            } else {
                if (buf != NULL) {
                    m_free(buf);
                }
                return NULL;
            }
        } else {
            if (buf != NULL) {
                m_free(buf);
            }
            mp_raise_ValueError("CBOR encoding failed");
        }
    }

    if (parent_enc == NULL) {
        *encoded_len = cbor_encoder_get_buffer_size(enc, buf);
        return buf;
    } else {
        return NULL;
    }
}

STATIC mp_obj_t cbor_dumps(mp_obj_t x_obj) {
    size_t len = 0;
    uint8_t *buf = mp_obj_to_cbor_text_recursive(x_obj, NULL, &len);

    if (buf == NULL) {
        mp_raise_ValueError("CBOR encoding failed");
    }

    mp_obj_t result = mp_obj_new_bytes(buf, len);
    m_free(buf);

    return result;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(cbor_loads_obj, cbor_loads);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cbor_dumps_obj, cbor_dumps);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR_loads, MP_OBJ_FROM_PTR(&cbor_loads_obj));
    mp_store_global(MP_QSTR_dumps, MP_OBJ_FROM_PTR(&cbor_dumps_obj));

    MP_DYNRUNTIME_INIT_EXIT
}
