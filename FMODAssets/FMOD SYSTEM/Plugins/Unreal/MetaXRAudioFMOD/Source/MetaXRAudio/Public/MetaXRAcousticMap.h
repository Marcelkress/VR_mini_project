// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#pragma once

#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DebugRenderSceneProxy.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticMap.generated.h"

// Fwd declare
class UMetaXRAcousticMaterial;
class UMetaXRAcousticGeometry;

#define META_XR_AUDIO_MAP_SPHERE_RADIUS 25.0f
#define META_XR_AUDIO_MAP_SPHERE_NUM_SLICES 8
#define META_XR_AUDIO_MAP_SPHERE_NUM_STACKS 8

/*
 * MetaXRAudio geometry components are used to customize an a static mesh actor's acoustic properties.
 */
UENUM(BlueprintType)
enum class EAcousticMapStatus : uint8 {
  Empty = 0 UMETA(DisplayName = "Empty"),
  Mapped = (1 << 0) UMETA(DisplayName = "Mapped"),
  Ready = (1 << 1) | Mapped UMETA(DisplayName = "Ready"),
};

class FAsyncSceneMappingTask : public FNonAbandonableTask {
 public:
  UMetaXRAcousticMap* AcousticMapComponent;
  ovrAudioSceneIR Map;
  ovrAudioSceneIRParameters Parameters;
  bool bMapOnly = false;

  FAsyncSceneMappingTask(UMetaXRAcousticMap* InMapComponent, ovrAudioSceneIR InMap, ovrAudioSceneIRParameters InParameters)
      : AcousticMapComponent(InMapComponent), Map(InMap), Parameters(InParameters) {}

  FAsyncSceneMappingTask(FAsyncSceneMappingTask& Other)
      : AcousticMapComponent(Other.AcousticMapComponent), Map(Other.Map), Parameters(Other.Parameters) {}

  void DoWork();

  FORCEINLINE TStatId GetStatId() const {
    RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncSceneMappingTask, STATGROUP_ThreadPoolAsyncTasks);
  }
};

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Map",
         ToolTip = "Precompute information about the acoustics into an Acoustic Map to reduce resource usage"))
class METAXRAUDIO_API UMetaXRAcousticMap : public UPrimitiveComponent {
  GENERATED_BODY()

 public:
  UMetaXRAcousticMap();
  virtual ~UMetaXRAcousticMap() override;

  // The path to the serialized acoustic map, relative to the project's Content directory
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  FString FilePath;

  // Only bake data for game objects marked as static when checked
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bStaticOnly = false;

  // Disables the creation of map data points for areas far above the environment's floor
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bNoFloating = true;

  // Precompute edge diffraction data for smooth occlusion. If disabled, a lower-quality but faster fallback diffraction will be used.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bDiffraction = true;

  // The size in centimeters of the smallest space that will be precomputed
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  float MinSpacing = 100.0f;

  // The maximum distance in centimeters between precomputed data points
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  float MaxSpacing = 1000.0f;

  // The distance above the floor in centimeters where data points are placed
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  float HeadHeight = 150.0f;

  // The maximum height in centimeters above the floor where data points are placed
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  float MaxHeight = 300.0f;

  // \brief The gravity vector indicates the direction which would match the direction of gravity within the game
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  FVector GravityVector = FVector(0, 0, -1.0f); // Default gravity vector

  // The number of reflections generated for each point in the acoustic map
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  int32 ReflectionCount = 6;

  // Indicates the user has selected to allow the use of manually placed points
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bCustomPointsEnabled = false;

  // Indicates that user has added at least one manually placed point
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bHasCustomPoints = false;

