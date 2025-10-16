// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/PrimitiveComponent.h"
#include "MetaXRAudioSpectrum.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticControlZone.generated.h"

#define META_XR_AUDIO_CONTROL_ZONE_MIN_RT60 -5.0f
#define META_XR_AUDIO_CONTROL_ZONE_MAX_RT60 5.0f
#define META_XR_AUDIO_CONTROL_ZONE_MIN_REVERB -24.0f
#define META_XR_AUDIO_CONTROL_ZONE_MAX_REVERB 24.0f

UCLASS(
    Placeable,
    ClassGroup = (Audio),
    HideCategories = (Cooking, Physics, Networking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Control Zone",
         ToolTip = "Adjust the reverb level and RT60 in a particular region"))
class METAXRAUDIO_API AMetaXRAcousticControlZone : public AActor {
  GENERATED_BODY()

 public:
  AMetaXRAcousticControlZone();

  // The color used to visualize this control zone's boundaries
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Meta XR Audio")
  FLinearColor Color = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

  // Adjust the blending of the control zone settings with the base map settings outside the boxSize
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Meta XR Audio", meta = (ClampMin = "0.0", UIMin = "0.0"))
  float FadeDistance = 100.0f;

  UPROPERTY()
  FMetaXRAudioSpectrum RT60;

  // Set the RT60 of a control zone
  UFUNCTION(BlueprintCallable, Category = "MetaXRAudioControlZone")
  void SetControlZoneRT60(const TArray<float>& NewRT60Frequencies, const TArray<float>& NewRT60Values);

  // Get the RT60 of a control zone
  UFUNCTION(BlueprintCallable, Category = "MetaXRAudioControlZone")
  void GetControlZoneRT60(TArray<float>& OutRT60Frequencies, TArray<float>& OutRT60Values);

  UPROPERTY()
  FMetaXRAudioSpectrum ReverbLevel;

  // Set the reverb level of a control zone
  UFUNCTION(BlueprintCallable, Category = "MetaXRAudioControlZone")
  void SetControlZoneReverbLevel(const TArray<float>& NewReverbLevelFrequencies, const TArray<float>& NewReverbLevelValues);

  // Get the reverb level of a control zone
  UFUNCTION(BlueprintCallable, Category = "MetaXRAudioControlZone")
  void GetControlZoneReverbLevel(TArray<float>& OutReverbLevelFrequencies, TArray<float>& OutReverbLevelValues);

  // The scene component provides the actor a reference so it can be transformed
  UPROPERTY()
  USceneComponent* MyRootComponent;

  UPROPERTY()
  UMetaXRAcousticControlZoneWrapper* MyControlZone;

  FLinearColor GetColorWithTransparency(float alpha) {
    return FLinearColor(Color.R, Color.G, Color.B, alpha);
  }
  void GetNativeSizes(FVector& OutBoxSize, FVector& OutFadeDistance) const;
  void ApplyProperties();

 private:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
  virtual void BeginDestroy() override;
  virtual void PostUnregisterAllComponents() override;
  void StartInternal();
  void DestroyInternal();
  void ApplyTransform();

  ovrAudioContext CachedContext;
  ovrAudioControlZone ControlZoneHandle = nullptr;
  FTransform PreviousTransform;
  float PreviousFadeDistance;
};

UCLASS(Hidden)
class METAXRAUDIO_API UMetaXRAcousticControlZoneWrapper : public UActorComponent {
  GENERATED_BODY()

 public:
  FLinearColor GetColorWithTransparency(float alpha) const;
  void GetNativeSizes(FVector& OutBoxSize, FVector& OutFadeDistance) const;

 private:
};
