// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetaXR_Audio.h"

#include "MetaXRAcousticProjectSettings.generated.h"

UENUM()
enum class EMetaXRAudioAcousticModel : int8 {
  Automatic = ovrAudioAcousticModel_Automatic,
  None = ovrAudioAcousticModel_None,
  ShoeboxRoom = ovrAudioAcousticModel_StaticRoom,
  AcousticRayTracing = ovrAudioAcousticModel_PropagationSystem
};

#define META_XR_AUDIO_DEFAULT_SAVE_FOLDER "MetaXRAcoustics"

// This struct is used to link on Physical Material to one Meta Acoustic Material
// It is more optimal to use a TMap, however Unreal does not sufficiently support using them as UPROPERTYs
USTRUCT()
struct FMaterialLinkage {
  GENERATED_BODY()

  UPROPERTY(
      EditDefaultsOnly,
      Category = "AcousticsSettings",
      meta = (AllowedClasses = "/Script/MetaXRAudio.MetaXRAcousticMaterialProperties"))
  FSoftObjectPath AcousticMaterial;

  UPROPERTY(EditDefaultsOnly, Category = "AcousticsSettings", meta = (AllowedClasses = "/Script/PhysicsCore.PhysicalMaterial"))
  FSoftObjectPath PhysicalMaterial;
};

UCLASS(config = Game, defaultconfig, BlueprintType)
class METAXRAUDIO_API UMetaXRAcousticProjectSettings : public UObject {
  GENERATED_BODY()

 public:
  UMetaXRAcousticProjectSettings();

#if WITH_EDITOR
  void BulkBakeAcousticMaps() const;
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
  virtual void PostInitProperties() override;

  // Select which type of acoustic modeling system is used to generate reverb and reflections.
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  EMetaXRAudioAcousticModel AcousticModel;

  // When enabled and using geometry, all spatailized AudioSources will diffract (propagate around corners and obstructions)
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  bool bDiffractionEnabled;

  // Exclude tags all you to specify Actor Tags which cause meshes with this tag to be excluded from acoustic simulation.
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  TArray<FString> ExcludeTags;

  // When you bake an acoustic map, also bake all the acoustic geometry files
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  bool bMapBakeWriteGeo;

  // The material used for geometry which isn't mapped to any Physical or Meta Material
  UPROPERTY(
      GlobalConfig,
      BlueprintReadWrite,
      EditAnywhere,
      Category = "AcousticsSettings",
      meta = (AllowedClasses = "/Script/MetaXRAudio.MetaXRAcousticMaterialProperties"));
  FSoftObjectPath FallbackMaterial;

  // The user's mapping between Physical Materials and Meta Materials if Use Physical Materials is enabled
  UPROPERTY(GlobalConfig, EditAnywhere, Category = "AcousticsSettings")
  TArray<FMaterialLinkage> MaterialMapping;

  // The Unreal Maps that will be included when performing a bulk bake of Acoustic Maps
  UPROPERTY(GlobalConfig, EditAnywhere, Category = "AcousticsSettings")
  TArray<FFilePath> MapsIncludedInBulkBake;

 private:
  void ApplyAcousticProjectSettings();
};
