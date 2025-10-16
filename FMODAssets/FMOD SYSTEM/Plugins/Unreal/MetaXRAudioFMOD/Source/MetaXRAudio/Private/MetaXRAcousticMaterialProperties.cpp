// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAcousticMaterialProperties.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

void UMetaXRAcousticMaterialProperties::AppendHash(FString& hash) {
  FString materialHash;

  for (const FMetaXRAudioPoint& Point : Data.Absorption.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  for (const FMetaXRAudioPoint& Point : Data.Transmission.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  for (const FMetaXRAudioPoint& Point : Data.Scattering.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  // Unreal FMD5Hash only accepts string, so convert to string first
  hash.Append(FMD5::HashAnsiString(*materialHash));
}

void UMetaXRAcousticMaterialProperties::ConstructMaterial(ovrAudioMaterial ovrMaterial) {
  const FMetaXRAcousticMaterialData& MaterialData = Data;

  if (MaterialData.IsEmpty() || !ovrMaterial) {
    return;
  }

  for (const FMetaXRAudioPoint& Point : MaterialData.Absorption.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material absorption at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }

  for (const FMetaXRAudioPoint& Point : MaterialData.Transmission.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material transmission at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }

  for (const FMetaXRAudioPoint& Point : MaterialData.Scattering.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material scattering at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }
}

void FMetaXRAcousticMaterialData::ApplyPreset(EMetaXRAudioMaterialPreset Preset) {
  switch (Preset) {
    case EMetaXRAudioMaterialPreset::AcousticTile:
      Absorption.Points = {{125.0f, 0.50f}, {250.0f, 0.70f}, {500.0f, 0.60f}, {1000.0f, 0.70f}, {2000.0f, 0.70f}, {4000.0f, 0.50f}};
      Scattering.Points = {{125.0f, 0.10f}, {250.f, 0.15f}, {500.f, 0.20f}, {1000.f, 0.20f}, {2000.f, 0.25f}, {4000.f, 0.30f}};

      Transmission.Points = {{125.0f, 0.05f}, {250.0f, 0.04f}, {500.f, 0.03f}, {1000.f, 0.02f}, {2000.f, 0.005f}, {4000.f, 0.002f}};
      break;
    case EMetaXRAudioMaterialPreset::Brick:
      Absorption.Points = {{125.f, 0.02f}, {250.f, 0.02f}, {500.f, 0.03f}, {1000.f, 0.04f}, {2000.f, 0.05f}, {4000.f, 0.07f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.25f}, {500.f, 0.30f}, {1000.f, 0.35f}, {2000.f, 0.40f}, {4000.f, 0.45f}};

      Transmission.Points = {{125.f, 0.025f}, {250.f, 0.019f}, {500.f, 0.01f}, {1000.f, 0.0045f}, {2000.f, 0.0018f}, {4000.f, 0.00089f}};
      break;
    case EMetaXRAudioMaterialPreset::BrickPainted:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.02f}, {1000.f, 0.02f}, {2000.f, 0.02f}, {4000.f, 0.03f}};

      Scattering.Points = {{125.f, 0.15f}, {250.f, 0.15f}, {500.f, 0.20f}, {1000.f, 0.20f}, {2000.f, 0.20f}, {4000.f, 0.25f}};

      Transmission.Points = {{125.f, 0.025f}, {250.f, 0.019f}, {500.f, 0.01f}, {1000.f, 0.0045f}, {2000.f, 0.0018f}, {4000.f, 0.00089f}};
      break;
    case EMetaXRAudioMaterialPreset::Cardboard:
      Absorption.Points = {
          {400.f, 0.41f},
          {500.f, 0.607f},
          {630.f, 0.773f},
          {800.f, 0.669f},
          {1000.f, 0.685f},
          {1250.f, 0.806f},
          {1600.f, 0.579f},
          {2000.f, 0.792f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.12f}, {500.f, 0.14f}, {1000.f, 0.16f}, {2000.f, 0.18f}, {4000.f, 0.20f}};

      Transmission.Points = {
          {400.f, 0.082f},
          {500.f, 0.121f},
          {630.f, 0.155f},
          {800.f, 0.134f},
          {1000.f, 0.137f},
          {1250.f, 0.161f},
          {1600.f, 0.116f},
          {2000.f, 0.158f}};
      break;
    case EMetaXRAudioMaterialPreset::Carpet:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.05f}, {500.f, 0.10f}, {1000.f, 0.20f}, {2000.f, 0.45f}, {4000.f, 0.65f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.15f}, {1000.f, 0.20f}, {2000.f, 0.30f}, {4000.f, 0.45f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::CarpetHeavy:
      Absorption.Points = {{125.f, 0.02f}, {250.f, 0.06f}, {500.f, 0.14f}, {1000.f, 0.37f}, {2000.f, 0.48f}, {4000.f, 0.63f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.15f}, {500.f, 0.20f}, {1000.f, 0.25f}, {2000.f, 0.35f}, {4000.f, 0.50f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::CarpetHeavyPadded:
      Absorption.Points = {{125.f, 0.08f}, {250.f, 0.24f}, {500.f, 0.57f}, {1000.f, 0.69f}, {2000.f, 0.71f}, {4000.f, 0.73f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.15f}, {500.f, 0.20f}, {1000.f, 0.25f}, {2000.f, 0.35f}, {4000.f, 0.50f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::CeramicTile:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.01f}, {1000.f, 0.01f}, {2000.f, 0.02f}, {4000.f, 0.02f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.12f}, {500.f, 0.14f}, {1000.f, 0.16f}, {2000.f, 0.18f}, {4000.f, 0.20f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::Concrete:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.02f}, {1000.f, 0.02f}, {2000.f, 0.02f}, {4000.f, 0.02f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.11f}, {500.f, 0.12f}, {1000.f, 0.13f}, {2000.f, 0.14f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::ConcreteRough:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.02f}, {500.f, 0.04f}, {1000.f, 0.06f}, {2000.f, 0.08f}, {4000.f, 0.10f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.12f}, {500.f, 0.15f}, {1000.f, 0.20f}, {2000.f, 0.25f}, {4000.f, 0.30f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::ConcreteBlock:
      Absorption.Points = {{125.f, 0.36f}, {250.f, 0.44f}, {500.f, 0.31f}, {1000.f, 0.29f}, {2000.f, 0.39f}, {4000.f, 0.21f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.12f}, {500.f, 0.15f}, {1000.f, 0.20f}, {2000.f, 0.30f}, {4000.f, 0.40f}};

      Transmission.Points = {{125.f, 0.02f}, {250.f, 0.01f}, {500.f, 0.0063f}, {1000.f, 0.0035f}, {2000.f, 0.00011f}, {4000.f, 0.00063f}};
      break;
    case EMetaXRAudioMaterialPreset::ConcreteBlockPainted:
      Absorption.Points = {{125.f, 0.10f}, {250.f, 0.05f}, {500.f, 0.06f}, {1000.f, 0.07f}, {2000.f, 0.09f}, {4000.f, 0.08f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.11f}, {500.f, 0.13f}, {1000.f, 0.15f}, {2000.f, 0.16f}, {4000.f, 0.20f}};

      Transmission.Points = {{125.f, 0.02f}, {250.f, 0.01f}, {500.f, 0.0063f}, {1000.f, 0.0035f}, {2000.f, 0.00011f}, {4000.f, 0.00063f}};
      break;
    case EMetaXRAudioMaterialPreset::Curtain:
      Absorption.Points = {{125.f, 0.07f}, {250.f, 0.31f}, {500.f, 0.49f}, {1000.f, 0.75f}, {2000.f, 0.70f}, {4000.f, 0.60f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.15f}, {500.f, 0.2f}, {1000.f, 0.3f}, {2000.f, 0.4f}, {4000.f, 0.5f}};

      Transmission.Points = {{125.f, 0.42f}, {250.f, 0.39f}, {500.f, 0.21f}, {1000.f, 0.14f}, {2000.f, 0.079f}, {4000.f, 0.045f}};
      break;
    case EMetaXRAudioMaterialPreset::Foliage:
      Absorption.Points = {{125.f, 0.03f}, {250.f, 0.06f}, {500.f, 0.11f}, {1000.f, 0.17f}, {2000.f, 0.27f}, {4000.f, 0.31f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.3f}, {500.f, 0.4f}, {1000.f, 0.5f}, {2000.f, 0.7f}, {4000.f, 0.8f}};

      Transmission.Points = {{125.f, 0.9f}, {250.f, 0.9f}, {500.f, 0.9f}, {1000.f, 0.8f}, {2000.f, 0.5f}, {4000.f, 0.3f}};
      break;
    case EMetaXRAudioMaterialPreset::Glass:
      Absorption.Points = {{125.f, 0.35f}, {250.f, 0.25f}, {500.f, 0.18f}, {1000.f, 0.12f}, {2000.f, 0.07f}, {4000.f, 0.05f}};

      Scattering.Points = {{125.f, 0.05f}, {250.f, 0.05f}, {500.f, 0.05f}, {1000.f, 0.05f}, {2000.f, 0.05f}, {4000.f, 0.05f}};

      Transmission.Points = {{125.f, 0.125f}, {250.f, 0.089f}, {500.f, 0.05f}, {1000.f, 0.028f}, {2000.f, 0.022f}, {4000.f, 0.079f}};
      break;
    case EMetaXRAudioMaterialPreset::GlassHeavy:
      Absorption.Points = {{125.f, 0.18f}, {250.f, 0.06f}, {500.f, 0.04f}, {1000.f, 0.03f}, {2000.f, 0.02f}, {4000.f, 0.02f}};

      Scattering.Points = {{125.f, 0.05f}, {250.f, 0.05f}, {500.f, 0.05f}, {1000.f, 0.05f}, {2000.f, 0.05f}, {4000.f, 0.05f}};

      Transmission.Points = {{125.f, 0.056f}, {250.f, 0.039f}, {500.f, 0.028f}, {1000.f, 0.02f}, {2000.f, 0.032f}, {4000.f, 0.014f}};
      break;
    case EMetaXRAudioMaterialPreset::Grass:
      Absorption.Points = {{125.f, 0.11f}, {250.f, 0.26f}, {500.f, 0.60f}, {1000.f, 0.69f}, {2000.f, 0.92f}, {4000.f, 0.99f}};

      Scattering.Points = {{125.f, 0.30f}, {250.f, 0.30f}, {500.f, 0.40f}, {1000.f, 0.50f}, {2000.f, 0.60f}, {4000.f, 0.70f}};

      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::Gravel:
      Absorption.Points = {{125.f, 0.25f}, {250.f, 0.60f}, {500.f, 0.65f}, {1000.f, 0.70f}, {2000.f, 0.75f}, {4000.f, 0.80f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.30f}, {500.f, 0.40f}, {1000.f, 0.50f}, {2000.f, 0.60f}, {4000.f, 0.70f}};

      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::GypsumBoard:
      Absorption.Points = {{125.f, 0.29f}, {250.f, 0.10f}, {500.f, 0.05f}, {1000.f, 0.04f}, {2000.f, 0.07f}, {4000.f, 0.09f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.11f}, {500.f, 0.12f}, {1000.f, 0.13f}, {2000.f, 0.14f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.035f}, {250.f, 0.0125f}, {500.f, 0.0056f}, {1000.f, 0.0025f}, {2000.f, 0.0013f}, {4000.f, 0.0032f}};
      break;
    case EMetaXRAudioMaterialPreset::Marble:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.01f}, {1000.f, 0.01f}, {2000.f, 0.02f}, {4000.f, 0.02f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.10f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::Mud:
      Absorption.Points = {{125.f, 0.15f}, {250.f, 0.25f}, {500.f, 0.30f}, {1000.f, 0.25f}, {2000.f, 0.20f}, {4000.f, 0.15f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.20f}, {500.f, 0.25f}, {1000.f, 0.40f}, {2000.f, 0.55f}, {4000.f, 0.70f}};

      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::PlasterOnBrick:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.02f}, {500.f, 0.02f}, {1000.f, 0.03f}, {2000.f, 0.04f}, {4000.f, 0.05f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.25f}, {500.f, 0.30f}, {1000.f, 0.35f}, {2000.f, 0.40f}, {4000.f, 0.45f}};

      Transmission.Points = {{125.f, 0.025f}, {250.f, 0.019f}, {500.f, 0.01f}, {1000.f, 0.0045f}, {2000.f, 0.0018f}, {4000.f, 0.00089f}};
      break;
    case EMetaXRAudioMaterialPreset::PlasterOnConcreteBlock:
      Absorption.Points = {{125.f, 0.12f}, {250.f, 0.09f}, {500.f, 0.07f}, {1000.f, 0.05f}, {2000.f, 0.05f}, {4000.f, 0.04f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.25f}, {500.f, 0.30f}, {1000.f, 0.35f}, {2000.f, 0.40f}, {4000.f, 0.45f}};

      Transmission.Points = {{125.f, 0.02f}, {250.f, 0.01f}, {500.f, 0.0063f}, {1000.f, 0.0035f}, {2000.f, 0.00011f}, {4000.f, 0.00063f}};
      break;
    case EMetaXRAudioMaterialPreset::Rubber:
      Absorption.Points = {{125.f, 0.05f}, {250.f, 0.05f}, {500.f, 0.1f}, {1000.f, 0.1f}, {2000.f, 0.05f}, {4000.f, 0.05f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.15f}, {4000.f, 0.20f}};

      Transmission.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.02f}, {1000.f, 0.02f}, {2000.f, 0.01f}, {4000.f, 0.01f}};
      break;
    case EMetaXRAudioMaterialPreset::Soil:
      Absorption.Points = {{125.f, 0.15f}, {250.f, 0.25f}, {500.f, 0.40f}, {1000.f, 0.55f}, {2000.f, 0.60f}, {4000.f, 0.60f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.20f}, {500.f, 0.25f}, {1000.f, 0.40f}, {2000.f, 0.55f}, {4000.f, 0.70f}};

      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::SoundProof:
      Absorption.Points = {{1000.f, 1.0f}};
      Scattering.Points = {{1000.f, 0.0f}};
      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::Snow:
      Absorption.Points = {{125.f, 0.45f}, {250.f, 0.75f}, {500.f, 0.90f}, {1000.f, 0.95f}, {2000.f, 0.95f}, {4000.f, 0.95f}};

      Scattering.Points = {{125.f, 0.20f}, {250.f, 0.30f}, {500.f, 0.40f}, {1000.f, 0.50f}, {2000.f, 0.60f}, {4000.f, 0.75f}};

      Transmission.Points = {};
      break;
    case EMetaXRAudioMaterialPreset::Steel:
      Absorption.Points = {{125.f, 0.05f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.07f}, {4000.f, 0.02f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.10f}};

      Transmission.Points = {{125.f, 0.25f}, {250.f, 0.2f}, {500.f, 0.17f}, {1000.f, 0.089f}, {2000.f, 0.089f}, {4000.f, 0.0056f}};
      break;
    case EMetaXRAudioMaterialPreset::Stone:
      Absorption.Points = {{125.f, 0.02f}, {500.f, 0.02f}, {2000.f, 0.05f}, {4000.f, 0.05f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.15f}, {1000.f, 0.20f}, {2000.f, 0.25f}, {4000.f, 0.30f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.00016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::Vent:
      Absorption.Points = {
          {63.5f, 0.15f},
          {125.f, 0.15f},
          {250.f, 0.20f},
          {500.f, 0.50f},
          {1000.f, 0.35f},
          {2000.f, 0.30f},
          {4000.f, 0.20f},
          {8000.f, 0.20f}};

      Scattering.Points = {
          {63.5f, 0.10f},
          {125.f, 0.10f},
          {250.f, 0.10f},
          {500.f, 0.15f},
          {1000.f, 0.30f},
          {2000.f, 0.40f},
          {4000.f, 0.50f},
          {8000.f, 0.50f}};

      Transmission.Points = {
          {63.5f, 0.135f},
          {125.f, 0.135f},
          {250.f, 0.18f},
          {500.f, 0.45f},
          {1000.f, 0.315f},
          {2000.f, 0.27f},
          {4000.f, 0.18f},
          {8000.f, 0.18f}};
      break;
    case EMetaXRAudioMaterialPreset::Water:
      Absorption.Points = {{125.f, 0.01f}, {250.f, 0.01f}, {500.f, 0.01f}, {1000.f, 0.02f}, {2000.f, 0.02f}, {4000.f, 0.03f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.07f}, {2000.f, 0.05f}, {4000.f, 0.05f}};

      Transmission.Points = {{125.f, 0.03f}, {250.f, 0.03f}, {500.f, 0.03f}, {1000.f, 0.02f}, {2000.f, 0.015f}, {4000.f, 0.01f}};
      break;
    case EMetaXRAudioMaterialPreset::WoodThin:
      Absorption.Points = {{125.f, 0.42f}, {250.f, 0.21f}, {500.f, 0.10f}, {1000.f, 0.08f}, {2000.f, 0.06f}, {4000.f, 0.06f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.2f}, {250.f, 0.125f}, {500.f, 0.079f}, {1000.f, 0.1f}, {2000.f, 0.089f}, {4000.f, 0.05f}};
      break;
    case EMetaXRAudioMaterialPreset::WoodThick:
      Absorption.Points = {{125.f, 0.19f}, {250.f, 0.14f}, {500.f, 0.09f}, {1000.f, 0.06f}, {2000.f, 0.06f}, {4000.f, 0.05f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.035f}, {250.f, 0.028f}, {500.f, 0.028f}, {1000.f, 0.028f}, {2000.f, 0.011f}, {4000.f, 0.0071f}};
      break;
    case EMetaXRAudioMaterialPreset::WoodFloor:
      Absorption.Points = {{125.f, 0.15f}, {250.f, 0.11f}, {500.f, 0.10f}, {1000.f, 0.07f}, {2000.f, 0.06f}, {4000.f, 0.07f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.071f}, {250.f, 0.025f}, {500.f, 0.0158f}, {1000.f, 0.0056f}, {2000.f, 0.0035f}, {4000.f, 0.0016f}};
      break;
    case EMetaXRAudioMaterialPreset::WoodOnConcrete:
      Absorption.Points = {{125.f, 0.04f}, {250.f, 0.04f}, {500.f, 0.07f}, {1000.f, 0.06f}, {2000.f, 0.06f}, {4000.f, 0.07f}};

      Scattering.Points = {{125.f, 0.10f}, {250.f, 0.10f}, {500.f, 0.10f}, {1000.f, 0.10f}, {2000.f, 0.10f}, {4000.f, 0.15f}};

      Transmission.Points = {{125.f, 0.004f}, {250.f, 0.0079f}, {500.f, 0.0056f}, {1000.f, 0.0016f}, {2000.f, 0.0014f}, {4000.f, 0.0005f}};
      break;
    case EMetaXRAudioMaterialPreset::MetaDefault:
      Absorption.Points = {{1000.0f, 0.1f}};

      Scattering.Points = {{1000.0f, 0.5f}};

      Transmission.Points = {{1000.0f, 0.0f}};
      break;
  }
}
