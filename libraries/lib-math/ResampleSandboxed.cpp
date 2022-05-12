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

#include "lib_struct_file.h"
rlbox_load_structs_from_library(soxr);

// TODO: sanitize
Resample::Resample(const bool useBestMethod, const double dMinFactor, const double dMaxFactor)
{
   rlbox_sandbox_mylib sandbox;
   sandbox.create_sandbox();

   this->SetMethod(useBestMethod);
   soxr_quality_spec_t q_spec;
   if (dMinFactor == dMaxFactor)
   {
      mbWantConstRateResampling = true; // constant rate resampling
      // q_spec = soxr_quality_spec("\0\1\4\6"[mMethod], 0);
      auto q_spec_tainted = sandbox.invoke_sandbox_function(soxr_quality_spec, "\0\1\4\6"[mMethod], 0);
      q_spec = q_spec_tainted.copy_and_verify(
         [](q_quality_spec_t ret) {
            bool valid_precision = ret.precision >= 0.0 && ret.precision <= 64;
            bool valid_phase_response = ret.phase_response >= 0.0 && ret.phase_response <= 100.0;          
            bool valid_passband_end = ret.passband_end >= 0.0 && ret.passband_end <= 1.0;
            bool valid_stopband_begin = ret.stopband_end > ret.passband_end && ret.stopband_end < 1e3;
            bool valid_e = 0 == strcmp((char *)ret.e, "invalid quality type");
            bool valid_flags = ret.flags <= 128u;
            if (valid_precision && valid_phase_response 
                     && valid_passband_end && valid_stopband_begin
                     && valid_e && valid_stopband_begin && valid_flags) {
               return ret;
            } else {
               printf("ERROR: INVALID q_quality_spec_t CAUGHT\n");
               exit(1);
            }
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
            bool valid_precision = ret.precision >= 0.0 && ret.precision <= 64;
            bool valid_phase_response = ret.phase_response >= 0.0 && ret.phase_response <= 100.0;          
            bool valid_passband_end = ret.passband_end >= 0.0 && ret.passband_end <= 1.0;
            bool valid_stopband_begin = ret.stopband_end > ret.passband_end && ret.stopband_end < 1e3;
            bool valid_e = 0 == strcmp((char *)ret.e, "invalid quality type");
            bool valid_flags = ret.flags <= 128u;
            if (valid_precision && valid_phase_response 
                     && valid_passband_end && valid_stopband_begin
                     && valid_e && valid_stopband_begin && valid_flags) {
               return ret;
            } else {
               printf("ERROR: INVALID q_quality_spec_t CAUGHT\n");
               exit(1);
            }
         }
      );
   }
   mHandle.reset(soxr_create(1, dMinFactor, 1, 0, 0, &q_spec, 0));

   sandbox.destroy_sandbox();
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
   rlbox_sandbox_mylib sandbox;
   sandbox.create_sandbox();

   size_t idone, odone;
   if (mbWantConstRateResampling)
   {
      soxr_process(mHandle.get(),
            inBuffer , (lastFlag? ~inBufferLen : inBufferLen), &idone,
            outBuffer,                           outBufferLen, &odone);
   }
   else
   {
      soxr_set_io_ratio(mHandle.get(), 1/factor, 0);

      inBufferLen = lastFlag? ~inBufferLen : inBufferLen;
      soxr_process(mHandle.get(),
            inBuffer , inBufferLen , &idone,
            outBuffer, outBufferLen, &odone);
   }

   sandbox.destroy_sandbox();
   // TODO verify idone and odone
   return { idone, odone };
}

void Resample::SetMethod(const bool useBestMethod)
{
   if (useBestMethod)
      mMethod = BestMethodSetting.ReadEnum();
   else
      mMethod = FastMethodSetting.ReadEnum();
}