  FString GetFilePath() {
    return FilePath;
  }
  void LoadData();
  void StartInternal(bool AutoLoad = true);
  void DestroyInternal();
  virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

#if WITH_EDITOR
  bool Compute(bool bMapOnly, bool bBlockingCompute = false);
  void FinishCompute();
  void CancelCompute();
  void AddPoint(FVector NewPointUE);
  FVector GetNewPointForRay(FVector EditorCameraPosition, FVector EditorCameraDirection);
  void UpdateCachedPoints();
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
  bool IsComputeCanceled() {
    return bComputeCanceled;
  }
  void SetDescription(FString NewDescription) {
    Description = NewDescription;
  }
  void SetComputeProgress(float NewComputeProgress) {
    ComputeProgress = NewComputeProgress;
  }
  void SetComputeTimeSeconds(float NewComputeTime) {
    ComputeTime = NewComputeTime;
  }
  FString GetDescription() {
    return Description;
  }
  float GetComputeProgress() {
    return ComputeProgress;
  }
  float GetComputeTimeSeconds() {
    return ComputeTime;
  }
  void StartTimer() {
    StartTimeSeconds = FPlatformTime::Seconds();
  }
  double CheckTimer() {
    return FPlatformTime::Seconds() - StartTimeSeconds;
  }
  void SetStageStartingTimeSeconds(double NewTime) {
    StageStartingTimeSeconds = NewTime;
  }
  double GetStageStartingTimeSeconds() {
    return StageStartingTimeSeconds;
  }
  virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
#endif

 private:
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void BeginDestroy() override;
  void CheckMapTransformValid();
  void ApplyTransform();

  ovrAudioSceneIR CachedMap = nullptr;
  ovrAudioSceneIRParameters MapParameters;
  FTransform PreviousTransform;

#if WITH_EDITOR
  virtual void PostEditComponentMove(bool bFinished) override;
  virtual void Activate(bool bReset = false) override;
  virtual void Deactivate() override;
  TSharedPtr<FAsyncTask<FAsyncSceneMappingTask>> MappingTask;
  void CheckIfOnlyMapInLevel();
  void GenerateFileNameIfEmpty();
  void GatherGeometriesAndMaterials();

  // Functions to manage the gizmo points
  void SetupSphereVertices();
  int GetNumPoints() const;
  void RemovePoint(int SelectedPoint);
  void RemoveSelectedPoint();
  void MoveSelectedPoint(FVector NewLocationUE);
  void SetPoints(TArray<float> NewPointsOVR, int NumPoints);
  FVector GetPointUE(int SelectedPoint) const;
  TArray<FVector> GetPointsUE() const;
  FVector GetSelectedPointUE() const;
  void SetSelectedPoint(int SelectedPoint);
  void ResetSelectedPoint();
  bool PointIsSelected() const;
  bool HasPoints() const;
  void RefreshSphereGizmo();
  // Functions to manage the gizmo points

  FString Hash;
  bool bComputing = false;
  bool bComputeFinished = false;
  bool bComputeCanceled = false;
  bool bComputeSucceeded = false;
  FString Description;
  float ComputeProgress;
  float ComputeTime;
  double StartTimeSeconds;
  double StageStartingTimeSeconds;
  int32 DataSize;
  EAcousticMapStatus Status;
  TArray<UMetaXRAcousticGeometry*> Geometries;
  TArray<UMetaXRAcousticMaterial*> Materials;

  // Stores all the information about the acoustic map points in OVR coordinate system
  TArray<FVector> PointsOVR;

  // Points remapped into a set of vertices to render spheres at the points position
  TArray<FDynamicMeshVertex> GizmoVertices;
  TArray<uint32_t> GizmoIndices;
  TArray<uint32_t> GizmoSelectedIndices;
  int32 SelectedPointIndex = MAX_int32;

  // Store precomputed sphere vertices in these arrays (not adjusted to a specific point)
  TArray<FVector> SphereVertices;
  TArray<uint32_t> SphereIndices;
#endif

  friend class FMetaXRAcousticMapDetails;
  friend class FAsyncSceneMappingTask;
  friend class FMetaXRAudioEditorMode;
  friend class FMetaXRAcousticMapSceneProxy;
};

#if WITH_EDITOR
class FMetaXRAcousticMapSceneProxy : public FDebugRenderSceneProxy {
 public:
  FMetaXRAcousticMapSceneProxy(const UPrimitiveComponent* InComponent);
  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
  virtual void GetDynamicMeshElements(
      const TArray<const FSceneView*>& Views,
      const FSceneViewFamily& ViewFamily,
      uint32 VisibilityMap,
      FMeshElementCollector& Collector) const override;
  virtual bool CanBeOccluded() const override {
    return false;
  }

 private:
  const UMetaXRAcousticMap* AcousticMapComponent = nullptr;
};
#endif
