// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAcousticGeometryDetails.h"

#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDesktopPlatform.h"
#include "IDetailGroup.h"
#include "MetaXRAcousticGeometry.h"
#include "MetaXRAcousticProjectSettings.h"
#include "MetaXRAudioEditorHelpers.h"
#include "MetaXRAudioEditorInfo.h"
#include "MetaXRAudioLogging.h"
#include "MetaXRAudioUtilities.h"
#include "Misc/EngineVersionComparison.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"

FString FMetaXRAcousticGeometryDetails::SaveFileDialog(AActor* Context) const {
  IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
  if (!DesktopPlatform) {
    return FString();
  }

  MetaXRAudioUtilities::CreateMetaXRAcousticContentDirectory(META_XR_AUDIO_DEFAULT_SAVE_FOLDER);

  FString OwnerName = "NewGeometry";
  if (Context != nullptr) {
    OwnerName = Context->GetActorLabel();
  }
  OwnerName += ".xrageo";

  const FString DialogTitle = TEXT("Save File As");
  const FString DefaultPath = FPaths::Combine(FPaths::ProjectContentDir(), META_XR_AUDIO_DEFAULT_SAVE_FOLDER);
  const FString DefaultFileName = OwnerName;
  const FString FileTypes = TEXT("xrageo (*.xrageo)|*.xrageo");
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

const FText FMetaXRAcousticGeometryDetails::GetCurrentFilePathText() const {
  return FilePathEditableTextBox->GetText();
}

void FMetaXRAcousticGeometryDetails::SetFilePathText(const FString& FilePath) {
  const FText FilePathText = FText::FromString(FilePath);
  FilePathEditableTextBox->SetText(FilePathText);
}

void FMetaXRAcousticGeometryDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) {
  CachedDetailBuilder = DetailBuilder; // Assignment does conversion to TWeakPtr.
  CustomizeDetails(*DetailBuilder);
}

void FMetaXRAcousticGeometryDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
  // Get a reference to the MetaXRAcousticGeometry being edited
  TArray<TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  if (OutObjects.Num() != 1) {
    return;
  }

  UMetaXRAcousticGeometry* EditedComponent = Cast<UMetaXRAcousticGeometry>(OutObjects[0].Get());
  if (!EditedComponent)
    return;

  // Add a checkbox to the category
  IDetailCategoryBuilder& Category =
      DetailBuilder.EditCategory(META_XR_AUDIO_DISPLAY_NAME, FText::GetEmpty(), ECategoryPriority::Important);

  Category.AddCustomRow(FText::FromString("Include Children"))
      .NameContent()[SNew(STextBlock)
                         .Text(FText::FromString("Include Child Meshes"))
                         .ToolTipText(FText::FromString("Include mesh data from all child static mesh components when baking"))]
      .ValueContent()[SNew(SCheckBox)
                          .IsChecked_Lambda([EditedComponent]() {
                            return EditedComponent->bIncludeChildren ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                          })
                          .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                            EditedComponent->Modify();
                            EditedComponent->bIncludeChildren = (State == ECheckBoxState::Checked);
                          })];

  IDetailGroup& AdvancedControlsGroup = Category.AddGroup("Advanced Controls", FText::FromString("Advanced Controls"));

  AdvancedControlsGroup.AddWidgetRow()
      .NameContent()[SNew(STextBlock)
                         .Text(FText::FromString("Use Physical Material Mapping"))
                         .ToolTipText(FText::FromString(
                             "Automatically choose Acoustic Materials during baking using each mesh's Physical Material"))]
      .ValueContent()[SNew(SCheckBox)
                          .IsChecked_Lambda([EditedComponent]() {
                            return EditedComponent->bUsePhysicalMaterials ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                          })
                          .OnCheckStateChanged_Lambda([EditedComponent](ECheckBoxState State) {
                            EditedComponent->Modify();
                            EditedComponent->bUsePhysicalMaterials = (State == ECheckBoxState::Checked);
                          })];

  // Mesh simplification controls
  IDetailGroup& MeshSimplificationControlsGroup =
      AdvancedControlsGroup.AddGroup("Mesh Simplification", FText::FromString("Mesh Simplification"));

  MeshSimplificationControlsGroup.AddWidgetRow()
      .NameContent()[SNew(STextBlock)
                         .Text(FText::FromString("Max Error"))
                         .ToolTipText(FText::FromString("Maximum tolerable mesh simplification error in centimeters"))]
      .ValueContent()[SNew(SSlider)
                          .MaxValue(100.0f)
                          .Value_Lambda([EditedComponent]() { return EditedComponent->MaxError; })
                          .OnValueChanged_Lambda([EditedComponent](float Value) {
                            EditedComponent->Modify();
                            EditedComponent->MaxError = Value;
                          })]
      .ExtensionContent()[SNew(SNumericEntryBox<float>)
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
                              .MaxFractionalDigits(3)
#endif
                              .Value_Lambda([EditedComponent]() { return EditedComponent->MaxError; })
                              .OnValueChanged_Lambda([EditedComponent](float Value) {
                                EditedComponent->Modify();
                                EditedComponent->MaxError = FMath::Max(FMath::Min(Value, 100.f), 0.f);
                              })];

  MeshSimplificationControlsGroup.AddWidgetRow()
      .NameContent()
          [SNew(STextBlock)
               .Text(FText::FromString("LOD"))
               .ToolTipText(FText::FromString(
                   "Which LOD to use for the acoustic geometry when using an LOD Group. The lowest value of 0 corresponds to the highest quality mesh."))]
      .ValueContent()[SNew(SSlider)
                          .Value_Lambda([EditedComponent]() { return static_cast<float>(EditedComponent->LOD) / 7.0f; })
                          .OnValueChanged_Lambda([EditedComponent](float Value) {
                            EditedComponent->Modify();
                            EditedComponent->LOD = static_cast<int32>(Value * 7.0f);
                          })]
      .ExtensionContent()[SNew(SNumericEntryBox<int32>)
                              .Value_Lambda([EditedComponent]() { return EditedComponent->LOD; })
                              .OnValueChanged_Lambda([EditedComponent](int32 Value) {
                                EditedComponent->Modify();
                                EditedComponent->LOD = FMath::Max(FMath::Min(Value, 7), 0);
                              })];

  this->FilePathEditableTextBox =
      SNew(SEditableTextBox)
          .OnKeyDownHandler_Lambda([this, EditedComponent](const FGeometry& Geo, const FKeyEvent& KeyEvent) -> FReply {
            if (KeyEvent.GetKey() != EKeys::Enter)
              return FReply::Handled();
            const FText CurrentText = this->GetCurrentFilePathText();
            this->HandleAcousticGeoFilePathChanged(EditedComponent, CurrentText);
            return FReply::Handled();
          })
          .IsReadOnly(false);

  this->FilePathEditableTextBox->SetText(FText::FromString(EditedComponent->FilePath));

  AdvancedControlsGroup.AddWidgetRow()
      .NameContent()[SNew(STextBlock)
                         .Text(FText::FromString("File Path"))
                         .ToolTipText(
                             FText::FromString("The path to the serialized mesh file, relative to the project's Content directory"))]
      .ValueContent()
          [SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1)[FilePathEditableTextBox.ToSharedRef()] +
           SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString("...")).OnClicked_Lambda([this, EditedComponent]() {
             FString NewPath = SaveFileDialog(EditedComponent->GetOwner());
             if (!NewPath.IsEmpty()) {
               EditedComponent->Modify();
               EditedComponent->FilePath = NewPath;
               SetFilePathText(NewPath);
             }
             return FReply::Handled();
           })]];

  if (EditedComponent->GetOwner() == nullptr) {
    Category.AddCustomRow(FText::FromString("Status"))
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Status")).ToolTipText(FText::FromString("The current state of this component"))]
        .ValueContent()[SNew(STextBlock)
                            .ColorAndOpacity_Lambda([this, EditedComponent]() { return FLinearColor::Yellow; })
                            .Text_Lambda([this, EditedComponent]() {
                              return FText::FromString("You can only bake the geometry component in the details pane.");
                            })];
    return;
  } else {
    Category.AddCustomRow(FText::FromString("Bake Mesh"))
        .ValueContent()[SNew(SButton)
                            .Text(FText::FromString(TEXT("Bake Mesh")))
                            .ToolTipText(FText::FromString("Precompute the geometry into a serialized format and write it to disk"))
                            .OnClicked_Lambda([this, EditedComponent] {
                              TArray<FString> GeoFileName = {FPaths::ProjectContentDir() / EditedComponent->GetFilePath()};
                              MetaXRAudioUtilities::CheckOutFilesInSourceControl(GeoFileName);
                              EditedComponent->WriteFile();
                              MetaXRAudioUtilities::CheckOutFilesInSourceControl(GeoFileName);
                              this->SetFilePathText(EditedComponent->FilePath);
                              return FReply::Handled();
                            })];
  }

  Category.AddCustomRow(FText::FromString("Vertex Count"))
      .NameContent()
          [SNew(STextBlock).Text(FText::FromString("Vertices")).ToolTipText(FText::FromString("The number of precomputed data points"))]
      .ValueContent()
      .HAlign(
          HAlign_Left)[SNew(STextBlock).Text_Lambda([EditedComponent]() { return FText::AsNumber(EditedComponent->GizmoVertices.Num()); })];

  Category.AddCustomRow(FText::FromString("File Size"))
      .NameContent()
          [SNew(STextBlock).Text(FText::FromString("Size")).ToolTipText(FText::FromString("The total size of the serialized data"))]
      .ValueContent()
      .HAlign(HAlign_Left)[SNew(STextBlock).Text_Lambda([EditedComponent]() {
        return FText::FromString(GetSizeString(EditedComponent->DataSize));
      })];

  if (EditedComponent->GetOwner() == nullptr) {
    Category.AddCustomRow(FText::FromString("Status"))
        .NameContent()
            [SNew(STextBlock).Text(FText::FromString("Status")).ToolTipText(FText::FromString("The current state of this Map file"))]
        .ValueContent()[SNew(STextBlock)
                            .ColorAndOpacity_Lambda([this, EditedComponent]() {
                              if (EditedComponent->NeedsRebake()) {
                                return FLinearColor::Yellow;
                              } else {
                                return FLinearColor::Gray;
                              }
                            })
                            .Text_Lambda([this, EditedComponent]() {
                              if (EditedComponent->NeedsRebake()) {
                                return FText::FromString("WARNING: Geometry file is stale, it needs to be rebaked!");
                              } else {
                                return FText::FromString("Geometry file is up to date");
                              }
                            })];
  }
}

