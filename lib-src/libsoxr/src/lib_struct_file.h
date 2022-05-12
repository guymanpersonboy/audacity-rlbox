/**********************************************************************

   Used to tell RLBox the layout of the structs in soxr.h

**********************************************************************/

#ifndef _LIB_STRUCT_FILE_H
#define _LIB_STRUCT_FILE_H

#include "stddef.h"
#include "soxr.h"
// #include "soxr.c"

#define sandbox_fields_reflection_soxr_class_soxr_quality_spec(f, g, ...) \
    f(double, precision, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(double, phase_response, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(double, passband_end, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(double, stopband_begin, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void *, e, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned long, flags, FIELD_NORMAL, ##__VA_ARGS__) g() \

#define sandbox_fields_reflection_soxr_class_soxr_io_spec(f, g, ...) \
    f(soxr_datatype_t, itype, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_datatype_t, otype, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(double, scale, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void *, e, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned long, flags, FIELD_NORMAL, ##__VA_ARGS__) g() \

#define sandbox_fields_reflection_soxr_class_soxr_runtime_spec(f, g, ...) \
    f(unsigned, log2_min_dft_size, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned, log2_large_dft_size, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned, coef_size_kbytes, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned, num_threads, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void *, e, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned long, flags, FIELD_NORMAL, ##__VA_ARGS__) g() \

#define sandbox_fields_reflection_soxr_class_soxr(f, g, ...) \
    f(unsigned, num_channels, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(double, io_ratio, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_error_t, error, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_quality_spec_t, q_spec, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_io_spec_t, io_spec, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_runtime_spec_t, runtime_spec, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void *, input_fn_state, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(soxr_input_fn_t, input_fn, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(size_t, max_ilen, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(resampler_shared_t, shared, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(resampler_t *, resamplers, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(control_block_t, control_block, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(deinterleave_t, deinterleave, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(interleave_t, interleave, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void * *, channel_ptrs, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(size_t, clips, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(void *, e, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(unsigned long, seed, FIELD_NORMAL, ##__VA_ARGS__) g() \
    f(int, flushing, FIELD_NORMAL, ##__VA_ARGS__) g() \

#define sandbox_fields_reflection_soxr_allClasses(f, ...)  \
    f(soxr_quality_spec, soxr, ##__VA_ARGS__) \
    f(soxr_io_spec, soxr, ##__VA_ARGS__) \
    f(soxr_runtime_spec, soxr, ##__VA_ARGS__)

#endif // _LIB_STRUCT_FILE_H
