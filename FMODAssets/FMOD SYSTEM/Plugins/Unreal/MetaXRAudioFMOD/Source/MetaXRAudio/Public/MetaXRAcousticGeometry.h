// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DebugRenderSceneProxy.h"
#include "LandscapeInfo.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticGeometry.generated.h"

// Fwd declare
class UMetaXRAcousticMaterialProperties;
class UMetaXRAcousticGeometry;

/*
 * MetaXRAudio geometry components are used to customize an a static mesh actor's acoustic properties.
 */
typedef uint32_t MetaXRAudioMeshFlags;

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Geometry",
         ToolTip = "Analyze a mesh to generate acoustics, occlusion, and diffraction"))
class METAXRAUDIO_API UMetaXRAcousticGeometry : public UPrimitiveComponent {
  GENERATED_BODY()

 public:
  UMetaXRAcousticGeometry();
  ~UMetaXRAcousticGeometry();

  // if IncludeChildren is true, children (attached) meshes will be merged
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bIncludeChildren = true;

  // Maximum tolerable mesh simplification error in centimeters
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  float MaxError = 10.0f;

  // Which LOD to use for the acoustic geometry when using an LOD Group. The lowest value of 0 corresponds to the highest quality mesh.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  int32 LOD = 0;

  // The path to the serialized mesh file, relative to the project's Content directory
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  FString FilePath;

  // If enabled the geometry file will be read from disk instead of computing the geometry each time at startup
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bFileEnabled = true;

  // Flags that indicate how the geometry mesh should be simplified to create an acoustic mesh
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  int32 MeshFlags = ovrAudioMeshFlags_enableMeshSimplification;

  // Automatically choose Acoustic Materials during baking using each mesh's Physical Material
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  bool bUsePhysicalMaterials;

  /// \brief A hash code of the game objects contributing to the current serialized acoustic geometry, for detecting changes.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Acoustics")
  FString HierarchyHash;

#if WITH_EDITOR
  TArray<FDynamicMeshVertex> GizmoVertices;
  TMap<UMetaXRAcousticMaterialProperties*, TArray<uint32>> GizmoMaterialIndices;

  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
  bool NeedsRebake() const {
    return bNeedsRebake;
  }
  static const bool IsValidAcousticGeoFilePath(const FString& FilePath);
#endif
#if WITH_EDITORONLY_DATA
  void OnObjectSelected(UObject* Object);
#endif
  bool CreatePropagationGeometry();
  bool UploadGeometry();
  bool StartInternal();
  bool DestroyInternal();
  bool ReadFile();
  bool WriteFile();
  FString ComputeHash();
  bool WriteFileInternal(ovrAudioGeometry GeometryHandle);
  bool IncludesChildren() const {
    return bIncludeChildren;
  }
  bool DiffractionEnabled() const {
    return MeshFlags & ovrAudioMeshFlags_enableDiffraction;
  }
  void SetDiffractionEnabled(bool bEnabled) {
    if (bEnabled)
      MeshFlags |= ovrAudioMeshFlags_enableDiffraction;
    else
      MeshFlags &= ~ovrAudioMeshFlags_enableDiffraction;
  }
  bool IsFileEnabled() const {
    return bFileEnabled;
  }
  ovrAudioGeometry GetHandle() const {
    return OvrGeometry;
  }
  FString GetFilePath() const {
    return FilePath;
  }

  virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
  virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

  // Structures
  struct FMeshMaterial {
    UStaticMeshComponent* StaticMesh = nullptr;
    int32 LOD = 0;
    TArray<UMetaXRAcousticMaterialProperties*> Materials;

    const bool IsInstanced() const;
    const int32 GetInstancedCount() const;
    // Gets the static mesh matrix relative to UMetaXRAcousticGeometry.
    // InstanceID = -1 means NON-instanced mesh matrix. Meaning, if the static mesh is non-instanced call with InstanceID = -1
    // IF the mesh is instanced call with InstanceID >= 0. 0 will get the matrix of the first instance, 1 will get matrix of second
    // instance, etc... False return value means failure to get the matrix (likely because caller used InstanceID >= 0 on NON-instanced mesh
    // OR InstanceID was invalid).
    const bool GetAcousticMeshLocalMatrix(const FMatrix& AcousticGeoCompWorldMatrix, const int32 InstanceID, FMatrix& OutLocalMatrix) const;
  };

  struct FLandscapeMaterial {
    ULandscapeInfo* LandscapeInfo = nullptr;
    TArray<UMetaXRAcousticMaterialProperties*> Materials;
  };

