/**********************************************************************

   Audacity: A Digital Audio Editor
   Audacity(R) is copyright (c) 1999-2012 Audacity Team.
   License: GPL v2 or later.  See License.txt.

   Resample.cpp
   Dominic Mazzoni, Rob Sykes, Vaughan Johnson

******************************************************************//**

\class Resample
\brief Interface to libsoxr.

   This class abstracts the interface to different resampling libraries:

      libsoxr, written by Rob Sykes. LGPL.

   Since Audacity always does resampling on mono streams that are
   contiguous in memory, this class doesn't support multiple channels
   or some of the other optional features of some of these resamplers.

*//*******************************************************************/

#include "Resample.h"
#include "Prefs.h"
#include "Internat.h"
#include "ComponentInterface.h"

#include <soxr.h>
// we added this for strcmp
#include "string.h"

// We're going to use RLBox in a single-threaded environment.
#define RLBOX_SINGLE_THREADED_INVOCATIONS

#include "../../../include/rlbox/rlbox.hpp"
#include "../../../include/rlbox/rlbox_noop_sandbox.hpp"

// All calls into the sandbox are resolved statically.
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
using namespace std;
using namespace rlbox;
//rlbox::rlbox_sandbox<rlbox::rlbox_wasm2c_sandbox> sandbox;

// Define base type for mylib using the noop sandbox
RLBOX_DEFINE_BASE_TYPES_FOR(soxr, noop);

#include <lib_struct_file.h>
rlbox_load_structs_from_library(soxr);

// --------------------------
// validator helper functions
// --------------------------

/**
 * return true if the field members of the soxr_quality_spec_t are valid
 */
bool check_quality_spec(soxr_quality_spec_t const * q_spec) {
   bool valid_precision = q_spec->precision >= 0.0 && q_spec->precision <= 64;
   bool valid_phase_response = q_spec->phase_response >= 0.0 && q_spec->phase_response <= 100.0;          
   bool valid_passband_end = q_spec->passband_end >= 0.0 && q_spec->passband_end <= 1.0;
   bool valid_stopband_begin = q_spec->stopband_end > q_spec->passband_end && q_spec->stopband_end < 1e3;
   bool valid_e = q_spec->e != NULL;
   bool valid_flags = q_spec->flags <= 128u;

   return valid_precision && valid_phase_response 
            && valid_passband_end && valid_stopband_begin
            && valid_e && valid_stopband_begin && valid_flags;
}

/**
 * return true if the field members of the soxr_io_spec_t are valid
 */
bool check_io_spec(soxr_io_spec_t const * io_spec) {
   bool valid_itype = io_spec->itype < 8;
   bool valid_otype = io_spec->otype < 8;
   bool valid_scale = io_spec->scale < 1e3;
   bool valid_e = io_spec->e != NULL;
   bool valid_flags = io_spec->flags == 0 || io_spec->flags == 8u;

   return valid_itype && valid_otype && valid_scale && valid_e && valid_flags;
}

/**
 * return true if the field members of the soxr_runtime_spec_t are valid
 */
bool check_runtime_spec(soxr_runtime_spec_t const * runtime_spec) {
   bool valid_min_dft = runtime_spec->log2_min_dft_size >= 8 && runtime_spec->log2_min_dft_size <= 15;
   bool valid_large_dft = runtime_spec->log2_large_dft_size >= 8 && runtime_spec->log2_large_dft_size <= 20;
   bool valid_coef_size_kbytes = runtime_spec->coef_size_kbytes <= 1000000;
   bool valid_num_threads = runtime_spec->num_threads <= 100;
   bool valid_e = runtime_spec.e != NULL;
   bool valid_flags = runtime_spec->flags <= 3u && runtime_spec->flags != 1u;

   return valid_min_dft && valid_large_dft && valid_coef_size_kbytes
            && valid_num_threads && valid_e && valid_flags;
}

/**
 * return true if the field members of the soxr_t are valid
 */
