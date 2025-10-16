#pragma once

#include "fmod.hpp"

extern "C" {
// Return a list of descriptions for all the nested plugins at once
F_EXPORT FMOD_PLUGINLIST* F_CALL FMODGetPluginDescriptionList();
}

/*===============================================================================================
 MetaXRAudio_FMODSource
===============================================================================================*/

// Source Parameters
enum {
  META_PARAM_INDEX_3D_ATTRIBUTES, // FMOD_DSP_PARAMETER_3DATTRIBUTES - both absolute and relative to camera
  META_PARAM_INDEX_ATTENUATION_RANGE, // FMOD_DSP_PARAMETER_ATTENUATION_RANGE
  META_PARAM_INDEX_OVERALLGAIN, // FMOD_DSP_PARAMETER_OVERALLGAIN - used for prioritization

  META_PARAM_INDEX_ATTENUATION_MIN_DISTANCE, // float
  META_PARAM_INDEX_ATTENUATION_MAX_DISTANCE, // float
  META_PARAM_INDEX_ATTENUATION_MODE, // int
  META_PARAM_INDEX_ACOUSTICS_ENABLED, // bool
  META_PARAM_INDEX_REVERB_SEND_LEVEL_DB, // float
  META_PARAM_INDEX_REFLECTIONS_SEND_LEVEL_DB, // float
  META_PARAM_INDEX_VOLUMETRIC_RADIUS, // float
  META_PARAM_INDEX_HRTF_INTENSITY, // float
  META_PARAM_INDEX_DIRECTIVITY_PATTERN, // enum
  META_PARAM_INDEX_DIRECTIVITY_INTENSITY, // float
  META_PARAM_INDEX_DIRECT_ENABLED, // bool
  META_PARAM_INDEX_REVERB_REACH, // float
  META_PARAM_INDEX_OCCLUSION_INTENSITY, // float
  META_PARAM_INDEX_MEDIUM_ABSORPTION, // bool

  META_PARAM_INDEX_META_SOURCE_NUM_PARAMETERS
};

// Source Function Definitions
FMOD_RESULT F_CALL FMOD_META_Source_dspcreate(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Source_dsprelease(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Source_dspreset(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Source_dspprocess(
    FMOD_DSP_STATE* dsp,
    unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* inbufferarray,
    FMOD_DSP_BUFFER_ARRAY* outbufferarray,
    FMOD_BOOL inputsidle,
    FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALL FMOD_META_Source_dspsetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float value);
FMOD_RESULT F_CALL FMOD_META_Source_dspsetparamint(FMOD_DSP_STATE* dsp_state, int index, int value);
FMOD_RESULT F_CALL FMOD_META_Source_dspsetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALL FMOD_META_Source_dspsetparamdata(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length);
FMOD_RESULT F_CALL FMOD_META_Source_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_META_Source_dspgetparamint(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_META_Source_dspgetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT F_CALL
FMOD_META_Source_dspgetparamdata(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);

extern "C" {
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMOD_META_Source_GetDSPDescription();
}

/*===============================================================================================
 MetaXRAudio_FMODAmbisonic
===============================================================================================*/

// Ambisonics Parameters
enum {
  META_PARAM_INDEX_AMBISONIC_SOUND_POSITION, // FMOD_DSP_PARAMETER_3DATTRIBUTES - both absolute and relative to camera
  META_PARAM_INDEX_AMBISONIC_OVERALL_GAIN, // FMOD_DSP_PARAMETER_OVERALLGAIN - used for prioritization

  META_PARAM_INDEX_META_AMBISONIC_NUM_PARAMETERS
};

// Ambisonic Function Definitions
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspcreate(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dsprelease(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspreset(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspprocess(
    FMOD_DSP_STATE* dsp,
    unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* inbufferarray,
    FMOD_DSP_BUFFER_ARRAY* outbufferarray,
    FMOD_BOOL inputsidle,
    FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspsetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float value);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspsetparamint(FMOD_DSP_STATE* dsp_state, int index, int value);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspsetparamdata(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspgetparamint(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_META_Ambisonic_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr);
FMOD_RESULT F_CALL
FMOD_META_Ambisonic_dspgetparamdata(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);

extern "C" {
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMOD_META_Ambisonic_GetDSPDescription();
}

/*===============================================================================================
 MetaXRAudio_FMODReflections
===============================================================================================*/

// Reflections Parameters
enum {
  META_PARAM_INDEX_REFLECTIONS_EARLY_REFLECTIONS_ENABLED, // bool
  META_PARAM_INDEX_REFLECTIONS_REVERB_ENABLED, // bool
  META_PARAM_INDEX_REFLECTIONS_REVERB_OVERALL_GAIN, // FMOD_DSP_PARAMETER_OVERALLGAIN - used for prioritization
  META_PARAM_INDEX_REFLECTIONS_REVERB_WET_LEVEL_DB, // float
  META_PARAM_INDEX_REFLECTIONS_VOICE_LIMIT, // int

  META_PARAM_INDEX_REFLECTIONS_NUM_PARAMETERS
};

// Reflections Function Definitions
FMOD_RESULT F_CALL FMOD_META_Reflections_dspcreate(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Reflections_dsprelease(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspprocess(
    FMOD_DSP_STATE* dsp,
    unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* inbufferarray,
    FMOD_DSP_BUFFER_ARRAY* outbufferarray,
    FMOD_BOOL inputsidle,
    FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspsetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float value);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspsetparamint(FMOD_DSP_STATE* dsp_state, int index, int value);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspsetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspsetparamdata(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspgetparamint(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_META_Reflections_dspgetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT F_CALL
FMOD_META_Reflections_dspgetparamdata(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);

extern "C" {
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMOD_META_Reflections_GetDSPDescription();
}
