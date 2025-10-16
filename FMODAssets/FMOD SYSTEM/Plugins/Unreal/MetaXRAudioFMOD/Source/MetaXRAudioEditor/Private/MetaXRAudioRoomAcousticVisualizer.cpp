// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAudioRoomAcousticVisualizer.h"

#include "EditorModes.h"
#include "MetaXRAudioRoomAcousticProperties.h"

void FMetaXRAudioRoomAcousticVisualizer::DrawVisualization(
    const UActorComponent* Component,
    const FSceneView* View,
    FPrimitiveDrawInterface* PDI) {
  static const IConsoleVariable* const GlobalCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos"));
  if (GlobalCVar == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Could not find MetaXRAudioGizmos CVar, gizmos will not be drawn."));
  }
  static const IConsoleVariable* const RoomCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos.Shoeboxes"));
  if (RoomCVar == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Could not find MetaXRAudioGizmos.Shoeboxes CVar, gizmos will not be drawn."));
  }

  // This is possible when the root actor containing the UMetaXRAudioRoomAcousticProperties component is deleted.
  if (Component == nullptr) {
    UE_LOG(LogAudio, Warning, TEXT("UActorComponent destroyed, but attempting to visualize it anyway. Not drawing Gizmos..."));
    return;
  }

  const UMetaXRAudioRoomAcousticProperties* RoomAcoustics = Cast<const UMetaXRAudioRoomAcousticProperties>(Component);
  AActor* OwnerActor = Component->GetOwner();

  if (GlobalCVar == nullptr || GlobalCVar->GetInt() == 0 || RoomCVar == nullptr || RoomCVar->GetInt() == 0 || !RoomAcoustics ||
      !OwnerActor) {
    return;
  }

  const FMatrix BoxToWorld = OwnerActor->GetTransform().ToMatrixNoScale();
  const FVector Radii(RoomAcoustics->GetDepth() / 2.f, RoomAcoustics->GetWidth() / 2.f, RoomAcoustics->GetHeight() / 2.f);
  const FMaterialRenderProxy* MaterialRenderProxy =
      new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), {1.f, 1.f, 1.f, 0.15f});

  DrawBox(PDI, BoxToWorld, Radii, MaterialRenderProxy, SDPG_World);
}