void FMetaXRAcousticGeometryDetails::HandleAcousticGeoFilePathChanged(UMetaXRAcousticGeometry* AcousticGeo, const FText& NewText) {
  const FString& CurrentFilePath = AcousticGeo->FilePath;
  const FString NewTextStr = NewText.ToString();
  if (CurrentFilePath == NewTextStr)
    return;

  if (!UMetaXRAcousticGeometry::IsValidAcousticGeoFilePath(NewTextStr)) {
    const FText DialogMsg = FText::FromString("Invalid Path Specified.");
    const EAppReturnType::Type DialogReturnMsg = FMessageDialog::Open(EAppMsgType::Ok, DialogMsg);
    SetFilePathText(CurrentFilePath);
    return;
  }

  const FString DialogMsgStr =
      "Setting new path can result in previous file being unused.\nPrevious File Path: " + CurrentFilePath + "\nDo you want to continue?";
  const FText DialogMsg = FText::FromString(DialogMsgStr);
  // blocking call
  const EAppReturnType::Type DialogReturnMsg = FMessageDialog::Open(EAppMsgType::YesNo, DialogMsg);
  if (DialogReturnMsg == EAppReturnType::Yes)
    AcousticGeo->FilePath = NewTextStr;
  else
    SetFilePathText(CurrentFilePath);
}
