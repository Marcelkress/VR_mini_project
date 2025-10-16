// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#include "MetaXRAcousticControlZoneDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "MetaXRAcousticControlZone.h"
#include "MetaXRAudioEditorInfo.h"
#include "Misc/EngineVersionComparison.h"
#include "PropertyCustomizationHelpers.h"
#include "SEnumCombo.h"
#include "SpectrumWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "Control Zone Details"

void FMetaXRAcousticControlZoneDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) {
  CachedDetailBuilder = DetailBuilder;
  CustomizeDetails(*DetailBuilder);
}

void FMetaXRAcousticControlZoneDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
  TArray<TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  if (OutObjects.Num() != 1) {
    return;
  }

  AMetaXRAcousticControlZone* EditedComponent = Cast<AMetaXRAcousticControlZone>(OutObjects[0].Get());

  if (EditedComponent) {
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(META_XR_AUDIO_DISPLAY_NAME, FText::GetEmpty(), ECategoryPriority::Important);

    TSharedRef<SSpectrumWidget> RT60Widget = SNew(SSpectrumWidget)
                                                 .Spectrum(&EditedComponent->RT60)
                                                 .GraphColor(FLinearColor(0.0f, 0.8f, 0.5f))
                                                 .Scale(EAxisScale::SquareCentered)
                                                 .RangeMin(META_XR_AUDIO_CONTROL_ZONE_MIN_RT60)
                                                 .RangeMax(META_XR_AUDIO_CONTROL_ZONE_MAX_RT60);

    TSharedRef<SSpectrumWidget> ReverbLevelWidget = SNew(SSpectrumWidget)
                                                        .Spectrum(&EditedComponent->ReverbLevel)
                                                        .GraphColor(FLinearColor(0.8f, 0.5f, 0.8f))
                                                        .Scale(EAxisScale::SquareCentered)
                                                        .RangeMin(META_XR_AUDIO_CONTROL_ZONE_MIN_REVERB)
                                                        .RangeMax(META_XR_AUDIO_CONTROL_ZONE_MAX_REVERB);

    // RT60
    IDetailGroup& RT60Group = Category.AddGroup("RT60 Group", FText::FromString("RT60"));
    RT60Group.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(120).WidthOverride(500)[RT60Widget]];

    IDetailGroup& RT60PointsGroup = RT60Group.AddGroup("RT60 Points", FText::FromString("Points"));
    RT60PointsGroup.AddWidgetRow()
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Size")).ToolTipText(FText::FromString("The number of points on the RT60 graph"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->RT60.Points.Num(); })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              EditedComponent->RT60.Points.SetNum(FMath::Max(
                                  META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              EditedComponent->ApplyProperties();
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    RT60PointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Frequency"))
                           .ToolTipText(FText::FromString("The frequency of a data point on the RT60 graph in Hz"))]
        .ValueContent()[SNew(STextBlock)
                            .Text(FText::FromString("RT60"))
                            .ToolTipText(FText::FromString("The RT60 value at this frequency in seconds"))];

    const TArray<FMetaXRAudioPoint>& RT60Points = EditedComponent->RT60.Points;
    for (int32 PointIndex = 0; PointIndex < RT60Points.Num(); ++PointIndex) {
      RT60PointsGroup.AddWidgetRow()
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda([EditedComponent, PointIndex]() { return EditedComponent->RT60.Points[PointIndex].Frequency; })
                             .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                               EditedComponent->Modify();
                               EditedComponent->RT60.Points[PointIndex].Frequency = FMath::Max(
                                   META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(FMath::RoundToZero(Value), META_XR_AUDIO_MAX_FREQUENCY));
                               EditedComponent->ApplyProperties();
                             })
                             .MinValue(TNumericLimits<float>::Lowest())
                             .MaxValue(TNumericLimits<float>::Max())
                             .Delta(1)]
          .ValueContent()[SNew(SNumericEntryBox<float>)
                              .Value_Lambda([EditedComponent, PointIndex]() { return EditedComponent->RT60.Points[PointIndex].Data; })
                              .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                                EditedComponent->Modify();
                                EditedComponent->RT60.Points[PointIndex].Data =
                                    FMath::Max(META_XR_AUDIO_CONTROL_ZONE_MIN_RT60, FMath::Min(Value, META_XR_AUDIO_CONTROL_ZONE_MAX_RT60));
                                EditedComponent->ApplyProperties();
                              })
                              .MinValue(TNumericLimits<float>::Lowest())
                              .MaxValue(TNumericLimits<float>::Max())
                              .Delta(1)]
          .ExtensionContent()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent, PointIndex]() {
            EditedComponent->Modify();
            EditedComponent->RT60.Points.RemoveAt(PointIndex);
            EditedComponent->ApplyProperties();
            Reset();
            return FReply::Handled();
          })];
    }

    // Reverb Level
    IDetailGroup& ReverbLevelGroup = Category.AddGroup("Reverb Level Group", FText::FromString("Reverb Level"));
    ReverbLevelGroup.AddWidgetRow().WholeRowContent()[SNew(SBox).HeightOverride(120).WidthOverride(500)[ReverbLevelWidget]];

    IDetailGroup& ReverbLevelPointsGroup = ReverbLevelGroup.AddGroup("Reverb Level Points", FText::FromString("Points"));
    ReverbLevelPointsGroup.AddWidgetRow()
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Size")).ToolTipText(FText::FromString("The number of points on the RT60 graph"))]
        .ValueContent()[SNew(SNumericEntryBox<int32>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->ReverbLevel.Points.Num(); })
                            .OnValueChanged_Lambda([this, EditedComponent](int32 Value) {
                              EditedComponent->Modify();
                              EditedComponent->ReverbLevel.Points.SetNum(FMath::Max(
                                  META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS, FMath::Min(Value, META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS)));
                              EditedComponent->ApplyProperties();
                              Reset();
                            })
                            .MinValue(TNumericLimits<int32>::Lowest())
                            .MaxValue(TNumericLimits<int32>::Max())];

    ReverbLevelPointsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Frequency"))
                           .ToolTipText(FText::FromString("The frequency of points on the RT60 graph in Hz"))]
        .ValueContent()[SNew(STextBlock)
                            .Text(FText::FromString("Reverb Levels"))
                            .ToolTipText(FText::FromString("The reverb level for a given frequency on the RT60 graph in Decibels"))];

    const TArray<FMetaXRAudioPoint>& ReverbLevelPoints = EditedComponent->ReverbLevel.Points;
    for (int32 PointIndex = 0; PointIndex < ReverbLevelPoints.Num(); ++PointIndex) {
      ReverbLevelPointsGroup.AddWidgetRow()
          .NameContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                             .MinFractionalDigits(0)
#endif
                             .Value_Lambda(
                                 [EditedComponent, PointIndex]() { return EditedComponent->ReverbLevel.Points[PointIndex].Frequency; })
                             .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                               EditedComponent->Modify();
                               EditedComponent->ReverbLevel.Points[PointIndex].Frequency = FMath::Max(
                                   META_XR_AUDIO_MIN_FREQUENCY, FMath::Min(FMath::RoundToZero(Value), META_XR_AUDIO_MAX_FREQUENCY));
                               EditedComponent->ApplyProperties();
                             })
                             .MinValue(TNumericLimits<float>::Lowest())
                             .MaxValue(TNumericLimits<float>::Max())
                             .Delta(1)]
          .ValueContent()[SNew(SNumericEntryBox<float>)
                              .Value_Lambda(
                                  [EditedComponent, PointIndex]() { return EditedComponent->ReverbLevel.Points[PointIndex].Data; })
                              .OnValueChanged_Lambda([EditedComponent, PointIndex](float Value) {
                                EditedComponent->Modify();
                                EditedComponent->ReverbLevel.Points[PointIndex].Data = FMath::Max(
                                    META_XR_AUDIO_CONTROL_ZONE_MIN_REVERB, FMath::Min(Value, META_XR_AUDIO_CONTROL_ZONE_MAX_REVERB));
                                EditedComponent->ApplyProperties();
                              })
                              .MinValue(TNumericLimits<float>::Lowest())
                              .MaxValue(TNumericLimits<float>::Max())
                              .Delta(1)]
          .ExtensionContent()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent, PointIndex]() {
            EditedComponent->Modify();
            EditedComponent->ReverbLevel.Points.RemoveAt(PointIndex);
            EditedComponent->ApplyProperties();
            Reset();
            return FReply::Handled();
          })];
    }
  }
}

void FMetaXRAcousticControlZoneDetails::Reset() {
  IDetailLayoutBuilder* LayoutBuilder = CachedDetailBuilder.Pin().Get();
  LayoutBuilder->ForceRefreshDetails();
}

#undef LOCTEXT_NAMESPACE
