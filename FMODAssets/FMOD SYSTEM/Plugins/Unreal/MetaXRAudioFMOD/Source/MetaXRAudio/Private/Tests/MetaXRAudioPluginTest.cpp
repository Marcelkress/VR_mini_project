// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioPlatform.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_DynamicRoom.h"
#include "MetaXR_Audio_Propagation.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaXRAudio, All, All);

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
#include "Tests/AutomationEditorCommon.h"
#endif

// Returns false on fail
#define OVR_AUDIO_TEST(Result, Context)                                                      \
  UE_LOG(LogMetaXRAudio, Log, TEXT("Testing " #Result));                                     \
  if (Result != ovrSuccess) {                                                                \
    const TCHAR* ErrString = GetMetaXRErrorString(Result);                                   \
    UE_LOG(LogMetaXRAudio, Error, TEXT("Meta XR Audio SDK Unit Test Fail - %s"), ErrString); \
    IsTestSuccessful = false;                                                                \
  }

#define OVR_GETTER_TEST(Variable, ExpectedValue)                             \
  if (Variable != ExpectedValue) {                                           \
    UE_LOG(LogMetaXRAudio, Error, TEXT("##Variable has unexpected value.")); \
    IsTestSuccessful = false;                                                \
  }

#if defined(META_NATIVE_UNREAL_PLUGIN)
/*
 * ------------------ Dependencies test--------------------------
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAllDependenciesTest,
    "MetaXRAudio.DependenciesCheck",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ProductFilter)

bool FAllDependenciesTest::RunTest(const FString& Parameters) {
  TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();

  int32 PluginIndex =
      EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("MetaXRAudio"); });

  if (PluginIndex == INDEX_NONE) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Unable to locate dependency directory because MetaXRAudio plugin not enabled."));
  }

  FString PluginDir = EnabledPlugins[PluginIndex]->GetBaseDir();

  // Windows DLL
  if (!FPaths::FileExists(PluginDir / "Binaries" / "Win64" / "metaxraudio64.dll")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find metaxraudio64.dll"));
    return false;
  }

  // Android 64
  if (!FPaths::FileExists(
          PluginDir / "Source" / "MetaXRAudio" / "Private" / "LibMetaXRAudio" / "lib" / "arm64-v8a" / "libmetaxraudio64.so")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find libmetaxraudio64.so"));
    return false;
  }

  // Android 32
  if (!FPaths::FileExists(
          PluginDir / "Source" / "MetaXRAudio" / "Private" / "LibMetaXRAudio" / "lib" / "armeabi-v7a" / "libmetaxraudio32.so")) {
    UE_LOG(LogMetaXRAudio, Error, TEXT("Could not find libmetaxraudio32.so"));
    return false;
  }

  return true;
}
#endif // defined(META_NATIVE_UNREAL_PLUGIN)

/*
 * ------------------ API test--------------------------
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCallsTest,
    "MetaXRAudio.ApiCalls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ProductFilter)

static uint32_t MockProgress(void* UserData, const char* String, float Progress) {
  return (uint32_t) true;
};

bool FApiCallsTest::RunTest(const FString& Parameters) {
  bool IsTestSuccessful = true;
  ovrAudioContext Context;

  // Create and initialize context
  {
    ovrAudioContextConfiguration Config;
    Config.acc_Size = sizeof(Config);
    Config.acc_SampleRate = 48000;
    Config.acc_BufferLength = 512;
    Config.acc_MaxNumSources = 16;

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateContext)(&Context, &Config), Context)
  }

  // Feature enable
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_LateReverberation, 1), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_LateReverberation, 0), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_SimpleRoomModeling, 1), Context);
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_Enable)(Context, ovrAudioEnable_SimpleRoomModeling, 0), Context);

  // Unit Scale
  {
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetUnitScale)(Context, 0.01f), Context);
    float UnitScale;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_GetUnitScale)(Context, &UnitScale), Context);
    OVR_GETTER_TEST(UnitScale, 0.01f);
  }

  // Box parameters
  {
    ovrAudioAdvancedBoxRoomParameters BoxParameters{};
    BoxParameters.abrp_Size = sizeof(ovrAudioAdvancedBoxRoomParameters);
    BoxParameters.abrp_Depth = 1;
    BoxParameters.abrp_Height = 1;
    BoxParameters.abrp_Width = 1;

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAdvancedBoxRoomParameters)(Context, &BoxParameters), Context);
  }

  // Clutter Factor
  {
    ovrAudioBands ClutterBands = {1.0f, 1.0f, 1.0f, 1.0f};
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetRoomClutterFactor)(Context, ClutterBands), Context);
  }

  // Listener vectors
  {
    float PosX = 0;
    float PosY = 0;
    float PosZ = 0;
    float FwdX = 1;
    float FwdY = 0;
    float FwdZ = 0;
    float UpX = 0;
    float UpY = 0;
    float UpZ = 1;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetListenerVectors)(Context, PosX, PosY, PosZ, FwdX, FwdY, FwdZ, UpX, UpY, UpZ), Context);
  }

  {
    int Sound = 0;
    float PosX = 0;
    float PosY = 0;
    float PosZ = 0;
    float FwdX = 1;
    float FwdY = 0;
    float FwdZ = 0;
    float UpX = 0;
    float UpY = 0;
    float UpZ = 1;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceVectors)(Context, Sound, PosX, PosY, PosZ, FwdX, FwdY, FwdZ, UpX, UpY, UpZ), Context);
  }
  // Source Directivity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSourceDirectivityEnabled)(Context, 0, 1), Context);

  // Source Directivity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSourceDirectivityIntensity)(Context, 0, 1), Context);

  // Audio Source Radius
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceRadius)(Context, 0, 1.0f), Context);

  // HRTF Intensity
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioHrtfIntensity)(Context, 0, 1.0f), Context);

  // Reverb Reach
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceReverbReach)(Context, 0, 1.0f), Context);

  // Reflection Send Levels
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioReflectionsSendLevel)(Context, 0, 1.0f), Context);

  // Reverb Send Levels
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioReverbSendLevel)(Context, 0, 1.0f), Context);

  // Audio Source Flags
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAudioSourceFlags)(Context, 0, ovrAudioSourceFlag_ReflectionsDisabled), Context);

  // Mix-In Shared Reverb
  {
    uint32_t Status = 0;
    float Interleaved[2];
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_MixInSharedReverbInterleaved)(Context, &Status, Interleaved), Context);
  }

  // Shared Reverb Wet Level

  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetSharedReverbWetLevel)(Context, 1.0f), Context);

  // Head Radius
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetHeadRadius)(Context, 1.0f), Context);

  // Ambisonics
  {
    ovrAudioAmbisonicStream Stream;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateAmbisonicStream)(Context, 1, 1, ovrAudioAmbisonicFormat_AmbiX, 1, &Stream), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_SetAmbisonicOrientation)(Stream, 1, 0, 0, 0, 0, 1), Context);

    float InData[1];
    float OutData[1];
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_ProcessAmbisonicStreamInterleaved)(Context, Stream, InData, OutData, 1), Context);
  }

  // ------------Propagation----------------
  // Audio Geometry & Material
  ovrAudioGeometry Geometry = nullptr;
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateAudioGeometry)(Context, &Geometry), Context);
  {
    ovrAudioMesh Mesh = {};
    ovrAudioMeshVertices ovrVertices = {};

    float Vertices[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    ovrVertices.vertices = Vertices;
    ovrVertices.vertexCount = 9;
    ovrVertices.vertexType = ovrAudioScalarType_Float32;
    Mesh.vertices = ovrVertices;

    ovrAudioMeshIndices ovrIndices = {};
    uint32 Indices[3] = {1, 2, 3};
    ovrIndices.indices = Indices;
    ovrIndices.indexCount = 3;
    ovrIndices.indexType = ovrAudioScalarType_UInt32;
    Mesh.indices = ovrIndices;

    ovrAudioMaterial ovrMaterial = nullptr;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateAudioMaterial)(Context, &ovrMaterial), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 63.5f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 125.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 250.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 500.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 1000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 2000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 4000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, 8000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 63.5f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 125.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 250.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 500.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 1000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 2000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 4000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, 8000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 63.5f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 125.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 250.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 500.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 1000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 2000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 4000.0f, 0), Context);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, 8000.0f, 0), Context);

    ovrAudioMeshGroup MeshGroup = {};
    MeshGroup.faceCount = 1;
    MeshGroup.faceType = ovrAudioFaceType_Triangles;
    MeshGroup.material = ovrMaterial;
    Mesh.groups = &MeshGroup;
    Mesh.groupCount = 1;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryUploadMesh)(Geometry, &Mesh), Context);

    const float IdMatrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometrySetTransform)(Geometry, IdMatrix), Context);

    FString Path = FPaths::ProjectDir() / "TestMesh.ovrmesh";
    // Delete test file if it already exists
    IFileManager& FM = IFileManager::Get();
    if (FPaths::ValidatePath(Path) && FPaths::FileExists(Path)) {
      FM.Delete(*Path);
    }

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryWriteMeshFile)(Geometry, TCHAR_TO_ANSI(*Path)), Context);

    TArray<uint8> Data;
    if (!FFileHelper::LoadFileToArray(Data, *Path)) {
      UE_LOG(LogMetaXRAudio, Error, TEXT("Failed to load audio geometry file: %s"), *Path);
      return false;
    }
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryReadMeshMemory)(Geometry, (const int8_t*)Data.GetData(), Data.Num()), Context);

    // Delete test file after done with unit tests
    if (FPaths::ValidatePath(Path) && FPaths::FileExists(Path)) {
      FM.Delete(*Path);
    }

    uint32_t NumVertices;
    uint32_t NumTriangles;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryGetSimplifiedMesh)(Geometry, nullptr, &NumVertices, nullptr, &NumTriangles), Context);
    TArray<float> SimplifiedVertices;
    SimplifiedVertices.SetNum(NumVertices * 3);
    TArray<uint32> SimplifiedIndices;
    SimplifiedIndices.SetNum(NumTriangles);
    OVR_AUDIO_TEST(
        OVRA_CALL(ovrAudio_AudioGeometryGetSimplifiedMesh)(
            Geometry, SimplifiedVertices.GetData(), &NumVertices, SimplifiedIndices.GetData(), &NumTriangles),
        Context);

    struct TestSerializer {
      static size_t Read(void* userData, void* bytes, size_t byteCount) {
        TestSerializer* Serializer = static_cast<TestSerializer*>(userData);
        UE_LOG(LogMetaXRAudio, Log, TEXT("Reading stored mesh data: %i bytes"), byteCount);
        if (Serializer->Pos + byteCount <= Serializer->Data.Num()) {
          FMemory::Memcpy(bytes, Serializer->Data.GetData() + Serializer->Pos, byteCount);
          Serializer->Pos += byteCount;

          return byteCount;
        } else {
          UE_LOG(LogMetaXRAudio, Error, TEXT("Tried to read more than what is stored (%i bytes)"), Serializer->Data.Num());
          return 0;
        }
      }
      static size_t Write(void* userData, const void* bytes, size_t byteCount) {
        TestSerializer* Serializer = static_cast<TestSerializer*>(userData);
        int AfterWriteSize = Serializer->Pos + byteCount;
        UE_LOG(LogMetaXRAudio, Log, TEXT("Writing mesh data: %i bytes"), byteCount);
        if (AfterWriteSize > Serializer->Data.Num()) {
          UE_LOG(LogMetaXRAudio, Log, TEXT("Expanding data TArray size from %i to %i bytes"), Serializer->Data.Num(), AfterWriteSize);
          Serializer->Data.SetNum(AfterWriteSize);
        }
        FMemory::Memcpy(Serializer->Data.GetData() + Serializer->Pos, bytes, byteCount);
        Serializer->Pos += byteCount;
        return byteCount;
      }
      static int64_t Seek(void* userData, int64_t seekOffset) {
        TestSerializer* Serializer = static_cast<TestSerializer*>(userData);
        Serializer->Pos = seekOffset <= Serializer->Data.Num() ? seekOffset : Serializer->Data.Num();
        return Serializer->Pos;
      }

      TArray<uint8> Data;
      size_t Pos = 0;
    };

    // Serialization
    ovrAudioSerializer AudioSerializer;
    AudioSerializer.read = TestSerializer::Read;
    AudioSerializer.write = TestSerializer::Write;
    AudioSerializer.seek = TestSerializer::Seek;

    TestSerializer Serializer;
    AudioSerializer.userData = &Serializer;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryWriteMeshData)(Geometry, &AudioSerializer), Context);

    Serializer.Pos = 0;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioGeometryReadMeshData)(Geometry, &AudioSerializer), Context);

    ovrAudioMeshSimplification Simplification;
    Simplification.thisSize = sizeof(ovrAudioMeshSimplification);
    Simplification.flags = ovrAudioMeshFlags_enableMeshSimplification;
    Simplification.unitScale = 1;
    Simplification.maxError = 0.1f;
    Simplification.minDiffractionEdgeAngle = 0.0f;
    Simplification.minDiffractionEdgeLength = 0.01f;
    Simplification.flagLength = 1.0f;
#if WITH_EDITOR
    Simplification.threadCount = 0; // Use as many threads as CPUs
#else
    Simplification.threadCount = 1;
#endif

    OVR_AUDIO_TEST(
        OVRA_CALL(ovrAudio_AudioGeometryUploadSimplifiedMeshArrays)(
            Geometry,
            SimplifiedVertices.GetData(),
            0,
            SimplifiedVertices.Num() / 3,
            sizeof(float) * 3,
            ovrAudioScalarType_Float32,
            SimplifiedIndices.GetData(),
            0,
            SimplifiedIndices.Num(),
            ovrAudioScalarType_UInt32,
            &MeshGroup,
            1,
            &Simplification),
        Context);
  }

  // Scene IR
  {
    ovrAudioSceneIR SceneIR = nullptr;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_CreateAudioSceneIR)(Context, &SceneIR), Context);
    ovrAudioSceneIRStatus Status = ovrAudioSceneIRStatus_Empty;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioSceneIRGetStatus)(SceneIR, &Status), Context);

    ovrAudioSceneIRParameters IRParams{};
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_InitializeAudioSceneIRParameters)(&IRParams), Context);

    IRParams.minResolution = 1.f;
    IRParams.maxResolution = 10.f;
    IRParams.maxHeight = 3.f;
    IRParams.gravityVector[0] = 0.f;
    IRParams.gravityVector[1] = -1.f;
    IRParams.gravityVector[2] = 0.f;
    IRParams.reflectionCount = 4;
    IRParams.callbacks.userData = this;
    IRParams.callbacks.progress = MockProgress;
    IRParams.thisSize = sizeof(ovrAudioSceneIRParameters);

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioSceneIRCompute)(SceneIR, &IRParams), Context);

    size_t PointCount;
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioSceneIRGetPointCount)(SceneIR, &PointCount), Context);

    TArray<float> Points;
    Points.SetNum(PointCount * 3);
    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_AudioSceneIRGetPoints)(SceneIR, Points.GetData(), PointCount), Context);

    OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_DestroyAudioSceneIR)(SceneIR), Context);
  }

  // Destroy Context & Geometry
  OVR_AUDIO_TEST(OVRA_CALL(ovrAudio_DestroyAudioGeometry)(Geometry), Context);

  OVRA_CALL(ovrAudio_DestroyContext)(Context);

  return IsTestSuccessful;
}
#endif // WITH_DEV_AUTOMATION_TESTS
