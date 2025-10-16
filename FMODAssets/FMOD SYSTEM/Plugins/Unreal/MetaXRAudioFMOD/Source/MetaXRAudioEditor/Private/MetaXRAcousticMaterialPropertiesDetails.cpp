// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#include "MetaXRAcousticMaterialPropertiesDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "MetaXRAcousticMaterialProperties.h"
#include "MetaXRAudioEditorInfo.h"
#include "Misc/EngineVersionComparison.h"
#include "SEnumCombo.h"
#include "SpectrumWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "Material Component Details"

void FMetaXRAcousticMaterialPropertiesDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) {
  CachedDetailBuilder = DetailBuilder;
  CustomizeDetails(*DetailBuilder);
}
void FMetaXRAcousticMaterialPropertiesDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
  TArray<TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  if (OutObjects.Num() != 1) {
    return;
  }

  UMetaXRAcousticMaterialProperties* EditedComponent = Cast<UMetaXRAcousticMaterialProperties>(OutObjects[0].Get());

  if (EditedComponent) {
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(META_XR_AUDIO_DISPLAY_NAME, FText::GetEmpty(), ECategoryPriority::Important);

    TSharedRef<SSpectrumWidget> AbsorptionWidget = SNew(SSpectrumWidget)
                                                       .Spectrum(&EditedComponent->Data.Absorption)
                                                       .GraphColor(FLinearColor(0.52f, 0.68f, 1.0f))
                                                       .Scale(EAxisScale::Cube)
                                                       .RangeMin(0.0f)
                                                       .RangeMax(1.0f);

    TSharedRef<SSpectrumWidget> TransmissionWidget = SNew(SSpectrumWidget)
                                                         .Spectrum(&EditedComponent->Data.Transmission)
                                                         .GraphColor(FLinearColor(1.f, 56.f / 85.f, 7.f / 255.f))
                                                         .Scale(EAxisScale::Cube)
                                                         .RangeMin(0.0f)
                                                         .RangeMax(1.0f);

    TSharedRef<SSpectrumWidget> ScatteringWidget = SNew(SSpectrumWidget)
                                                       .Spectrum(&EditedComponent->Data.Scattering)
                                                       .GraphColor(FLinearColor(1.0f, 0.25f, 0.25f))
                                                       .Scale(EAxisScale::Linear)
                                                       .RangeMin(0.0f)
                                                       .RangeMax(1.0f);

    UEnum* PresetEnum = FindFirstObjectSafe<UEnum>(TEXT("EMetaXRAudioMaterialPreset"));
    Category.AddCustomRow(FText::FromString("Preset"))
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Preset"))]
        .ValueContent()[SNew(SBox)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Center)
                                [SNew(SEnumComboBox, PresetEnum)
                                     .CurrentValue_Lambda([EditedComponent]() { return static_cast<int32>(EditedComponent->Preset); })
                                     .OnEnumSelectionChanged_Lambda([this, EditedComponent](int32 Value, ESelectInfo::Type Type) {
                                       EditedComponent->ApplyPreset(static_cast<EMetaXRAudioMaterialPreset>(Value));
                                       EditedComponent->MarkPackageDirty();
                                       Reset();
                                     })]];

    // Absorption
    IDetailGroup& AbsorptionGroup = Category.AddGroup("Absorption Group", FText::FromString("Absorption"));
    AbsorptionGroup.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[AbsorptionWidget]];

    IDetailGroup& AbsorptionPoints = AbsorptionGroup.AddGroup("Absorption Points", FText::FromString("Points"));
    AbsorptionPoints.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->Data.Absorption.Points.Num(); })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              EditedComponent->Data.Absorption.Points.SetNum(FMath::Max(
                                  META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    AbsorptionPoints.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Absorption"))];

    const TArray<FMetaXRAudioPoint>& Points = EditedComponent->Data.Absorption.Points;
    for (int32 PointIndex = 0; PointIndex < Points.Num(); ++PointIndex) {
      AbsorptionPoints.AddWidgetRow()
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda([EditedComponent, PointIndex]() {
                               if (PointIndex < EditedComponent->Data.Absorption.Points.Num()) {
                                 return EditedComponent->Data.Absorption.Points[PointIndex].Frequency;
                               } else {
                                 return 0.0f;
                               }
                             })
                             .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                               if (PointIndex < EditedComponent->Data.Absorption.Points.Num()) {
                                 EditedComponent->Modify();
                                 EditedComponent->Data.Absorption.Points[PointIndex].Frequency = FMath::Max(
                                     META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(FMath::RoundToZero(Value), META_XR_AUDIO_MAX_FREQUENCY));
                               }
                             })
                             .MinValue(TNumericLimits<float>::Lowest())
                             .MaxValue(TNumericLimits<float>::Max())
                             .Delta(1)]
          .ValueContent()[SNew(SNumericEntryBox<float>)
                              .Value_Lambda([EditedComponent, PointIndex]() {
                                if (PointIndex < EditedComponent->Data.Absorption.Points.Num()) {
                                  return EditedComponent->Data.Absorption.Points[PointIndex].Data;
                                } else {
                                  return 0.0f;
                                }
                              })
                              .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                                if (PointIndex < EditedComponent->Data.Absorption.Points.Num()) {
                                  EditedComponent->Modify();
                                  EditedComponent->Data.Absorption.Points[PointIndex].Data =
                                      FMath::Max(META_XR_AUDIO_MIN_ABSOPRTION, FMath::Min(Value, META_XR_AUDIO_MAX_ABSOPRTION));
                                }
                              })
                              .MinValue(TNumericLimits<float>::Lowest())
                              .MaxValue(TNumericLimits<float>::Max())
                              .Delta(1)]
          .ExtensionContent()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent, PointIndex]() {
            if (PointIndex < EditedComponent->Data.Absorption.Points.Num()) {
              EditedComponent->Data.Absorption.Points.RemoveAt(PointIndex);
              Reset();
            }
            return FReply::Handled();
          })];
    }

    // Transmission
    IDetailGroup& TransmissionGroup = Category.AddGroup("Transmission Group", FText::FromString("Transmission"));
    TransmissionGroup.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[TransmissionWidget]];

    IDetailGroup& TransmissionPointsGroup = TransmissionGroup.AddGroup("Transmission Points", FText::FromString("Points"));
    TransmissionPointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->Data.Transmission.Points.Num(); })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              EditedComponent->Data.Transmission.Points.SetNum(FMath::Max(
                                  META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    TransmissionPointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Transmission"))];

    const TArray<FMetaXRAudioPoint>& TransmissionPoints = EditedComponent->Data.Transmission.Points;
    for (int32 PointIndex = 0; PointIndex < TransmissionPoints.Num(); ++PointIndex) {
      TransmissionPointsGroup.AddWidgetRow()
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda([EditedComponent, PointIndex]() {
                               if (PointIndex < EditedComponent->Data.Transmission.Points.Num()) {
                                 return EditedComponent->Data.Transmission.Points[PointIndex].Frequency;
                               } else {
                                 return 0.0f;
                               }
                             })
                             .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                               if (PointIndex < EditedComponent->Data.Transmission.Points.Num()) {
                                 EditedComponent->Modify();
                                 EditedComponent->Data.Transmission.Points[PointIndex].Frequency = FMath::Max(
                                     META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(FMath::RoundToZero(Value), META_XR_AUDIO_MAX_FREQUENCY));
                               }
                             })
                             .MinValue(TNumericLimits<float>::Lowest())
                             .MaxValue(TNumericLimits<float>::Max())
                             .Delta(1)]
          .ValueContent()[SNew(SNumericEntryBox<float>)
                              .Value_Lambda([EditedComponent, PointIndex]() {
                                if (PointIndex < EditedComponent->Data.Transmission.Points.Num()) {
                                  return EditedComponent->Data.Transmission.Points[PointIndex].Data;
                                } else {
                                  return 0.0f;
                                }
                              })
                              .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                                if (PointIndex < EditedComponent->Data.Transmission.Points.Num()) {
                                  EditedComponent->Modify();
                                  EditedComponent->Data.Transmission.Points[PointIndex].Data =
                                      FMath::Max(META_XR_AUDIO_MIN_TRANSMISSION, FMath::Min(Value, META_XR_AUDIO_MAX_TRANSMISSION));
                                }
                              })
                              .MinValue(TNumericLimits<float>::Lowest())
                              .MaxValue(TNumericLimits<float>::Max())
                              .Delta(1)]
          .ExtensionContent()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent, PointIndex]() {
            if (PointIndex < EditedComponent->Data.Transmission.Points.Num()) {
              EditedComponent->Data.Transmission.Points.RemoveAt(PointIndex);
              Reset();
            }
            return FReply::Handled();
          })];
    }

    // Scattering
    IDetailGroup& ScatteringGroup = Category.AddGroup("Scattering Group", FText::FromString("Scattering"));
    ScatteringGroup.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(140).WidthOverride(500)[ScatteringWidget]];

    IDetailGroup& ScatteringPointsGroup = ScatteringGroup.AddGroup("Scattering Points", FText::FromString("Points"));
    ScatteringPointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Size"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->Data.Scattering.Points.Num(); })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              EditedComponent->Data.Scattering.Points.SetNum(FMath::Max(
                                  META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    ScatteringPointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock).Text(FText::FromString("Frequency"))]
        .ValueContent()[SNew(STextBlock).Text(FText::FromString("Scattering"))];

    const TArray<FMetaXRAudioPoint>& ScatteringPoints = EditedComponent->Data.Scattering.Points;
    for (int32 PointIndex = 0; PointIndex < ScatteringPoints.Num(); ++PointIndex) {
      ScatteringPointsGroup.AddWidgetRow()
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda([EditedComponent, PointIndex]() {
                               if (PointIndex < EditedComponent->Data.Scattering.Points.Num()) {
                                 return EditedComponent->Data.Scattering.Points[PointIndex].Frequency;
                               } else {
                                 return 0.0f;
                               }
                             })
                             .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                               if (PointIndex < EditedComponent->Data.Scattering.Points.Num()) {
                                 EditedComponent->Modify();
                                 EditedComponent->Data.Scattering.Points[PointIndex].Frequency = FMath::Max(
                                     META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(FMath::RoundToZero(Value), META_XR_AUDIO_MAX_FREQUENCY));
                               }
                             })
                             .MinValue(TNumericLimits<float>::Lowest())
                             .MaxValue(TNumericLimits<float>::Max())
                             .Delta(1)]
          .ValueContent()[SNew(SNumericEntryBox<float>)
                              .Value_Lambda([EditedComponent, PointIndex]() {
                                if (PointIndex < EditedComponent->Data.Scattering.Points.Num()) {
                                  return EditedComponent->Data.Scattering.Points[PointIndex].Data;
                                } else {
                                  return 0.0f;
                                }
                              })
                              .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                                if (PointIndex < EditedComponent->Data.Scattering.Points.Num()) {
                                  EditedComponent->Modify();
                                  EditedComponent->Data.Scattering.Points[PointIndex].Data =
                                      FMath::Max(META_XR_AUDIO_MIN_SCATTERING, FMath::Min(Value, META_XR_AUDIO_MAX_SCATTERING));
                                }
                              })
                              .MinValue(TNumericLimits<float>::Lowest())
                              .MaxValue(TNumericLimits<float>::Max())
                              .Delta(1)]
          .ExtensionContent()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent, PointIndex]() {
            if (PointIndex < EditedComponent->Data.Scattering.Points.Num()) {
              EditedComponent->Data.Scattering.Points.RemoveAt(PointIndex);
            }
            Reset();
            return FReply::Handled();
          })];
    }
  }
}

void FMetaXRAcousticMaterialPropertiesDetails::Reset() {
  IDetailLayoutBuilder* LayoutBuilder = CachedDetailBuilder.Pin().Get();
  LayoutBuilder->ForceRefreshDetails();
}

#undef LOCTEXT_NAMESPACE
