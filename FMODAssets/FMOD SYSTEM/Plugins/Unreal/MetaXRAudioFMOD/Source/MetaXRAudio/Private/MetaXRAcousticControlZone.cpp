// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAcousticControlZone.h"
#include "IMetaXRAudioPlugin.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioPlatform.h"
#include "MetaXRAudioUtilities.h"
#ifdef META_NATIVE_UNREAL_PLUGIN
#include "MetaXRAudioContextManager.h"
#endif

AMetaXRAcousticControlZone::AMetaXRAcousticControlZone() {
  MyRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));
  RootComponent = MyRootComponent;
  MyControlZone = CreateDefaultSubobject<UMetaXRAcousticControlZoneWrapper>(TEXT("MyControlZone"));
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = true;
}

void AMetaXRAcousticControlZone::BeginPlay() {
  Super::BeginPlay();
  // Only actually create the control zone during play mode
  if (MetaXRAudioUtilities::PlayModeActive(GetWorld())) {
    StartInternal();
  }

  if (ControlZoneHandle == nullptr) {
    return;
  }

  ApplyProperties();

  if (OVRA_CALL(ovrAudio_ControlZoneSetEnabled)(ControlZoneHandle, true) != ovrSuccess) {
    UE_LOG(LogAudio, Error, TEXT("Unable to enable Control Zone"));
  } else {
    UE_LOG(LogAudio, Log, TEXT("Enabled Control Zone %p"), ControlZoneHandle);
  }
}

void AMetaXRAcousticControlZone::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  if (ControlZoneHandle == nullptr) {
    return;
  }

  // Check if the transform was changed.
  FTransform Transform = GetTransform();
  if (!Transform.Equals(PreviousTransform) || (FadeDistance != PreviousFadeDistance)) {
    ApplyTransform();
  }

  // Check if the spectrums were updated
  if (ReverbLevel.IsDirty || RT60.IsDirty) {
    ApplyProperties();
  }
}

void AMetaXRAcousticControlZone::BeginDestroy() {
  Super::BeginDestroy();
  DestroyInternal();
}

void AMetaXRAcousticControlZone::PostUnregisterAllComponents() {
  Super::PostUnregisterAllComponents();
  DestroyInternal();
}

