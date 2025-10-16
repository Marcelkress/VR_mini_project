// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#ifndef METAXRAUDIOUTILITIES
#define METAXRAUDIOUTILITIES

#include "AudioPluginUtilities.h"
#include "Engine/Engine.h"
#include "MetaXRAudioEditorInfo.h"
#include "MetaXRAudioPlatform.h"
#include "MetaXR_Audio.h"

#if WITH_EDITOR
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlOperations.h"
#endif // WITH_EDITOR

class MetaXRAudioUtilities {
 public:
  // Helper function to convert from UE coords to OVR coords.
  static FVector ToOVRVector(const FVector& InVec) {
    return FVector(InVec.Y, InVec.Z, -InVec.X);
  }

  // Helper function to convert from OVR coords to UE coords.
  static FVector ToUEVector(const FVector& InVec) {
    return FVector(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector3f ToUEVector3f(const FVector3f& InVec) {
    return FVector3f(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector ToOVRVector(const Audio::FChannelPositionInfo& ChannelPositionInfo) {
    FVector OvrVector;
    OvrVector.X = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Cos(ChannelPositionInfo.Elevation);
    OvrVector.Y = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Sin(ChannelPositionInfo.Elevation);
    OvrVector.Z = ChannelPositionInfo.Radius * FMath::Cos(ChannelPositionInfo.Azimuth);

    return OvrVector;
  }

  // Currently a UE Vector directly to a UE Vector3f
  static FVector3f VectorToVector3f(const FVector& InVec) {
    return FVector3f(InVec.X, InVec.Y, InVec.Z);
  }

  static float dbToLinear(float db) {
    return powf(10.0f, db / 20.0f);
  }

  static void ConvertUETransformToOVRTransform(const FTransform& InTransform, float OutTransform[16]) {
    // UE:        x:forward, y:right, z:up
    // Meta XR:  x:right,   y:up,    z:backward
    // UE y = Meta x | UE z = meta y | negative UE x = meta z
    const FMatrix Matrix = InTransform.ToMatrixWithScale();
    OutTransform[0] = (float)Matrix.M[1][1];
    OutTransform[1] = (float)Matrix.M[1][2];
    OutTransform[2] = (float)-Matrix.M[1][0];
    OutTransform[3] = (float)Matrix.M[1][3]; // UE y right - > Meta x right

    OutTransform[4] = (float)Matrix.M[2][1];
    OutTransform[5] = (float)Matrix.M[2][2];
    OutTransform[6] = (float)-Matrix.M[2][0];
    OutTransform[7] = (float)Matrix.M[2][3]; // UE z up-> Meta y up

    OutTransform[8] = (float)-Matrix.M[0][1];
    OutTransform[9] = (float)-Matrix.M[0][2];
    OutTransform[10] = (float)Matrix.M[0][0];
    OutTransform[11] = (float)-Matrix.M[0][3]; // UE x forward -> Meta z backward

    OutTransform[12] = (float)Matrix.M[3][1];
    OutTransform[13] = (float)Matrix.M[3][2];
    OutTransform[14] = (float)-Matrix.M[3][0];
    OutTransform[15] = (float)Matrix.M[3][3]; // position
  }

  static bool PlayModeActive(UWorld* World) {
    if (World != nullptr) {
      if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) {
        return true;
      }
    }
    return false;
  }

#if WITH_EDITOR
  // There is a bug with Unreal where the level always returns persistent level name
  // Getting the outer returns the actual level name associated with the aactor
  static FString GetActorLevelName(AActor* Actor) {
    if (Actor) {
      ULevel* Level = Actor->GetLevel();
      if (Level) {
        UObject* OuterLevel = Level->GetOuter();
        if (OuterLevel) {
          return OuterLevel->GetName();
        }
      }
    }
    return FString();
  }

  static void CreateMetaXRAcousticContentDirectory(FString MetaContentDirectoryName) {
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FString DesiredDirectory = FPaths::ProjectContentDir() + MetaContentDirectoryName;
    if (!PlatformFile.DirectoryExists(*DesiredDirectory)) {
      PlatformFile.CreateDirectoryTree(*DesiredDirectory);
      UE_LOG(
          LogAudio,
          Display,
          TEXT("Generating the %s folder in Content as the recommend place to save Meta XR Audio files"),
          *MetaContentDirectoryName)
    }
  }
#endif

  static bool IsMetaXRAudioTheCurrentSpatializationPlugin() {
#ifdef META_NATIVE_UNREAL_PLUGIN
#if WITH_EDITOR
    static FString MetaXRDisplayName = FString(TEXT(META_XR_AUDIO_DISPLAY_NAME));
    return AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
#else // !WITH_EDITOR
    // For non editor situations, we can cache whether this is the current plugin or not the first time we check.
    static FString MetaXRDisplayName = FString(TEXT(META_XR_AUDIO_DISPLAY_NAME));
    static bool bCheckedSpatializationPlugin = false;
    static bool bIsMetaXRCurrentSpatiatizationPlugin = false;

    if (!bCheckedSpatializationPlugin) {
      bIsMetaXRCurrentSpatiatizationPlugin =
          AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
      bCheckedSpatializationPlugin = true;
    }

    return bIsMetaXRCurrentSpatiatizationPlugin;
#endif // WITH_EDITOR
#else // !META_NATIVE_UNREAL_PLUGIN
    return false;
#endif // META_NATIVE_UNREAL_PLUGIN
  }

#if WITH_EDITOR
  static void CheckOutFilesInSourceControl(TArray<FString> FilePaths) {
    // Ensure Source Control is enabled for the project and is allowed to checkout files
    ISourceControlProvider* SourceControlProvider = &ISourceControlModule::Get().GetProvider();
    if ((SourceControlProvider == nullptr) || !SourceControlProvider->IsEnabled() || !SourceControlProvider->UsesCheckout()) {
      UE_LOG(LogAudio, Log, TEXT("Will not check out files as source control is not available"));
      return;
    }

    // Get the state of the requested files in source control
    ECommandResult::Type Result;
    TArray<FSourceControlStateRef> OutStates;
    Result = SourceControlProvider->GetState(FilePaths, OutStates, EStateCacheUsage::ForceUpdate);

    // We can skip if the file is already checked-out or added
    TArray<FString> FilesToBeCheckedOut;
    TArray<FString> FilesToBeAdded;
    for (FSourceControlStateRef OutState : OutStates) {
      if (!OutState->IsCheckedOut() && !OutState->IsAdded() && OutState->IsSourceControlled() && OutState->CanCheckout()) {
        FilesToBeCheckedOut.Add(OutState->GetFilename());
      }
      if (!OutState->IsAdded() && OutState->CanAdd() && !OutState->IsSourceControlled()) {
        FilesToBeAdded.Add(OutState->GetFilename());
      }
    }

    // Now that the operation if confirmed to be valid, perform the checkout
    if (FilesToBeCheckedOut.Num()) {
      Result = SourceControlProvider->Execute(ISourceControlOperation::Create<FCheckOut>(), FilesToBeCheckedOut);
      for (FString FilePath : FilesToBeCheckedOut) {
        UE_LOG(LogAudio, Log, TEXT("Source control checkout on file %s finished with result: %i"), *FilePath, Result);
      }
    }

    // If the file is new, mark it for adding
    if (FilesToBeAdded.Num()) {
      Result = SourceControlProvider->Execute(ISourceControlOperation::Create<FMarkForAdd>(), FilesToBeAdded);
      for (FString FilePath : FilesToBeAdded) {
        UE_LOG(LogAudio, Log, TEXT("Source control added file %s finished with result: %i"), *FilePath, Result);
      }
    }
  }
#endif // WITH_EDITOR

}; // end class MetaXRAudioUtilities

#endif // METAXRAUDIOUTILITIES
