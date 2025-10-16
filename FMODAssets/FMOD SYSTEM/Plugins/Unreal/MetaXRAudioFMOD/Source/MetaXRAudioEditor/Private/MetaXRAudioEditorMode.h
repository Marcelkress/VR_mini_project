// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "BaseBehaviors/KeyAsModifierInputBehavior.h"
#include "BaseGizmos/TransformProxy.h"
#include "DynamicMeshBuilder.h"
#include "EdMode.h"
#include "Editor.h"
#include "MaterialShared.h"
#include "MetaXRAcousticMap.h"

class FColoredMaterialRenderProxy;

class FMetaXRAudioEditorMode : public FEdMode {
 public:
  const static FEditorModeID EM_MetaXRAcousticMapEditorModeId;

  FMetaXRAudioEditorMode();
  virtual ~FMetaXRAudioEditorMode() override;

  virtual void Enter() override;
  virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
  virtual void Exit() override;
  virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

 private:
  bool PerformRaycast(FEditorViewportClient* ViewportClient, const FVector2D& MousePosition);

  UPROPERTY()
  TObjectPtr<UTransformProxy> TransformProxy;
  UPROPERTY()
  TObjectPtr<UCombinedTransformGizmo> TransformGizmo;

  // Define sphere parameters
  UMetaXRAcousticMap* EditedComponent = nullptr;
};
