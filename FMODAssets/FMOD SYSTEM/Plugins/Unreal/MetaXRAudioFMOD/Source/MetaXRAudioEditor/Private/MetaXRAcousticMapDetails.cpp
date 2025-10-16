// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#include "MetaXRAcousticMapDetails.h"

#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorModeManager.h"
#include "IDetailGroup.h"
#include "MetaXRAcousticMap.h"
#include "MetaXRAcousticProjectSettings.h"
#include "MetaXRAudioEditorHelpers.h"
#include "MetaXRAudioEditorInfo.h"
#include "MetaXRAudioEditorMode.h"
#include "MetaXRAudioUtilities.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"

FString FMetaXRAcousticMapDetails::SaveFileDialog(AActor* Context) const {
  IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
  if (!DesktopPlatform) {
    return FString();
  }

  MetaXRAudioUtilities::CreateMetaXRAcousticContentDirectory(META_XR_AUDIO_DEFAULT_SAVE_FOLDER);

  FString OwnerName = "NewAcousticMap";
  if (Context != nullptr) {
    OwnerName = Context->GetActorLabel();
  }
  OwnerName += ".xramap";

  const FString DialogTitle = TEXT("Save File As");
  const FString DefaultPath = FPaths::Combine(FPaths::ProjectContentDir(), META_XR_AUDIO_DEFAULT_SAVE_FOLDER);
  const FString DefaultFileName = OwnerName;
  const FString FileTypes = TEXT("xramap (*.xramap)|*.xramap");
  TArray<FString> OutFilePath;

  bool bSaved = DesktopPlatform->SaveFileDialog(nullptr, DialogTitle, DefaultPath, DefaultFileName, FileTypes, 0, OutFilePath);

  if (bSaved) {
    // Create an empty file
    IFileManager& FileManager = IFileManager::Get();
    const FString SaveFilePath = OutFilePath[0];
    FString RelativeFilePath = SaveFilePath;
    FPaths::MakePathRelativeTo(RelativeFilePath, *DefaultPath);
    if (FArchive* Archive = FileManager.CreateFileWriter(*SaveFilePath)) {
      Archive->Flush();
      Archive->Close();
      return RelativeFilePath;
    }
  }

  return FString();
}

void FMetaXRAcousticMapDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
  TArray<TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  if (OutObjects.Num() != 1) {
    return;
  }

  UMetaXRAcousticMap* EditedComponent = Cast<UMetaXRAcousticMap>(OutObjects[0].Get());

  if (EditedComponent) {
    // Add a button to the details view
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(META_XR_AUDIO_DISPLAY_NAME, FText::GetEmpty(), ECategoryPriority::Important);

    if (EditedComponent->GetOwner() == nullptr) {
      Category.AddCustomRow(FText::FromString("Status"))
          .NameContent()
              [SNew(STextBlock).Text(FText::FromString("Status")).ToolTipText(FText::FromString("The current state of this component"))]
          .ValueContent()[SNew(STextBlock)
                              .ColorAndOpacity_Lambda([this, EditedComponent]() { return FLinearColor::Yellow; })
                              .Text_Lambda([this, EditedComponent]() {
                                return FText::FromString("You can only edit the map component in the details pane.");
                              })];
      return;
    }

    IDetailGroup& AdvancedControlsGroup = Category.AddGroup("Advanced Controls", FText::FromString("Advanced Controls"));
    AdvancedControlsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Static Only"))
                           .ToolTipText(FText::FromString("Only bake data for game objects marked as static when checked"))]
        .ValueContent()[SNew(SCheckBox)
                            .IsChecked_Lambda([EditedComponent]() {
                              return EditedComponent->bStaticOnly ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                            })
                            .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                              EditedComponent->Modify();
                              EditedComponent->bStaticOnly = (State == ECheckBoxState::Checked);
                            })];

    AdvancedControlsGroup.AddWidgetRow()
        .NameContent()
            [SNew(STextBlock)
                 .Text(FText::FromString("Edge Diffraction"))
                 .ToolTipText(FText::FromString(
                     "Precompute edge diffraction data for smooth occlusion. If disabled, a lower-quality but faster fallback diffraction will be used."))]
        .ValueContent()[SNew(SCheckBox)
                            .IsChecked_Lambda([EditedComponent]() {
                              return EditedComponent->bDiffraction ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                            })
                            .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                              EditedComponent->Modify();
                              EditedComponent->bDiffraction = (State == ECheckBoxState::Checked);
                            })];

    AdvancedControlsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Reflections"))
                           .ToolTipText(FText::FromString("The number of reflections generated for each point in the acoustic map"))]
        .ValueContent()[SNew(SSlider)
                            .Value_Lambda([EditedComponent]() { return static_cast<float>(EditedComponent->ReflectionCount) / 12.0f; })
                            .OnValueChanged_Lambda([EditedComponent](float Value) {
                              EditedComponent->Modify();
                              EditedComponent->ReflectionCount = static_cast<int32>(Value * 12);
                            })]
        .ExtensionContent()[SNew(SNumericEntryBox<int32>)
                                .Value_Lambda([EditedComponent]() { return EditedComponent->ReflectionCount; })
                                .OnValueChanged_Lambda([EditedComponent](int32 Value) {
                                  EditedComponent->Modify();
                                  EditedComponent->ReflectionCount = FMath::Max(FMath::Min(Value, 12), 0);
                                })];

    AdvancedControlsGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("File Path"))
                           .ToolTipText(
                               FText::FromString("The path to the serialized acoustic map, relative to the project's Content directory"))]
        .ValueContent()
            [SNew(SHorizontalBox) +
             SHorizontalBox::Slot().FillWidth(
                 1)[SNew(SEditableTextBox)
                        .Text_Lambda([EditedComponent]() { return FText::FromString(EditedComponent->FilePath); })
                        .IsReadOnly(true)] +
             SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent]() {
               FString NewPath = SaveFileDialog(EditedComponent->GetOwner());
               if (!NewPath.IsEmpty()) {
                 EditedComponent->Modify();
                 EditedComponent->FilePath = NewPath;
               }
               return FReply::Handled();
             })]

    ];

    // Add a slider for the integer property
    IDetailGroup& MappingConfigurationGroup = Category.AddGroup("Mapping Configuration", FText::FromString("Mapping Configuration"));

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("No Floating"))
                           .ToolTipText(
                               FText::FromString("Disables the creation of map data points for areas far above the environment's floor"))]
        .ValueContent()[SNew(SCheckBox)
                            .IsChecked_Lambda([EditedComponent]() {
                              return EditedComponent->bNoFloating ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                            })
                            .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                              EditedComponent->Modify();
                              EditedComponent->bNoFloating = (State == ECheckBoxState::Checked);
                            })];

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Min Spacing"))
                           .ToolTipText(FText::FromString("The size in centimeters of the smallest space that will be precomputed"))]
        .ValueContent()[SNew(SNumericEntryBox<float>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->MinSpacing; })
                            .OnValueChanged_Lambda([EditedComponent](float Value) {
                              EditedComponent->Modify();
                              EditedComponent->MinSpacing = FMath::Max(5.0f, FMath::Min(Value, EditedComponent->MaxSpacing));
                            })
                            .MinValue(TNumericLimits<float>::Lowest())
                            .MaxValue(TNumericLimits<float>::Max())
                            .Delta(1)];

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Max Spacing"))
                           .ToolTipText(FText::FromString("The maximum distance in centimeters between precomputed data points"))]
        .ValueContent()[SNew(SNumericEntryBox<float>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->MaxSpacing; })
                            .OnValueChanged_Lambda([EditedComponent](float Value) {
                              EditedComponent->Modify();
                              EditedComponent->MaxSpacing = FMath::Max(Value, EditedComponent->MinSpacing);
                            })
                            .MinValue(TNumericLimits<float>::Lowest())
                            .MaxValue(TNumericLimits<float>::Max())
                            .Delta(1)];

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Head Height"))
                           .ToolTipText(FText::FromString("The distance above the floor in centimeters where data points are placed"))]
        .ValueContent()[SNew(SNumericEntryBox<float>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->HeadHeight; })
                            .OnValueChanged_Lambda([EditedComponent](float Value) {
                              EditedComponent->Modify();
                              EditedComponent->HeadHeight = FMath::Max(0.0f, FMath::Min(Value, EditedComponent->MaxHeight));
                            })
                            .MinValue(TNumericLimits<float>::Lowest())
                            .MaxValue(TNumericLimits<float>::Max())
                            .Delta(1)];

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Max Height"))
                           .ToolTipText(
                               FText::FromString("The maximum height in centimeters above the floor where data points are placed"))]
        .ValueContent()[SNew(SNumericEntryBox<float>)
                            .Value_Lambda([EditedComponent]() { return EditedComponent->MaxHeight; })
                            .OnValueChanged_Lambda([EditedComponent](float Value) {
                              EditedComponent->Modify();
                              EditedComponent->MaxHeight = FMath::Max(EditedComponent->HeadHeight, Value);
                            })
                            .MinValue(TNumericLimits<float>::Lowest())
                            .MaxValue(TNumericLimits<float>::Max())
                            .Delta(1)];

    MappingConfigurationGroup.AddWidgetRow()
        .NameContent()[SNew(STextBlock)
                           .Text(FText::FromString("Custom Points"))
                           .ToolTipText(FText::FromString("Enable custom points to move and place points in the acoustic map manually"))]
        .ValueContent()
            [SNew(SCheckBox)
                 .IsChecked_Lambda([EditedComponent]() {
                   return EditedComponent->bCustomPointsEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                 })
                 .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                   if (State == ECheckBoxState::Checked) {
                     EditedComponent->Modify();
                     EditedComponent->bCustomPointsEnabled = true;
                   } else if (State == ECheckBoxState::Unchecked) {
                     EAppReturnType::Type Result = FMessageDialog::Open(
                         EAppMsgType::YesNo,
                         FText::FromString(
                             "Overwrite Custom Points?\nAre you sure you want to overwrite the custom points and re-map the scene?"));
                     if (Result == EAppReturnType::Yes) {
                       EditedComponent->Modify();
                       EditedComponent->bCustomPointsEnabled = false;
                       EditedComponent->Compute(true);
                     }
                   }
                 })];

    MappingConfigurationGroup.AddWidgetRow().ValueContent()
        [SNew(SButton)
             .ToolTipText(FText::FromString(
                 "Automatically place points in the scene.\nNOTE: this will clear any previously baked data or custom points"))
             .Text_Lambda([EditedComponent]() {
               if (EditedComponent->bHasCustomPoints)
                 return FText::FromString(TEXT("Remap Scene"));
               else
                 return FText::FromString(TEXT("Map Scene"));
             })
             .OnClicked_Lambda([EditedComponent] {
               if (EditedComponent->bHasCustomPoints) {
                 EAppReturnType::Type Result = FMessageDialog::Open(
                     EAppMsgType::YesNo, FText::FromString("Are you sure you want to overwrite the custom points and re-map the scene?"));
                 if (Result == EAppReturnType::Yes) {
                   EditedComponent->Compute(true);
                 }
               } else {
                 EditedComponent->Compute(true);
               }
               return FReply::Handled();
             })
             .IsEnabled_Lambda([EditedComponent]() { return EditedComponent->bCustomPointsEnabled; })];

    MappingConfigurationGroup.AddWidgetRow()
        .ValueContent()[SNew(SButton)
                            .ToolTipText(FText::FromString("Toggle editing of the points in the scene view."))
                            .Text_Lambda([EditedComponent]() {
                              FEditorModeTools& EdModeTools = GLevelEditorModeTools();
                              return FText::FromString(
                                  EdModeTools.IsModeActive(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId)
                                      ? TEXT("Lock Point Editing")
                                      : TEXT("Unlock Point Editing"));
                            })
                            .OnClicked_Lambda([EditedComponent] {
                              FEditorModeTools& EdModeTools = GLevelEditorModeTools();
                              if (!EdModeTools.IsModeActive(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId)) {
                                EdModeTools.ActivateMode(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId);
                                EditedComponent->ResetSelectedPoint();
                              } else {
                                EdModeTools.ActivateDefaultMode();
                                EditedComponent->ResetSelectedPoint();
                              }
                              return FReply::Handled();
                            })
                            .IsEnabled_Lambda([EditedComponent]() { return EditedComponent->bCustomPointsEnabled; })];

    TSharedPtr<SHorizontalBox> PointEditBox;
    MappingConfigurationGroup.AddWidgetRow().ValueContent()[SAssignNew(PointEditBox, SHorizontalBox)];
    PointEditBox->AddSlot().AutoWidth()
        [SNew(SButton)
             .Text(FText::FromString(TEXT("Add Point")))
             .ToolTipText(FText::FromString(
                 "Add a new point at the center of the current view.\nTo quickly create points by left mouse button, enable editing in the viewport."))
             .OnClicked_Lambda([EditedComponent] {
               // Get information about the viewport camera to help decide where to place the new point
               FEditorViewportClient* ViewportClient = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
               FVector EditorCameraDirection = ViewportClient->GetViewRotation().Vector();
               FVector EditorCameraPosition = ViewportClient->GetViewLocation();

               EditedComponent->Modify();
               EditedComponent->bHasCustomPoints = true;
               EditedComponent->AddPoint(EditedComponent->GetNewPointForRay(EditorCameraPosition, EditorCameraDirection));
               EditedComponent->SetSelectedPoint(EditedComponent->GetNumPoints() - 1);

               FPropertyChangedEvent PropertyChangedEvent(
                   UMetaXRAcousticMap::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMetaXRAcousticMap, PointsOVR)));
               EditedComponent->PostEditChangeProperty(PropertyChangedEvent);
               return FReply::Handled();
             })
             .IsEnabled_Lambda([EditedComponent]() {
               FEditorModeTools& EdModeTools = GLevelEditorModeTools();
               return EdModeTools.IsModeActive(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId) &&
                   EditedComponent->bCustomPointsEnabled;
             })];

    PointEditBox->AddSlot().AutoWidth()
        [SNew(SButton)
             .Text(FText::FromString(TEXT("Remove Point")))
             .ToolTipText(FText::FromString(
                 "Remove the currently-selected point.\nTo quickly delete points by pressing backspace, enable editing in the viewport"))
             .OnClicked_Lambda([EditedComponent] {
               if (EditedComponent->PointIsSelected()) {
                 EditedComponent->Modify();
                 EditedComponent->bHasCustomPoints = true;
                 EditedComponent->RemoveSelectedPoint();
               }
               return FReply::Handled();
             })
             .IsEnabled_Lambda([EditedComponent]() {
               FEditorModeTools& EdModeTools = GLevelEditorModeTools();
               return EdModeTools.IsModeActive(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId) &&
                   EditedComponent->bCustomPointsEnabled && EditedComponent->PointIsSelected();
             })];

    Category.AddCustomRow(FText::FromString("Bake Acoustics"))
        .ValueContent()[SNew(SButton)
                            .Text_Lambda([EditedComponent]() {
                              if (EditedComponent->bComputing && !EditedComponent->bComputeCanceled) {
                                return FText::FromString("Cancel");
                              } else {
                                return FText::FromString(TEXT("Bake Acoustics"));
                              }
                            })
                            .OnClicked_Lambda([EditedComponent] {
                              if (EditedComponent->bComputing && !EditedComponent->bComputeCanceled) {
                                EditedComponent->CancelCompute();
                              } else {
                                EditedComponent->Compute(false);
                              }
                              return FReply::Handled();
                            })
                            .IsEnabled_Lambda([EditedComponent]() {
                              // The compute can only be cancelled once the simulation has begun
                              return !EditedComponent->bComputing || (EditedComponent->GetDescription().Compare("Simulating") == 0);
                            })
                            .ToolTipText(FText::FromString("Simulate the scene, and also map it if custom points are not provided."))];

    Category.AddCustomRow(FText::FromString("Bake Status"))
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Status")).ToolTipText(FText::FromString("The current state of the precomputed data"))]
        .ValueContent()[SNew(STextBlock)
                            .Text_Lambda([EditedComponent]() {
                              if (EditedComponent->bComputing) {
                                float Progress = EditedComponent->GetComputeProgress();
                                float timeRemaining = Progress <= 0.0f
                                    ? 0.0f
                                    : EditedComponent->GetComputeTimeSeconds() / Progress - EditedComponent->GetComputeTimeSeconds();
                                FString ETAString = "ETA: " + FString::SanitizeFloat(static_cast<int>(timeRemaining)) + " Seconds";
                                FString ProgressPercent = FString::SanitizeFloat(static_cast<int>(Progress * 100.0f)) + "%";
                                FString ProgressStatus = "Computing: " + ProgressPercent + " " + ETAString;
                                if (EditedComponent->GetDescription().Compare("Simulating")) {
                                  ProgressStatus = "Preparing " + EditedComponent->GetDescription() + " " + ProgressPercent;
                                }
                                return FText::FromString(ProgressStatus);
                              }
                              switch (EditedComponent->Status) {
                                case EAcousticMapStatus::Empty:
                                  return FText::FromString("EMPTY");
                                case EAcousticMapStatus::Mapped:
                                  return FText::FromString("MAPPED");
                                case EAcousticMapStatus::Ready:
                                  return FText::FromString("READY");
                              }
                              return FText::GetEmpty();
                            })
                            .ColorAndOpacity_Lambda([this, EditedComponent]() {
                              if (EditedComponent->bComputing) {
                                return FLinearColor::Yellow;
                              } else {
                                return FLinearColor::Gray;
                              }
                            })];

    Category.AddCustomRow(FText::FromString("Points"))
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Points")).ToolTipText(FText::FromString("The number of precomputed data points"))]
        .ValueContent()[SNew(STextBlock).Text_Lambda([EditedComponent]() {
          if (EditedComponent->bHasCustomPoints)
            return FText::Format(FText::FromString("{0} (Custom)"), FText::AsNumber(EditedComponent->GetNumPoints()));
          else
            return FText::AsNumber(EditedComponent->GetNumPoints());
        })];

    Category.AddCustomRow(FText::FromString("Data Size"))
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Data Size")).ToolTipText(FText::FromString("The total size of the serialized data"))]
        .ValueContent()[SNew(STextBlock).Text_Lambda([EditedComponent]() {
          return FText::FromString(GetSizeString(EditedComponent->DataSize));
        })];
  }
}
