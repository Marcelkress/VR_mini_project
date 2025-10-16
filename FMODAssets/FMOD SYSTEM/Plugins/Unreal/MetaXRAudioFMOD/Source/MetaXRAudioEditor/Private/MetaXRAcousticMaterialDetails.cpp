// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#include "MetaXRAcousticMaterialDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "MetaXRAcousticMaterial.h"
#include "MetaXRAudioEditorInfo.h"
#include "Misc/EngineVersionComparison.h"
#include "PropertyCustomizationHelpers.h"
#include "SEnumCombo.h"
#include "SpectrumWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "Meta XR Acoustic Material Details"

void FMetaXRAcousticMaterialDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) {
  CachedDetailBuilder = DetailBuilder; // Assignment does conversion to TWeakPtr.
  CustomizeDetails(*DetailBuilder);
}

void FMetaXRAcousticMaterialDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
  TArray<TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  if (OutObjects.Num() != 1) {
    return;
  }

  UMetaXRAcousticMaterial* EditedComponent = Cast<UMetaXRAcousticMaterial>(OutObjects[0].Get());

  if (EditedComponent) {
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(META_XR_AUDIO_DISPLAY_NAME, FText::GetEmpty(), ECategoryPriority::Important);

    TSharedRef<SSpectrumWidget> AbsorptionWidget = SNew(SSpectrumWidget)
                                                       .Spectrum(&EditedComponent->GetMaterialPreset()->Data.Absorption)
                                                       .GraphColor(FLinearColor(0.52f, 0.68f, 1.0f))
                                                       .Scale(EAxisScale::Cube)
                                                       .RangeMin(0.0f)
                                                       .RangeMax(1.0f)
                                                       .IsEnabled(false);

    TSharedRef<SSpectrumWidget> TransmissionWidget =
        SNew(SSpectrumWidget)
            .Spectrum(&EditedComponent->GetMaterialPreset()->Data.Transmission)
            .GraphColor(FLinearColor(1.f, 56.f / 85.f, 7.f / 255.f))
            .Scale(EAxisScale::Cube)
            .RangeMin(0.0f)
            .RangeMax(1.0f)
            .IsEnabled_Lambda([EditedComponent]() { return (EditedComponent->GetMaterialPreset() != nullptr); });

    TSharedRef<SSpectrumWidget> ScatteringWidget =
        SNew(SSpectrumWidget)
            .Spectrum(&EditedComponent->GetMaterialPreset()->Data.Scattering)
            .GraphColor(FLinearColor(1.0f, 0.25f, 0.25f))
            .Scale(EAxisScale::Linear)
            .RangeMin(0.0f)
            .RangeMax(1.0f)
            .IsEnabled_Lambda([EditedComponent]() { return (EditedComponent->GetMaterialPreset() != nullptr); });

    Category.AddCustomRow(FText::FromString("Acoustic Material Properties"))
        .WholeRowContent()[SNew(SObjectPropertyEntryBox)
                               .AllowedClass(UMetaXRAcousticMaterialProperties::StaticClass())
                               .ObjectPath_Lambda([EditedComponent]() { return EditedComponent->GetMaterialPreset()->GetPathName(); })
                               .AllowClear(true)
                               .OnObjectChanged_Lambda([this, EditedComponent](const FAssetData& AssetData) {
                                 UMetaXRAcousticMaterialProperties* SelectedAsset =
                                     Cast<UMetaXRAcousticMaterialProperties>(AssetData.GetAsset());
                                 if (SelectedAsset) {
                                   EditedComponent->SetMaterialPropertiesAsset(SelectedAsset);
                                   EditedComponent->MarkPackageDirty();
                                 } else {
                                   EditedComponent->SetMaterialPropertiesAsset(nullptr);
                                   EditedComponent->MarkPackageDirty();
                                 }
                                 Reset();
                               })];

    if (EditedComponent->GetMaterialPreset() == nullptr) {
      return;
    }

    UEnum* PresetEnum = FindFirstObjectSafe<UEnum>(TEXT("EMetaXRAudioMaterialPreset"));
    Category.AddCustomRow(FText::FromString("Preset"))
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Preset"))]
        .ValueContent()[SNew(SBox)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Center)[SNew(SEnumComboBox, PresetEnum)
                                                       .CurrentValue_Lambda([EditedComponent]() {
                                                         return static_cast<int32>(EditedComponent->GetMaterialPreset()->Preset);
                                                       })
                                                       .IsEnabled(false)]];

    // Absorption
    IDetailGroup& AbsorptionGroup = Category.AddGroup("Absorption Group", FText::FromString("Absorption"));
    AbsorptionGroup.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[AbsorptionWidget]];

    IDetailGroup& AbsorptionPoints = AbsorptionGroup.AddGroup("Absorption Points", FText::FromString("Points"));
    AbsorptionPoints.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .IsEnabled(false)
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() {
                              return EditedComponent->GetMaterialPreset()
                                  ? EditedComponent->GetMaterialPreset()->Data.Absorption.Points.Num()
                                  : 0;
                            })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              if (EditedComponent->GetMaterialPreset())
                                EditedComponent->GetMaterialPreset()->Data.Absorption.Points.SetNum(FMath::Max(
                                    META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    AbsorptionPoints.AddWidgetRow()
        .IsEnabled(false)
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Absorption"))];

    const TArray<FMetaXRAudioPoint>& Points = EditedComponent->GetMaterialPreset()->Data.Absorption.Points;
    for (int32 PointIndex = 0; PointIndex < Points.Num(); ++PointIndex) {
      AbsorptionPoints.AddWidgetRow()
          .IsEnabled(false)
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda([EditedComponent, PointIndex]() {
                               if (EditedComponent->GetMaterialPreset() != nullptr &&
                                   PointIndex < EditedComponent->GetMaterialPreset()->Data.Absorption.Points.Num()) {
                                 return EditedComponent->GetMaterialPreset()->Data.Absorption.Points[PointIndex].Frequency;
                               } else {
                                 return 0.0f;
                               }
                             })]
          .ValueContent()[SNew(SNumericEntryBox<float>).Value_Lambda([EditedComponent, PointIndex]() {
            if (EditedComponent->GetMaterialPreset() != nullptr &&
                PointIndex < EditedComponent->GetMaterialPreset()->Data.Absorption.Points.Num()) {
              return EditedComponent->GetMaterialPreset()->Data.Absorption.Points[PointIndex].Data;
            } else {
              return 0.0f;
            }
          })];
    }

    // Transmission
    IDetailGroup& TransmissionGroup = Category.AddGroup("Transmission Group", FText::FromString("Transmission"));
    TransmissionGroup.AddWidgetRow()
        .IsEnabled(false)
        .WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[TransmissionWidget]];

    IDetailGroup& TransmissionPointsGroup = TransmissionGroup.AddGroup("Transmission Points", FText::FromString("Points"));
    TransmissionPointsGroup.AddWidgetRow()
        .IsEnabled(false)
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                            .MinFractionalDigits(0)
#endif
                            .Value_Lambda([EditedComponent]() {
                              return EditedComponent->GetMaterialPreset()
                                  ? EditedComponent->GetMaterialPreset()->Data.Transmission.Points.Num()
                                  : 0;
                            })];

    TransmissionPointsGroup.AddWidgetRow()
        .IsEnabled(false)
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Transmission"))];

    const TArray<FMetaXRAudioPoint>& TransmissionPoints = EditedComponent->GetMaterialPreset()->Data.Transmission.Points;
    for (int32 PointIndex = 0; PointIndex < TransmissionPoints.Num(); ++PointIndex) {
      TransmissionPointsGroup.AddWidgetRow()
          .IsEnabled(false)
          .NameContent()[SNew(SNumericEntryBox<float>).Value_Lambda([EditedComponent, PointIndex]() {
            if (EditedComponent->GetMaterialPreset() != nullptr &&
                PointIndex < EditedComponent->GetMaterialPreset()->Data.Transmission.Points.Num()) {
              return EditedComponent->GetMaterialPreset()->Data.Transmission.Points[PointIndex].Frequency;
            } else {
              return 0.0f;
            }
          })]
          .ValueContent()[SNew(SNumericEntryBox<float>).Value_Lambda([EditedComponent, PointIndex]() {
            if (EditedComponent->GetMaterialPreset() != nullptr &&
                PointIndex < EditedComponent->GetMaterialPreset()->Data.Transmission.Points.Num()) {
              return EditedComponent->GetMaterialPreset()->Data.Transmission.Points[PointIndex].Data;
            } else {
              return 0.0f;
            }
          })];
    }

    // Scattering
    IDetailGroup& ScatteringGroup = Category.AddGroup("Scattering Group", FText::FromString("Scattering"));
    ScatteringGroup.AddWidgetRow().IsEnabled(false).WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[ScatteringWidget]];

    IDetailGroup& ScatteringPointsGroup = ScatteringGroup.AddGroup("Scattering Points", FText::FromString("Points"));
    ScatteringPointsGroup.AddWidgetRow()
        .IsEnabled(false)
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                            .MinFractionalDigits(0)
#endif
                            .Value_Lambda([EditedComponent]() {
                              return EditedComponent->GetMaterialPreset()
                                  ? EditedComponent->GetMaterialPreset()->Data.Scattering.Points.Num()
                                  : 0;
                            })];

    ScatteringPointsGroup.AddWidgetRow()
        .IsEnabled(false)
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Scattering"))];

    const TArray<FMetaXRAudioPoint>& ScatteringPoints = EditedComponent->GetMaterialPreset()->Data.Scattering.Points;
    for (int32 PointIndex = 0; PointIndex < ScatteringPoints.Num(); ++PointIndex) {
      ScatteringPointsGroup.AddWidgetRow()
          .IsEnabled(false)
          .NameContent()[SNew(SNumericEntryBox<float>).Value_Lambda([EditedComponent, PointIndex]() {
            if (EditedComponent->GetMaterialPreset() != nullptr &&
                PointIndex < EditedComponent->GetMaterialPreset()->Data.Scattering.Points.Num()) {
              return EditedComponent->GetMaterialPreset()->Data.Scattering.Points[PointIndex].Frequency;
            } else {
              return 0.0f;
            }
          })]
          .ValueContent()[SNew(SNumericEntryBox<float>).Value_Lambda([EditedComponent, PointIndex]() {
            if (EditedComponent->GetMaterialPreset() != nullptr &&
                PointIndex < EditedComponent->GetMaterialPreset()->Data.Scattering.Points.Num()) {
              return EditedComponent->GetMaterialPreset()->Data.Scattering.Points[PointIndex].Data;
            } else {
              return 0.0f;
            }
          })];
    }
  }
}

void FMetaXRAcousticMaterialDetails::Reset() {
  IDetailLayoutBuilder* LayoutBuilder = CachedDetailBuilder.Pin().Get();
  LayoutBuilder->ForceRefreshDetails();
}

#undef LOCTEXT_NAMESPACE
