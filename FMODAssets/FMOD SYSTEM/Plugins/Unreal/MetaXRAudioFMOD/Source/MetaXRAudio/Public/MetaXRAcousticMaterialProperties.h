// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetaXRAudioRoomAcousticProperties.h"
#include "MetaXRAudioSpectrum.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticMaterialProperties.generated.h"

#define META_XR_AUDIO_MIN_ABSOPRTION 0.0f
#define META_XR_AUDIO_MAX_ABSOPRTION 1.0f
#define META_XR_AUDIO_MIN_TRANSMISSION 0.0f
#define META_XR_AUDIO_MAX_TRANSMISSION 1.0f
#define META_XR_AUDIO_MIN_SCATTERING 0.0f
#define META_XR_AUDIO_MAX_SCATTERING 1.0f
#define META_XR_AUDIO_DEFAULT_MATERIAL EMetaXRAudioMaterialPreset::MetaDefault

USTRUCT(BlueprintType)
struct FMetaXRAcousticMaterialData {
  GENERATED_BODY()

  UPROPERTY()
  FMetaXRAudioSpectrum Absorption;

  UPROPERTY()
  FMetaXRAudioSpectrum Transmission;

  UPROPERTY()
  FMetaXRAudioSpectrum Scattering;

  void Clone(const FMetaXRAcousticMaterialData& Other) {
    Absorption.Clone(Other.Absorption);
    Transmission.Clone(Other.Transmission);
    Scattering.Clone(Other.Scattering);
  }

  bool IsEmpty() const {
    return Absorption.Points.Num() == 0 && Transmission.Points.Num() == 0 && Scattering.Points.Num() == 0;
  }

  void ApplyPreset(EMetaXRAudioMaterialPreset Preset);
};

UCLASS(BlueprintType)
class METAXRAUDIO_API UMetaXRAcousticMaterialProperties : public UDataAsset {
  GENERATED_BODY()

 public:
  UMetaXRAcousticMaterialProperties() {
    ApplyPreset(Preset);
  }

  UPROPERTY()
  FMetaXRAcousticMaterialData Data;

  UPROPERTY()
  EMetaXRAudioMaterialPreset Preset = META_XR_AUDIO_DEFAULT_MATERIAL;

  // The color used to visualize an Acoustic Geometry set to use this material
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Meta XR Audio")
  FLinearColor Color = FLinearColor::Yellow;

  void AppendHash(FString& hash);

  void ConstructMaterial(ovrAudioMaterial Material);

  void ApplyPreset(EMetaXRAudioMaterialPreset NewPreset) {
    Data.ApplyPreset(NewPreset);
    Preset = NewPreset;
  }
};