bool check_soxr_t(soxr_t const * _soxr) {
   bool v_num_channels = _soxr.num_channels <= 100;
   bool v_io_ratio = (_soxr.io_ratio >= 0 && _soxr.io_ratio <= 1) || _soxr.io_ration == -1;
   bool v_error = _soxr.error == 0;
   bool v_quality_spec = check_quality_spec(&_soxr.q_spec);
   bool v_io_spec = check_io_spec(&_soxr.io_spec);
   bool v_runtime_spec = check_runtime_spec(&_soxr.runtime_spec);

   bool v_input_fn_state = _soxr.input_fn_state != NULL;
   bool v_input_fn = _soxr.input_fn < 1000000;
   bool v_max_ilen = _soxr.max_ilen < 1000000 || _soxr.max_ilen == (size_t)-1;

   bool v_resampler_shared = _soxr.shared != NULL;
   bool v_resampler = _soxr.resamplers != NULL && *_soxr.resamplers != NULL;
   bool v_control_block = _soxr.control_block != NULL;
   // bool v_deinterleave = ; // I have no idea how this type is defined
   // bool v_interleave = ; // I have no idea how this type is defined

   bool v_channel_ptrs = _soxr.channel_ptrs != NULL && *_soxr.channel_ptrs != NULL;
   // bool v_clips = _soxr.clips < 1000000; // could be super big anyway?
   // bool v_seed = _soxr.seed != 0; // could be anything?
   bool v_flushing = _soxr.flushing == 0 || _soxr.flushing == 1 // used as a bool

   return v_num_channels && v_io_ratio && v_error
            && v_quality_spec && v_io_spec && v_runtime_spec
            && v_input_fn_state && v_input_fn && v_max_ilen
            && v_resampler_shared && v_resampler && v_control_block
            && v_channel_ptrs && v_flushing;
}

bool check_idone_odone(size_t idone, size_t ilen, size_t odone, size_t olen) {
   return *idone <= ilen && *odone <= olen;
}



Resample::Resample(const bool useBestMethod, const double dMinFactor, const double dMaxFactor)
{
   rlbox_sandbox_soxr sandbox;
   sandbox.create_sandbox();

   this->SetMethod(useBestMethod);
   soxr_quality_spec_t q_spec;
   if (dMinFactor == dMaxFactor)
   {
      mbWantConstRateResampling = true; // constant rate resampling
      // q_spec = soxr_quality_spec("\0\1\4\6"[mMethod], 0);
      auto q_spec_tainted = sandbox.invoke_sandbox_function(soxr_quality_spec, "\0\1\4\6"[mMethod], 0);
      q_spec = q_spec_tainted.copy_and_verify(
         [](soxr_quality_spec_t ret) {
            if (check_quality_spec(&ret)) {
               return ret;
            }
            printf("ERROR: INVALID soxr_quality_spec_t CAUGHT\n");
            exit(1);
         }
      );
   }
   else
   {
      mbWantConstRateResampling = false; // variable rate resampling
      // q_spec = soxr_quality_spec(SOXR_HQ, SOXR_VR);
      auto q_spec_tainted = sandbox.invoke_sandbox_function(soxr_quality_spec, SOXR_HQ, SOXR_VR);
      q_spec = q_spec_tainted.copy_and_verify(
         [](soxr_quality_spec_t ret) {
            if (check_quality_spec(&ret)) {
               return ret;
            }
            printf("ERROR: INVALID soxr_quality_spec_t CAUGHT\n");
            exit(1);
         }
      );
   }
   // mHandle.reset(soxr_create(1, dMinFactor, 1, 0, 0, &q_spec, 0));
   auto soxr_tainted = sandbox.invoke_sandbox_function(soxr_create, 1, dMinFactor, 1, 0, 0, &q_spec, 0);
   soxr_t _soxr = soxr_tainted.copy_and_verify(
      [](soxr_t ret) {
         if (!ret) {
            printf("ERROR: soxr_create FAILED\n");
            exit(1);
         }
         if (check_soxr_t(&ret)) {
            return ret;
         }
         printf("ERROR: INVALID soxr_t CAUGHT\n");
         exit(1);
      }
   );

   sandbox.destroy_sandbox();
   mHandle.reset(_soxr);
}

Resample::~Resample()
{
}

//////////
static const std::initializer_list<EnumValueSymbol> methodNames{
   { wxT("LowQuality"), XO("Low Quality (Fastest)") },
   { wxT("MediumQuality"), XO("Medium Quality") },
   { wxT("HighQuality"), XO("High Quality") },
   { wxT("BestQuality"), XO("Best Quality (Slowest)") }
};

