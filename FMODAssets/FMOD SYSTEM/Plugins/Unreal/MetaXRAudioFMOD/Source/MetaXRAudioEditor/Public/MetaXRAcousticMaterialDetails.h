// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "IDetailCustomization.h"
#include "MetaXRAcousticMaterial.h"

class METAXRAUDIOEDITOR_API FMetaXRAcousticMaterialDetails : public IDetailCustomization {
 public:
  static TSharedRef<IDetailCustomization> MakeInstance() {
    return MakeShareable(new FMetaXRAcousticMaterialDetails);
  }

  virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
  virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
  void Reset();

 private:
  TWeakPtr<IDetailLayoutBuilder> CachedDetailBuilder;
};
