// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "MetaXRAcousticMaterialPropertiesFactory.generated.h"

class FAssetTypeActions_MetaXRAcousticMaterialProperties : public FAssetTypeActions_Base {
 public:
  virtual FText GetName() const override;
  virtual FColor GetTypeColor() const override;
  virtual const TArray<FText>& GetSubMenus() const override;
  virtual UClass* GetSupportedClass() const override;
  virtual uint32 GetCategories() override;
};

UCLASS()
class UMetaXRAcousticMaterialPropertiesFactory : public UFactory {
  GENERATED_BODY()

 public:
  UMetaXRAcousticMaterialPropertiesFactory();

  virtual UObject*
  FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