static auto intChoicesMethod = {
   0, 1, 2, 3
};

EnumSetting< int > Resample::FastMethodSetting{
   wxT("/Quality/LibsoxrSampleRateConverterChoice"),
   methodNames,
   1,  // Medium Quality

   // for migrating old preferences:
   intChoicesMethod,
   wxT("/Quality/LibsoxrSampleRateConverter")
};

EnumSetting< int > Resample::BestMethodSetting
{
   wxT("/Quality/LibsoxrHQSampleRateConverterChoice"),
   methodNames,
   3, // Best Quality,

   // for migrating old preferences:
   intChoicesMethod,
   wxT("/Quality/LibsoxrHQSampleRateConverter")
};


// TODO: sanitize
//////////
std::pair<size_t, size_t>
      Resample::Process(double  factor,
                        float  *inBuffer,
                        size_t  inBufferLen,
                        bool    lastFlag,
                        float  *outBuffer,
                        size_t  outBufferLen)
{
   rlbox_sandbox_soxr sandbox;
   sandbox.create_sandbox();

   size_t idone, odone;
   if (mbWantConstRateResampling)
   {
      // soxr_process(mHandle.get(),
      //       inBuffer , (lastFlag? ~inBufferLen : inBufferLen), &idone,
      //       outBuffer,                           outBufferLen, &odone);
      inBufferLen = lastFlag? ~inBufferLen : inBufferLen;
      auto error_tainted = sandbox.invoke_sandbox_function(soxr_process, mHandle.get(),
                                 inBuffer , inBufferLen , &idone,
                                 outBuffer, outBufferLen, &odone);

      soxr_error_t error = error_tainted.unverified_safe_because("A const char *; either 0 or set as string literal.");
      if (error) {
         printf("ERROR: soxr_process FAILED. %s\n", error);
         exit(1);
      }
      if (! check_soxr_t(mHandle.get())) {
         printf("ERROR: INVALID mHandle CAUGHT\n");
         exit(1);
      }
      if (! check_idone_odone(idone, inBufferLen, odone, outBufferLen)) {
         printf("ERROR: INVALID idone OR odone CAUGHT\n");
         exit(1);
      }
   }
   else
   {
      // soxr_set_io_ratio(mHandle.get(), 1/factor, 0);
      auto error_tainted = sandbox.invoke_sandbox_function(soxr_set_io_ratio, mHandle.get(),
                                 1/factor, 0);
      
      soxr_error_t error = error_tainted.unverified_safe_because("A const char *; either 0 or set as string literal.");
      if (error) {
         printf("ERROR: soxr_set_io_ratio FAILED. %s\n", error);
         exit(1);
      }
      if (! check_soxr_t(mHandle.get())) {
         printf("ERROR: INVALID mHandle CAUGHT\n");
         exit(1);
      }

      inBufferLen = lastFlag? ~inBufferLen : inBufferLen;
      // soxr_process(mHandle.get(),
      //       inBuffer , inBufferLen , &idone,
      //       outBuffer, outBufferLen, &odone);
      auto error_tainted = sandbox.invoke_sandbox_function(soxr_process, mHandle.get(),
                                 inBuffer , inBufferLen , &idone,
                                 outBuffer, outBufferLen, &odone);
      soxr_error_t error = error_tainted.unverified_safe_because("A const char *; either 0 or set as string literal.");
      if (error) {
         printf("ERROR: soxr_process FAILED. %s\n", error);
         exit(1);
      }
      if (! check_soxr_t(mHandle.get())) {
         printf("ERROR: INVALID mHandle CAUGHT\n");
         exit(1);
      }
      if (! check_idone_odone(idone, inBufferLen, odone, outBufferLen)) {
         printf("ERROR: INVALID idone OR odone CAUGHT\n");
         exit(1);
      }
   }

   sandbox.destroy_sandbox();
   return { idone, odone };
}

void Resample::SetMethod(const bool useBestMethod)
{
   if (useBestMethod)
      mMethod = BestMethodSetting.ReadEnum();
   else
      mMethod = FastMethodSetting.ReadEnum();
}