void AMetaXRAcousticControlZone::StartInternal() {
  // Ensure the control zone is not initialized twice.
  if (ControlZoneHandle == nullptr) {
    // Create the internal Control Zone.
#ifdef META_NATIVE_UNREAL_PLUGIN
    if (OVRA_CALL(ovrAudio_CreateControlZone)(FMetaXRAudioContextManager::GetContext(this), &ControlZoneHandle) != ovrSuccess) {
      UE_LOG(
          LogAudio,
          Error,
          TEXT(
              "Unable to create Control Zone. Please check that the Spatialization Plugin and Reverb Plugin are set to Meta XR Audio in project settings. Restart the project after setting."));
#else
    if (OVRA_CALL(ovrAudio_CreateControlZone)(FMetaXRAudioLibraryManager::Get().GetPluginContext(), &ControlZoneHandle) != ovrSuccess) {
      UE_LOG(LogAudio, Error, TEXT("Unable to create Control Zone. Please check the Meta XR Audio binaries have been placed correctly."));
#endif
      return;
    } else {
      UE_LOG(LogAudio, Log, TEXT("Created Control Zone %p"), ControlZoneHandle);
    }
  }

  // Run the updates to initialize the control.
  ApplyProperties();
  ApplyTransform();
}

void AMetaXRAcousticControlZone::DestroyInternal() {
  if (ControlZoneHandle != nullptr) {
    UE_LOG(LogAudio, Log, TEXT("Destroying Control Zone %p"), ControlZoneHandle);
    if (OVRA_CALL(ovrAudio_DestroyControlZone)(ControlZoneHandle) != ovrSuccess) {
      UE_LOG(LogAudio, Error, TEXT("Unable to destroy Control Zone"));
      return;
    }
    ControlZoneHandle = nullptr;
  }
}

void AMetaXRAcousticControlZone::GetNativeSizes(FVector& OutBoxSize, FVector& OutFadeDistance) const {
  FVector Scale = GetTransform().GetScale3D();
  OutFadeDistance =
      FVector(Scale.X ? FadeDistance / Scale.X : 0.0f, Scale.Y ? FadeDistance / Scale.Y : 0.0f, Scale.Z ? FadeDistance / Scale.Z : 0.0f);
  OutBoxSize = FVector(200.0f + OutFadeDistance.X, 200.0f + OutFadeDistance.Y, 200.0f + OutFadeDistance.Z);
}

void AMetaXRAcousticControlZone::ApplyTransform() {
  FTransform UETransform = GetTransform();
  float OVRTransform[16];
  MetaXRAudioUtilities::ConvertUETransformToOVRTransform(UETransform, OVRTransform);

  if (OVRA_CALL(ovrAudio_ControlZoneSetTransform)(ControlZoneHandle, OVRTransform) != ovrSuccess) {
    UE_LOG(LogAudio, Log, TEXT("Failed to set transform for Control Zone %p"), ControlZoneHandle);
  } else {
    UE_LOG(LogAudio, Log, TEXT("Set transform for Control Zone %p"), ControlZoneHandle);
  }

  PreviousTransform = UETransform;

  // Box Size (converted from ovrAudio coordinates to UE coordinates)
  FVector NativeBoxSize, NativeFadeDistance;
  GetNativeSizes(NativeBoxSize, NativeFadeDistance);
  if (OVRA_CALL(ovrAudio_ControlZoneSetBox)(ControlZoneHandle, NativeBoxSize.Y, NativeBoxSize.Z, NativeBoxSize.X) != ovrSuccess) {
    UE_LOG(LogAudio, Error, TEXT("Failed to set box for Control Zone %p"), ControlZoneHandle);
  } else {
    UE_LOG(
        LogAudio,
        Log,
        TEXT("Set box size for Control Zone %p to {%f, %f ,%f}"),
        ControlZoneHandle,
        NativeBoxSize.X,
        NativeBoxSize.Y,
        NativeBoxSize.Z);
  }

  // Fade Distance (converted from ovrAudio coordinates to UE coordinates)
  if (OVRA_CALL(ovrAudio_ControlZoneSetFadeDistance)(ControlZoneHandle, NativeFadeDistance.Y, NativeFadeDistance.Z, NativeFadeDistance.X) !=
      ovrSuccess) {
    UE_LOG(LogAudio, Error, TEXT("Failed to set fade distance for Control Zone %p"), ControlZoneHandle);
  } else {
    UE_LOG(
        LogAudio,
        Log,
        TEXT("Set fade distance for Control Zone %p to {%f, %f ,%f}"),
        ControlZoneHandle,
        NativeFadeDistance.X,
        NativeFadeDistance.Y,
        NativeFadeDistance.Z);
  }

  PreviousFadeDistance = FadeDistance;
}

void AMetaXRAcousticControlZone::ApplyProperties() {
  if (ControlZoneHandle == nullptr) {
    return;
  }

  // Note both box size and fade distance must convert from UE space to Audio SDK space
  // UE:        x:forward, y:right, z:up
  // Meta XR:  x:right,   y:up,    z:backward

  // RT60
  if (OVRA_CALL(ovrAudio_ControlZoneReset)(ControlZoneHandle, ovrAudioControlZoneProperty_RT60) != ovrSuccess) {
    UE_LOG(LogAudio, Error, TEXT("Failed to reset rt60 for Control Zone %p"), ControlZoneHandle);
  }
  for (FMetaXRAudioPoint p : RT60.Points) {
    if (OVRA_CALL(ovrAudio_ControlZoneSetFrequency)(ControlZoneHandle, ovrAudioControlZoneProperty_RT60, p.Frequency, p.Data) !=
        ovrSuccess) {
      UE_LOG(LogAudio, Error, TEXT("Failed to set rt60 for Control Zone %p"), ControlZoneHandle);
    } else {
      UE_LOG(LogAudio, Log, TEXT("Set rt60 for Control Zone %p for %f Hz to %f"), ControlZoneHandle, p.Frequency, p.Data);
    }
  }
  RT60.IsDirty = false;

  // Reverb level
  if (OVRA_CALL(ovrAudio_ControlZoneReset)(ControlZoneHandle, ovrAudioControlZoneProperty_ReverbLevel) != ovrSuccess) {
    UE_LOG(LogAudio, Error, TEXT("Failed to reset reverb level for Control Zone %p"), ControlZoneHandle);
  }
  for (FMetaXRAudioPoint p : ReverbLevel.Points) {
    if (OVRA_CALL(ovrAudio_ControlZoneSetFrequency)(ControlZoneHandle, ovrAudioControlZoneProperty_ReverbLevel, p.Frequency, p.Data) !=
        ovrSuccess) {
      UE_LOG(LogAudio, Error, TEXT("Failed to set reverb level for Control Zone %p"), ControlZoneHandle);
    } else {
      UE_LOG(LogAudio, Log, TEXT("Set Reverb Level for Control Zone %p for %f Hz to %f"), ControlZoneHandle, p.Frequency, p.Data);
    }
  }
  ReverbLevel.IsDirty = false;
}

FLinearColor UMetaXRAcousticControlZoneWrapper::GetColorWithTransparency(float alpha) const {
  AMetaXRAcousticControlZone* RealControlZone = Cast<AMetaXRAcousticControlZone>(GetOwner());
  if (RealControlZone != nullptr) {
    return RealControlZone->GetColorWithTransparency(alpha);
  } else {
    UE_LOG(
        LogAudio, Log, TEXT("Actor component is not attached to a Meta XR Audio Acoustic Control Zone AActor so will use default color"));
    return FLinearColor::Blue;
  }
}

void UMetaXRAcousticControlZoneWrapper::GetNativeSizes(FVector& OutBoxSize, FVector& OutFadeDistance) const {
  AMetaXRAcousticControlZone* RealControlZone = Cast<AMetaXRAcousticControlZone>(GetOwner());
  if (RealControlZone != nullptr) {
    RealControlZone->GetNativeSizes(OutBoxSize, OutFadeDistance);
  } else {
    OutBoxSize = FVector::ZeroVector;
    OutFadeDistance = FVector::ZeroVector;
    UE_LOG(
        LogAudio,
        Log,
        TEXT("Cannot visualize control zone because the actor component is not attached to a Meta XR Audio Acoustic Control Zone AActor"));
  }
}

void AMetaXRAcousticControlZone::SetControlZoneRT60(const TArray<float>& NewRT60Frequencies, const TArray<float>& NewRT60Values) {
  if (NewRT60Frequencies.Num() != NewRT60Values.Num()) {
    UE_LOG(LogAudio, Error, TEXT("Cannot set control zone RT60 because the frequencies and values arrays are not of equal length"));
    return;
  }
  int NumPoints = FMath::Max(0, FMath::Min(META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS, NewRT60Frequencies.Num()));
  RT60.Points.Empty();
  RT60.Points.SetNum(NumPoints);
  for (int i = 0; i < NumPoints; ++i) {
    float Frequency = FMath::Max(META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(META_XR_AUDIO_MAX_FREQUENCY, NewRT60Frequencies[i]));
    float Value = FMath::Max(META_XR_AUDIO_CONTROL_ZONE_MIN_RT60, FMath::Min(META_XR_AUDIO_CONTROL_ZONE_MAX_RT60, NewRT60Values[i]));
    RT60.Points[i] = FMetaXRAudioPoint{Frequency, Value};
  }
  RT60.IsDirty = true;
}

void AMetaXRAcousticControlZone::GetControlZoneRT60(TArray<float>& OutRT60Frequencies, TArray<float>& OutRT60Values) {
  OutRT60Frequencies.SetNum(RT60.Points.Num());
  OutRT60Values.SetNum(RT60.Points.Num());
  for (int i = 0; i < RT60.Points.Num(); ++i) {
    OutRT60Frequencies[i] = RT60.Points[i].Frequency;
    OutRT60Values[i] = RT60.Points[i].Data;
  }
}

void AMetaXRAcousticControlZone::SetControlZoneReverbLevel(
    const TArray<float>& NewReverbLevelFrequencies,
    const TArray<float>& NewReverbLevelValues) {
  if (NewReverbLevelFrequencies.Num() != NewReverbLevelValues.Num()) {
    UE_LOG(LogAudio, Error, TEXT("Cannot set control zone Reverb Level because the frequencies and values arrays are not of equal length"));
    return;
  }
  int NumPoints = FMath::Max(0, FMath::Min(META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS, NewReverbLevelFrequencies.Num()));
  ReverbLevel.Points.Empty();
  ReverbLevel.Points.SetNum(NumPoints);
  for (int i = 0; i < NumPoints; ++i) {
    float Frequency = FMath::Max(META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(META_XR_AUDIO_MAX_FREQUENCY, NewReverbLevelFrequencies[i]));
    float Value =
        FMath::Max(META_XR_AUDIO_CONTROL_ZONE_MIN_REVERB, FMath::Min(META_XR_AUDIO_CONTROL_ZONE_MAX_REVERB, NewReverbLevelValues[i]));
    ReverbLevel.Points[i] = FMetaXRAudioPoint{Frequency, Value};
  }
  ReverbLevel.IsDirty = true;
}

void AMetaXRAcousticControlZone::GetControlZoneReverbLevel(TArray<float>& OutReverbLevelFrequencies, TArray<float>& OutReverbLevelValues) {
  OutReverbLevelFrequencies.SetNum(ReverbLevel.Points.Num());
  OutReverbLevelValues.SetNum(ReverbLevel.Points.Num());
  for (int i = 0; i < ReverbLevel.Points.Num(); ++i) {
    OutReverbLevelFrequencies[i] = ReverbLevel.Points[i].Frequency;
    OutReverbLevelValues[i] = ReverbLevel.Points[i].Data;
  }
}
