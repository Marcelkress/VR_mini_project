// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/********************************************************************************/ /**
 \file      MetaXR_Audio_Errors.h
 \brief     OVR Audio SDK public header file containing return codes
 ************************************************************************************/

#ifndef OVR_Audio_Errors_h
#define OVR_Audio_Errors_h

#include <stdint.h>

#if defined(__cplusplus) && (__cplusplus >= 201703)
#define OVRA_NO_DISARD_RESULT_TYPE
#endif

/// Enumerates error codes that can be returned by OVRAudio
#ifdef OVRA_NO_DISARD_RESULT_TYPE
typedef enum [[nodiscard]] ovrAudioError : int32_t {
  ovrSuccess = 0,
#else
typedef enum {
#endif
  ovrError_AudioUnknown = 2000, ///< An unknown error has occurred.
  ovrError_AudioInvalidParam = 2001, ///< An invalid parameter, e.g. NULL pointer or out of range variable, was passed
  ovrError_AudioBadSampleRate = 2002, ///< An unsupported sample rate was declared
  ovrError_AudioMissingDLL = 2003, ///< The DLL or shared library could not be found
  ovrError_AudioBadAlignment = 2004, ///< Buffers did not meet 16b alignment requirements
  ovrError_AudioUninitialized = 2005, ///< audio function called before initialization
  ovrError_AudioHRTFInitFailure = 2006, ///< HRTF provider initialization failed
  ovrError_AudioBadVersion = 2007, ///< Mismatched versions between header and libs
  ovrError_AudioSymbolNotFound = 2008, ///< Couldn't find a symbol in the DLL
  ovrError_SharedReverbDisabled = 2009, ///< Late reverberation is disabled
  ovrError_AudioBadAlloc = 2016,
  ovrError_AudioNoAvailableAmbisonicInstance = 2017, /// < Ran out of ambisonic sources
  ovrError_AudioMemoryAllocFailure = 2018, ///< out of memory (fatal)
  ovrError_AudioUnsupportedFeature = 2019, ///< Unsupported feature
  ovrError_AudioInvalidAudioContext = 2020,
  ovrError_AudioBadMesh = 2021,
  ovrError_AudioInternalEnd = 2099, ///< Internal errors used by Audio SDK defined down towards public errors
                                    ///< NOTE: Since we do not define a beginning range for Internal codes, make sure
                                    ///< not to hard-code range checks (since that can vary based on build)
} ovrAudioError;

/// Result type used by the OVRAudio API
#ifdef OVR_RESULT_DEFINED
#error "duplicate ovrResult definition"
#else
#define OVR_RESULT_DEFINED
#ifdef OVRA_NO_DISARD_RESULT_TYPE
typedef enum ovrAudioError ovrResult;
#define OVR_SUCCESS_DEFINED
#else
typedef int32_t ovrResult;

/// Success is zero, while all error types are non-zero values.
#ifndef OVR_SUCCESS_DEFINED
#define OVR_SUCCESS_DEFINED
#define ovrSuccess 0
#endif
#endif
#endif

#endif // OVR_Audio_Errors_h
