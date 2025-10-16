// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAcousticMaterialPropertiesFactory.h"
#include "MetaXRAcousticMaterialProperties.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_MetaXRAcousticMaterialProperties::GetName() const {
  return LOCTEXT("AssetTypeActions_MetaXRAudioSpatializationSettings", "Meta XR Acoustic Material Properties");
}

const TArray<FText>& FAssetTypeActions_MetaXRAcousticMaterialProperties::GetSubMenus() const {
  static const TArray<FText> SubMenus{LOCTEXT("AssetSoundMetaXRSubMenu", "MetaXR")};

  return SubMenus;
}

FColor FAssetTypeActions_MetaXRAcousticMaterialProperties::GetTypeColor() const {
  return FColor(100, 100, 100);
}

UClass* FAssetTypeActions_MetaXRAcousticMaterialProperties::GetSupportedClass() const {
  return UMetaXRAcousticMaterialProperties::StaticClass();
}

uint32 FAssetTypeActions_MetaXRAcousticMaterialProperties::GetCategories() {
  return EAssetTypeCategories::Sounds;
}

UMetaXRAcousticMaterialPropertiesFactory::UMetaXRAcousticMaterialPropertiesFactory() {
  bCreateNew = true;
  bEditAfterNew = true;
  SupportedClass = UMetaXRAcousticMaterialProperties::StaticClass();
}

UObject* UMetaXRAcousticMaterialPropertiesFactory::FactoryCreateNew(
    UClass* Class,
    UObject* InParent,
    FName Name,
    EObjectFlags Flags,
    UObject* Context,
    FFeedbackContext* Warn) {
  return NewObject<UMetaXRAcousticMaterialProperties>(InParent, Class, Name, Flags);
}

#undef LOCTEXT_NAMESPACE
