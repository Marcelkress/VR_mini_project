// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAcousticMaterial.h"

#if WITH_EDITOR
void UMetaXRAcousticMaterial::SetMaterialPropertiesAsset(UMetaXRAcousticMaterialProperties* NewProperties) {
  MaterialPreset = NewProperties;
}
#endif

void UMetaXRAcousticMaterial::AppendHash(FString& hash) {
  if (MaterialPreset != nullptr) {
    MaterialPreset->AppendHash(hash);
  }
}
