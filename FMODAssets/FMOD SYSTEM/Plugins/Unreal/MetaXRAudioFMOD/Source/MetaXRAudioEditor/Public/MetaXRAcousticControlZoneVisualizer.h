// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "ComponentVisualizer.h"
#include "CoreMinimal.h"

/**
 *    Editor visualization of Control Zone box
 */
class METAXRAUDIOEDITOR_API FMetaXRAcousticControlZoneVisualizer : public FComponentVisualizer {
 public:
  virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