  // Define visitor class skeleton and declare the implementations
  class ITransformVisitor {
   public:
    virtual ~ITransformVisitor(){};
    virtual TArray<UMetaXRAcousticMaterialProperties*> Visit(
        AActor* Transform,
        const TArray<UMetaXRAcousticMaterialProperties*>* UserData) = 0;
  };

  class FAgeChecker : public ITransformVisitor {
   public:
    FAgeChecker(FDateTime TimeStamp, bool bUsePhysicalMaterials);
    TArray<UMetaXRAcousticMaterialProperties*> Visit(AActor* Transform, const TArray<UMetaXRAcousticMaterialProperties*>* UserData)
        override;
    bool CheckAssetTime(const UObject* Asset);

    FDateTime TimeStamp;
    bool bUsePhysicalMaterials;
    bool bIsOlder = false;
    FString Hash;
  };

  class FHashAppender : public ITransformVisitor {
   public:
    FHashAppender(FString Hash, bool bUsePhysicalMaterials);
    TArray<UMetaXRAcousticMaterialProperties*> Visit(AActor* Transform, const TArray<UMetaXRAcousticMaterialProperties*>* UserData)
        override;
    FString Hash;
    bool bUsePhysicalMaterials;
  };

  class FMeshGatherer : public ITransformVisitor {
   public:
    FMeshGatherer(bool IgnoreStatic, bool UsePhysicalMaterials, int LODSelection = 0);

    TArray<UMetaXRAcousticMaterialProperties*> Visit(AActor* Transform, const TArray<UMetaXRAcousticMaterialProperties*>* UserData)
        override;
    void CollectTerrains(AActor* Actor, const TArray<UMetaXRAcousticMaterialProperties*>& MaterialsToApply);

    TArray<UMetaXRAcousticGeometry::FLandscapeMaterial> GetTerrains() const {
      return Terrains;
    }
    TArray<UMetaXRAcousticGeometry::FMeshMaterial> GetMeshes() const {
      return Meshes;
    }

    int LodSelection = 0;
    int IgnoredMeshCount = 0;
    bool bIgnoreStatic;
    bool bUsePhysicalMaterials = false;
    TArray<UMetaXRAcousticGeometry::FLandscapeMaterial> Terrains;
    TArray<UMetaXRAcousticGeometry::FMeshMaterial> Meshes;
  };

 private:
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void PostLoad() override;
  virtual void BeginDestroy() override;
  virtual void DestroyComponent(bool bPromoteChildren) override;
  virtual void Activate(bool bReset = false) override;
  virtual void Deactivate() override;

  // Hierarchy Traversal
  static void TraverseMeshHierarchy(
      AActor* Actor,
      const bool IncludeChildren,
      const bool bParentWasExcluded,
      ITransformVisitor& Visitor,
      const TArray<UMetaXRAcousticMaterialProperties*>* ParentData = nullptr);

#if WITH_EDITOR
  void UpdateGizmoMesh();
  void UpdateGizmoMesh(ovrAudioGeometry GeometryHandle);
  void GenerateFileNameIfEmpty();
  void RefreshNeedsRebaked();
  void MapGizmoMaterials(FMeshGatherer* Gatherer = nullptr);
#endif

  bool DestroyPropagationGeometry();
  bool UploadMesh(ovrAudioGeometry GeometryHandle);
  bool UploadMesh(ovrAudioGeometry GeometryHandle, AActor* Owner, bool IgnoreStatic, int& OutIgnoredMeshCount);
  void ApplyTransform();
  void LoadGeometryAsync();
  bool IsStatic();
  void CheckGeoTransformValid();

  int32 DataSize;
  ovrAudioGeometry OvrGeometry;
  ovrAudioContext CachedContext;
  FTransform PreviousTransform;
  ovrAudioGeometry PreviousGeometry;
  TArray<FString> ExcludeTags;
#if WITH_EDITOR
  TArray<UMetaXRAcousticMaterialProperties*> GizmoMaterialMapping;
  bool HasUpdatedGizmoMaterialMapOnStartup = false;
  bool bNeedsRebake;
#endif // WITH_EDITOR

  friend class FMetaXRAcousticGeometryDetails;
};

class FMetaXRAcousticGeometrySceneProxy : public FDebugRenderSceneProxy {
 public:
  FMetaXRAcousticGeometrySceneProxy(const UPrimitiveComponent* InComponent);
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
  const UMetaXRAcousticGeometry* GeometryComponent = nullptr;
};
