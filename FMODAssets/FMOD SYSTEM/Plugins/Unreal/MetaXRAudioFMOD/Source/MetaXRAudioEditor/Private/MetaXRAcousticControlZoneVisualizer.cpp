// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAcousticControlZoneVisualizer.h"

#include "EditorModes.h"
#include "MetaXRAcousticControlZone.h"

void FMetaXRAcousticControlZoneVisualizer::DrawVisualization(
    const UActorComponent* Component,
    const FSceneView* View,
    FPrimitiveDrawInterface* PDI) {
  static const IConsoleVariable* const GlobalCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos"));
  if (GlobalCVar == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Could not find MetaXRAudioGizmos CVar, gizmos will not be drawn."));
  }
  static const IConsoleVariable* const ControlZoneCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos.ControlZones"));
  if (ControlZoneCVar == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Could not find MetaXRAudioGizmos.ControlZones CVar, gizmos will not be drawn."));
  }

  if (Component == nullptr || PDI == nullptr) {
    return;
  }

  const UMetaXRAcousticControlZoneWrapper* ControlZone = Cast<const UMetaXRAcousticControlZoneWrapper>(Component);
  AActor* OwnerActor = Component->GetOwner();

  if (GlobalCVar == nullptr || GlobalCVar->GetInt() == 0 || ControlZoneCVar == nullptr || ControlZoneCVar->GetInt() == 0 || !ControlZone ||
      !OwnerActor) {
    return;
  }

  // Box outline (native box size is diameter so half for radii)
  const FMatrix BoxToWorld = OwnerActor->GetTransform().ToMatrixWithScale();
  FVector NativeBoxSize, NativeFadeDistance;
  ControlZone->GetNativeSizes(NativeBoxSize, NativeFadeDistance);
  const FMaterialRenderProxy* MaterialRenderProxy =
      new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), ControlZone->GetColorWithTransparency(1.0f));

  FBox box;
  box.BuildAABB(BoxToWorld.GetOrigin(), NativeBoxSize / 2.0);
  DrawWireBox(PDI, box, FLinearColor::Gray, SDPG_World);

  // Box shading (native box size is diameter so half for radii)
  const FMaterialRenderProxy* ShadingMaterialRenderProxy =
      new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), ControlZone->GetColorWithTransparency(0.1f));

  DrawBox(PDI, BoxToWorld, NativeBoxSize / 2.0, ShadingMaterialRenderProxy, SDPG_World);

  // Inner box (native inner size is 200, so radii is 100)
  const FVector3f InnerRadii3f = FVector3f::OneVector * 100.0f;
  const FVector InnerRadii(InnerRadii3f.X, InnerRadii3f.Y, InnerRadii3f.Z);
  const FMaterialRenderProxy* InnerMaterialRenderProxy =
      new FDynamicColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), ControlZone->GetColorWithTransparency(0.2f));

  DrawBox(PDI, BoxToWorld, InnerRadii, InnerMaterialRenderProxy, SDPG_World);
}
