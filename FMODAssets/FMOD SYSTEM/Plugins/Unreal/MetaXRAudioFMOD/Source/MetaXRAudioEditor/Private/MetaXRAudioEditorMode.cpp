// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "MetaXRAudioEditorMode.h"
#include "BaseGizmos/TransformGizmoUtil.h"
#include "BaseGizmos/TransformProxy.h"
#include "EdModeInteractiveToolsContext.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"
#include "LevelInstance/LevelInstanceTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MetaXRAcousticMap.h"
#include "MetaXRAudioUtilities.h"
#include "Selection.h"

#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "Materials/MaterialRenderProxy.h"
#else
#include "MaterialShared.h"
#endif

const FEditorModeID FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId = TEXT("MetaXR Acoustic Map Editor");

FMetaXRAudioEditorMode::FMetaXRAudioEditorMode() {}

FMetaXRAudioEditorMode::~FMetaXRAudioEditorMode() {}

void FMetaXRAudioEditorMode::Enter() {
  FEditorModeTools* MyOwner = Owner;
  UModeManagerInteractiveToolsContext* ToolsContext = MyOwner->GetInteractiveToolsContext();
  TObjectPtr<UInteractiveGizmoManager> GizmoManager = ToolsContext->GizmoManager;

  UE::TransformGizmoUtil::RegisterTransformGizmoContextObject(ToolsContext);

  TransformProxy = NewObject<UTransformProxy>();
  TransformGizmo =
      UE::TransformGizmoUtil::CreateCustomRepositionableTransformGizmo(GizmoManager, ETransformGizmoSubElements::TranslateAllAxes, this);
  TransformGizmo->SetActiveTarget(TransformProxy, Owner->GetInteractiveToolsContext()->GizmoManager);
  TransformGizmo->SetVisibility(false);

  TransformProxy->OnTransformChanged.AddLambda([this](UTransformProxy* Proxy, FTransform Transform) {
    if (!EditedComponent || !EditedComponent->HasPoints() || !EditedComponent->PointIsSelected())
      return;

    EditedComponent->bHasCustomPoints = true;

    // The transform gizmo will provide a new position for the point in UE coordinates
    EditedComponent->MoveSelectedPoint(Transform.GetLocation());
  });
}

void FMetaXRAudioEditorMode::Exit() {
  EditedComponent = nullptr;
  Owner->GetInteractiveToolsContext()->GizmoManager->DestroyGizmo(TransformGizmo);
}

// This function makes sure all points match their gizmo transform counterparts and visibility is correctly set
void FMetaXRAudioEditorMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime) {
  // Get the selected objects when entering the editor mode
  TArray<UObject*> SelectedActors;
  UWorld* World = GEditor->GetEditorWorldContext().World();
  if (World) {
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr) {
      AActor* CurrentActor = *ActorItr;
      UMetaXRAcousticMap* FoundComponent = CurrentActor->FindComponentByClass<UMetaXRAcousticMap>();
      if (FoundComponent && FoundComponent != EditedComponent) {
        EditedComponent = FoundComponent;
        EditedComponent->ResetSelectedPoint();
        if (EditedComponent->HasPoints() && EditedComponent->PointIsSelected()) {
          // Here we want the transform applied and UE coordinates because the transform proxy will be visualized
          const FTransform Transform(FQuat::Identity, EditedComponent->GetSelectedPointUE(), FVector(1.0f));
          TransformProxy->SetTransform(Transform);
        }
        break;
      }
    }
  }

  bool bTransformGizmoVisible = false;
  if (EditedComponent) {
    bTransformGizmoVisible = EditedComponent ? (EditedComponent->bCustomPointsEnabled && EditedComponent->PointIsSelected()) : false;
  }

  TransformGizmo->SetVisibility(bTransformGizmoVisible);
}

// This function collects information about the location of a mouse press in our Map Editing mode
bool FMetaXRAudioEditorMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) {
  if (!EditedComponent)
    return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);

  if (Event == IE_Pressed) {
    if (Key == EKeys::LeftMouseButton) {
      FIntPoint MousePosition;
      Viewport->GetMousePos(MousePosition);
      return PerformRaycast(ViewportClient, MousePosition);
    }
  }

  return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

// This function compares the mouse click location to all existing Map points to see if any were selected
bool FMetaXRAudioEditorMode::PerformRaycast(FEditorViewportClient* ViewportClient, const FVector2D& MousePosition) {
  if (!ViewportClient)
    return false;

  if (FViewport* ActiveViewport = ViewportClient->Viewport) {
    FSceneViewFamilyContext ViewFamily(
        FSceneViewFamily::ConstructionValues(ActiveViewport, ViewportClient->GetScene(), ViewportClient->EngineShowFlags)
            .SetRealtimeUpdate(true));
    FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

    const FIntPoint ViewportSize = ActiveViewport->GetSizeXY();
    const FIntRect ViewRect = FIntRect(0, 0, ViewportSize.X, ViewportSize.Y);
    const FMatrix InvViewProjectionMatrix = View->ViewMatrices.GetInvViewProjectionMatrix();

    // Convert the 2D mouse click into a 3D position within the game
    FVector OutWorldPosition, OutWorldDirection;
    FSceneView::DeprojectScreenToWorld(MousePosition, ViewRect, InvViewProjectionMatrix, OutWorldPosition, OutWorldDirection);

    // Compare every Map Point to the mouse click to find out if any match
    for (int32 i = 0; i < EditedComponent->GetNumPoints(); i++) {
      // Here we want the transform applied and UE coordinates as this matches the mouse click specifications
      const FVector Point = EditedComponent->GetPointUE(i);

      FVector RayToSphere = Point - OutWorldPosition;
      float DotProduct = FVector::DotProduct(RayToSphere, OutWorldDirection);

      // Compute the closest point on the ray to the sphere center
      FVector ClosestPoint = OutWorldPosition + OutWorldDirection * DotProduct;

      // Check if the distance from the closest point to the sphere center is less than or equal to the sphere radius
      if (FVector::DistSquared(ClosestPoint, Point) <= META_XR_AUDIO_MAP_SPHERE_RADIUS * META_XR_AUDIO_MAP_SPHERE_RADIUS) {
        EditedComponent->SetSelectedPoint(i);

        FTransform Transform(FQuat::Identity, Point, FVector(1.0f));
        TransformGizmo->SetNewGizmoTransform(Transform);
        GEditor->SelectNone(true, true);
        GEditor->SelectActor(EditedComponent->GetOwner(), true, true);
        GEditor->SelectComponent(EditedComponent, true, true);

        return true;
      }
    }
  }

  return false;
}
