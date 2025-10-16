// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#pragma once

#include "IDetailCustomization.h"

class METAXRAUDIOEDITOR_API FMetaXRAcousticMapDetails : public IDetailCustomization {
 public:
  static TSharedRef<IDetailCustomization> MakeInstance() {
    return MakeShareable(new FMetaXRAcousticMapDetails);
  }
  FString SaveFileDialog(AActor* Context) const;
  virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
