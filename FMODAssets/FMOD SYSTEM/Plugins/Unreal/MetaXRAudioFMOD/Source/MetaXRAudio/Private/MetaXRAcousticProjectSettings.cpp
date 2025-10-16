// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAcousticProjectSettings.h"

#include "MetaXRAudioDllManager.h"
#include "MetaXR_Audio.h"

#if WITH_EDITOR
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "MetaXRAcousticMap.h"
#include "Misc/EngineVersionComparison.h"
#endif // WITH_EDITOR

UMetaXRAcousticProjectSettings::UMetaXRAcousticProjectSettings()
    : AcousticModel(EMetaXRAudioAcousticModel::Automatic), bDiffractionEnabled(true), ExcludeTags(), bMapBakeWriteGeo(true) {}

void UMetaXRAcousticProjectSettings::PostInitProperties() {
  // Ensure the settings are applied when the project or game is loaded
  ApplyAcousticProjectSettings();
  Super::PostInitProperties();
}

void UMetaXRAcousticProjectSettings::ApplyAcousticProjectSettings() {
  // This call only works for wwise and fmod. native unreal applies these settings in reverb process function
  ovrAudioContext currentContext = FMetaXRAudioLibraryManager::Get().GetPluginContext();

  if (currentContext != nullptr) {
    ovrResult Result = OVRA_CALL(ovrAudio_Enable)(currentContext, ovrAudioEnable_Diffraction, bDiffractionEnabled);
    if (Result == ovrSuccess) {
      UE_LOG(LogAudio, Log, TEXT("Diffraction enabled successfully set to %i"), bDiffractionEnabled);
    } else {
      UE_LOG(LogAudio, Warning, TEXT("Failed to set diffraction enabled with error: %s"), GetMetaXRErrorString(Result));
    }

    Result = OVRA_CALL(ovrAudio_SetAcousticModel)(currentContext, static_cast<ovrAudioAcousticModel>(AcousticModel));
    if (Result == ovrSuccess) {
      UE_LOG(LogAudio, Log, TEXT("Acoustic Model successfully set to %i"), AcousticModel);
    } else {
      UE_LOG(LogAudio, Warning, TEXT("Failed to set Acoustic Model with error: %s"), GetMetaXRErrorString(Result));
    }
  }
}

#if WITH_EDITOR
void UMetaXRAcousticProjectSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) {
  ApplyAcousticProjectSettings();
  Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UMetaXRAcousticProjectSettings::BulkBakeAcousticMaps() const {
  UE_LOG(LogAudio, Log, TEXT("Bulk bake of Acoustic Maps is now starting"));

  if (MapsIncludedInBulkBake.IsEmpty()) {
    UE_LOG(
        LogAudio,
        Warning,
        TEXT("There are no UMaps specified in the Meta XR Acoustics Project Settings to perform the bake on, aborting now."));
    return;
  }

  // Generate a progress bar (only available after 5.4)
#if UE_VERSION_OLDER_THAN(5, 4, 0)
#else
#define LOCTEXT_NAMESPACE "MetaXRAudioBulkBake"
  FScopedSlowTask ProgressBar(
      MapsIncludedInBulkBake.Num(), LOCTEXT("MetaXRAudioBulkBakeTextLabel", "Bulk Bake in Progress..."), true, *GWarn);
  ProgressBar.MakeDialog(true);
#undef LOCTEXT_NAMESPACE
#endif // older than 5.4.0

  for (FFilePath MapPath : MapsIncludedInBulkBake) {
#if UE_VERSION_OLDER_THAN(5, 4, 0)
#else
    // Check if user requested early exit
    if (ProgressBar.ShouldCancel()) {
      UE_LOG(LogAudio, Warning, TEXT("Bulk bake was cancelled before completeion"));
      return;
    }

    // Update the progress bar where each loop represents 1 out of the total number of maps
    ProgressBar.EnterProgressFrame();
#endif // older than 5.4.0

    // Load the Unreal uMap that the user specified
    FString MapFile = MapPath.FilePath;
    if (!FEditorFileUtils::LoadMap(MapFile, false, true)) {
      UE_LOG(LogAudio, Error, TEXT("Failed to load the following umap during the bulk bake: %s"), *MapFile);
      continue;
    } else {
      UE_LOG(LogAudio, Display, TEXT("Bulk bake successfully loaded the following umap: %s"), *MapFile);
    }

    // The UWorld is specific to each umap and required to find Acoustic Maps
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World == nullptr) {
      UE_LOG(LogAudio, Warning, TEXT("Failed to find a UWorld associated with the umap: %s"), *MapFile);
      continue;
    }

    TArray<FString> LevelsWithMap;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr) {
      AActor* CurrentActor = *ActorItr;
      UMetaXRAcousticMap* MapComponent = CurrentActor->FindComponentByClass<UMetaXRAcousticMap>();
      if (MapComponent) {
        UE_LOG(LogAudio, Display, TEXT("Bulk bake detected and will start bake for an Acoustic map in umap: %s"), *MapFile);
        // Need to perform blocking compute (not async). Otherwise we will go to the next map and destroy the map component before it
        // finishes
        MapComponent->Compute(false, true);
      }
    }
  }

  UE_LOG(LogAudio, Display, TEXT("Bulk bake of Acoustic Maps is now complete"));
}
#endif
