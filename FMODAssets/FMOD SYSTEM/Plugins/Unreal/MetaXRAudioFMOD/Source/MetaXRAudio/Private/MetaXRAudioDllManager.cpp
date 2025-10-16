// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioRoomAcousticProperties.h"

#ifdef META_WWISE_UNREAL_PLUGIN
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"
#endif // META_WWISE_UNREAL_PLUGIN

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"

// forward decleration should match OAP_Globals
extern "C" ovrResult ovrAudio_GetPluginContext(ovrAudioContext* context);
extern "C" ovrResult ovrAudio_SetFMODPluginUnitScale(float unitScale);
#ifdef META_WWISE_UNREAL_PLUGIN
extern "C" int WwiseEndpoint_SetListenerTransform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float rotW);
#endif // META_WWISE_UNREAL_PLUGIN

FMetaXRAudioLibraryManager& FMetaXRAudioLibraryManager::Get() {
  static FMetaXRAudioLibraryManager* sInstance;
  if (sInstance == nullptr) {
    sInstance = new FMetaXRAudioLibraryManager();
  }

  // Because the constructor of FMetaXRAudioLibraryManager is empty, expect we can only fail here if out of memory
  if (sInstance == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio: Failed to get or create Meta XR Audio library manager"));
  }
  check(sInstance != nullptr);
  return *sInstance;
}

FMetaXRAudioLibraryManager::FMetaXRAudioLibraryManager()
    : MetaXRAudioDllHandle(nullptr), NumInstances(0), bInitialized(false), CachedPluginContext(nullptr) {}

FMetaXRAudioLibraryManager::~FMetaXRAudioLibraryManager() {
  FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void FMetaXRAudioLibraryManager::Initialize() {
  if (NumInstances == 0) {
    if (!LoadDll()) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio: Failed to load OVR Audio dll"));
      return;
    }
  }

  NumInstances++;
  if (!bInitialized) {
    // Check the version number
    int32 MajorVersionNumber = -1;
    int32 MinorVersionNumber = -1;
    int32 PatchNumber = -1;

    const char* OvrVersionString = OVRA_CALL(ovrAudio_GetVersion)(&MajorVersionNumber, &MinorVersionNumber, &PatchNumber);
    if (MajorVersionNumber != OVR_AUDIO_MAJOR_VERSION || MinorVersionNumber != OVR_AUDIO_MINOR_VERSION) {
      UE_LOG(
          LogAudio,
          Warning,
          TEXT("Meta XR Audio: Using mismatched OVR Audio SDK Version! %d.%d vs. %d.%d"),
          OVR_AUDIO_MAJOR_VERSION,
          OVR_AUDIO_MINOR_VERSION,
          MajorVersionNumber,
          MinorVersionNumber);
      return;
    }
    bInitialized = true;
  }
}

void FMetaXRAudioLibraryManager::Shutdown() {
  if (NumInstances == 0) {
    // This means we failed to load the OVR Audio module during initialization and there's nothing to shutdown.
    return;
  }

  NumInstances--;

  if (NumInstances == 0) {
    // Shutdown OVR audio
    ReleaseDll();
    bInitialized = false;
  }
}

const FString GetUENativePluginName() {
#if PLATFORM_WINDOWS
  return (sizeof(void*) == 4) ? FString("metaxraudio32.dll") : FString("metaxraudio64.dll");
#elif PLATFORM_ANDROID
  return (sizeof(void*) == 4) ? FString("libmetaxraudio32.so") : FString("libmetaxraudio64.so");
#else
  return FString();
#endif
}

// Below functions only meant to run on windows...
#if PLATFORM_WINDOWS
const bool GetMetaXRWwisePluginPath(const TArray<TSharedRef<IPlugin>>& EnabledPlugins, FString& OutMetaXRWWisePluginPath) {
  const TCHAR WWISE_DLL_NAME_WITH_EXT[] = TEXT("MetaXRAudioWwise.dll");
  OutMetaXRWWisePluginPath.Empty();

  // Even IF MetaXRAudioWwise.dll exists, we still need to exit if Wwise plugin not enabled...
  const int32 WwiseIndex =
      EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("Wwise"); });
  if (WwiseIndex == INDEX_NONE)
    return false;

  IFileManager& FileMan = IFileManager::Get();

#ifdef METAXR_WWISE_PLUGIN_PATH
  const FString TempMetaXRWWisePluginPath = FString(METAXR_WWISE_PLUGIN_PATH);
  const bool IsValidWwisePluginDLLPath =
      TempMetaXRWWisePluginPath.EndsWith(WWISE_DLL_NAME_WITH_EXT) && FileMan.FileExists(*TempMetaXRWWisePluginPath);
  if (IsValidWwisePluginDLLPath) {
    OutMetaXRWWisePluginPath = TempMetaXRWWisePluginPath;
    return true;
  }
  UE_LOG(
      LogAudio,
      Error,
      TEXT(
          "[FMetaXRAudioLibraryManager] METAXR_WWISE_PLUGIN_PATH is defined, but its value does not point to a valid path to MetaXRAudioWwise.dll. Must use fallback method to find DLL..."));
#endif

  // The user could have integrated various versions all build with different Windows SDK, so we do a general search for the binary.
  const FString WwisePluginPath = EnabledPlugins[WwiseIndex]->GetBaseDir();
  TArray<FString> FilePaths;
  FileMan.FindFilesRecursive(FilePaths, *WwisePluginPath, WWISE_DLL_NAME_WITH_EXT, true, false, true);

  if (FilePaths.Num() > 1) {
    UE_LOG(LogAudio, Warning, TEXT("Meta XR Audio: Multiple versions of Meta's Wwise plugin found:"));
    // Here we default to the last plugin seen that contains x64 in its path.
    FString DefaultDLL;
    for (FString& FilePath : FilePaths) {
      UE_LOG(LogAudio, Warning, TEXT("\t%s"), *FilePath);
      if (FilePath.Contains("x64"))
        DefaultDLL = FilePath;
    }

    if (DefaultDLL.IsEmpty()) {
      DefaultDLL = FilePaths[0];
      UE_LOG(LogAudio, Warning, TEXT("Defaulting to the first plugin seen: %s"), *DefaultDLL);
    } else {
      UE_LOG(LogAudio, Warning, TEXT("Defaulting to the LAST plugin containing x64 in its path: %s"), *DefaultDLL);
    }

    OutMetaXRWWisePluginPath = DefaultDLL;
    return true;
  }

  if (FilePaths.Num() > 0) {
    OutMetaXRWWisePluginPath = FilePaths[0];
    return true;
  }

  return false;
}

const bool GetMetaXRFmodPluginPath(const TArray<TSharedRef<IPlugin>>& EnabledPlugins, FString& OutMetaXRFmodPluginPath) {
  const TCHAR FMOD_DLL_NAME_WITH_EXT[] = TEXT("MetaXRAudioFMOD.dll");
  OutMetaXRFmodPluginPath.Empty();

  const int32 FMODIndex =
      EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("FMODStudio"); });
  if (FMODIndex == INDEX_NONE)
    return false;

  const FString FmodWindowsBinariesPath = EnabledPlugins[FMODIndex]->GetBaseDir() / "Binaries" / "Win64" / FMOD_DLL_NAME_WITH_EXT;
  if (FPaths::FileExists(*FmodWindowsBinariesPath)) {
    OutMetaXRFmodPluginPath = FmodWindowsBinariesPath;
    return true;
  }

  return false;
}

const bool GetMetaXRNativePluginPath(const TArray<TSharedRef<IPlugin>>& EnabledPlugins, FString& OutMetaXRNativePluginPath) {
  OutMetaXRNativePluginPath.Empty();
  const FString UE_DLL_NAME = GetUENativePluginName();
  const int32 UnrealNativeIndex =
      EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("MetaXRAudio"); });
  if (UnrealNativeIndex == INDEX_NONE)
    return false;

  const FString MetaXRNativeBinary = EnabledPlugins[UnrealNativeIndex]->GetBaseDir() / "Binaries" / "Win64" / UE_DLL_NAME;
  if (FPaths::FileExists(*MetaXRNativeBinary)) {
    OutMetaXRNativePluginPath = MetaXRNativeBinary;
    return true;
  }

  return false;
}

// This is the main function that chooses which MetaXR plugin integration. We go in this order:
// Wwise -> Fmod -> Native (we only attempt to find next integration IF we failed to find current integration plugin)
const bool SelectMetaXRPluginIntegrationRoutine(FString& OutMetaXRPluginPath) {
  const TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();
  if (GetMetaXRWwisePluginPath(EnabledPlugins, OutMetaXRPluginPath)) {
    UE_LOG(
        LogAudio,
        Display,
        TEXT(
            "Meta XR Audio: MetaXRAudioWwise.dll found in Wwise Thirdparty plugin directory, using the Wwise version of the Meta XR Audio UE integration"));
    return true;
  } else if (GetMetaXRFmodPluginPath(EnabledPlugins, OutMetaXRPluginPath)) {
    UE_LOG(
        LogAudio,
        Display,
        TEXT("Meta XR Audio: MetaXRAudioFMOD.dll found in Engine Plugins, using the FMOD version of the Meta XR Audio UE integration"));
    return true;
  } else if (GetMetaXRNativePluginPath(EnabledPlugins, OutMetaXRPluginPath)) {
    UE_LOG(LogAudio, Display, TEXT("Meta XR Audio: Found native UE AudioMixer."));
    return true;
  }

  UE_LOG(
      LogAudio,
      Error,
      TEXT(
          "Meta XR Audio: Unable to find Meta XR Audio DLL in any of the expected integration folders. Make sure the binary exists and if in a non-standard location, edit the logice of the function MetaXRAudioDLLManager::LoadDll() accordingly."));
  return false;
}

const bool TryLoadWindowsDLL(const FString& FullDllPath, void*& DllHandle) {
  IFileManager& FileMan = IFileManager::Get();

  if (!FullDllPath.EndsWith(".dll") || !FileMan.FileExists(*FullDllPath))
    return false;

  const FString DllPath = FPaths::GetPath(FullDllPath);
  FPlatformProcess::PushDllDirectory(*DllPath);
  DllHandle = FPlatformProcess::GetDllHandle(*FullDllPath);
  FPlatformProcess::PopDllDirectory(*DllPath);
  return DllHandle != nullptr;
}
#endif

#if PLATFORM_ANDROID
const bool SelectLoadPluginIntegrationRoutine(void*& DllHandle) {
  const FString WwiseAndroidPluginName("libMetaXRAudioWwise.so");
  const FString FmodAndroidPluginName("libMetaXRAudioFMOD.so");
  const FString NativeAndroidPlugin = GetUENativePluginName();

  if (DllHandle != nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio Internal error. Non-Null DllHandle passed into: SelectLoadPluginIntegrationRoutine"));
    return false;
  }

  DllHandle = FPlatformProcess::GetDllHandle(*WwiseAndroidPluginName);
  if (DllHandle != nullptr) {
    UE_LOG(
        LogAudio,
        Display,
        TEXT("Meta XR Audio: %s found, using the Wwise version of the Meta XR Audio UE integration"),
        *WwiseAndroidPluginName);
    return true;
  }
  DllHandle = FPlatformProcess::GetDllHandle(*FmodAndroidPluginName);
  if (DllHandle != nullptr) {
    UE_LOG(
        LogAudio,
        Display,
        TEXT("Meta XR Audio: %s found, using the FMOD version of the Meta XR Audio UE integration"),
        *FmodAndroidPluginName);
    return true;
  }
  DllHandle = FPlatformProcess::GetDllHandle(*NativeAndroidPlugin);

  if (DllHandle != nullptr) {
    UE_LOG(
        LogAudio,
        Display,
        TEXT("Meta XR Audio: Middleware plugins not found, %s found, assuming native UE AudioMixer"),
        *NativeAndroidPlugin);
    return true;
  }

  UE_LOG(
      LogAudio,
      Error,
      TEXT(
          "Meta XR Audio: Unable to load any Meta XR Audio UE integratiton plugins.Make sure you have one (and only one) of the plugin integrations enabled (Wwise, Fmod, or MetaXRAudio).Also, make sure you have moved the plugin integration into their corresponding folders. See Meta docs for more details."));
  return false;
}
#endif

bool FMetaXRAudioLibraryManager::LoadDll() {
  if (MetaXRAudioDllHandle == nullptr) {
#if PLATFORM_WINDOWS
    FString SelectedIntegrationPluginPath;
    if (!SelectMetaXRPluginIntegrationRoutine(SelectedIntegrationPluginPath)) {
      UE_LOG(
          LogAudio,
          Error,
          TEXT(
              "Meta XR Audio: Unable to find Meta XR Audio DLL in any of the expected integration folders. Make sure the binary exists and if in a non-standard location, edit the logice of the function MetaXRAudioDLLManager::LoadDll() accordingly."));
      return false;
    }

    void* TempMetaXrAudioDllHandle = nullptr;
    if (!TryLoadWindowsDLL(SelectedIntegrationPluginPath, TempMetaXrAudioDllHandle)) {
      UE_LOG(
          LogAudio,
          Error,
          TEXT("[FMetaXRAudioLibraryManager] Unable to Load MetaXR integration plugin. Check this path: %s"),
          *SelectedIntegrationPluginPath);
      return false;
    }

    MetaXRAudioDllHandle = TempMetaXrAudioDllHandle;

#elif PLATFORM_ANDROID
    void* TempDllHandle = nullptr;
    if (!SelectLoadPluginIntegrationRoutine(TempDllHandle)) {
      UE_LOG(LogAudio, Error, TEXT("[FMetaXRAudioLibraryManager] Failed to load android metaxr integration plugin..."));
      return false;
    }

    MetaXRAudioDllHandle = TempDllHandle;
#endif

    return (MetaXRAudioDllHandle != nullptr);
  }
  return true;
}

void FMetaXRAudioLibraryManager::ReleaseDll() {
#if PLATFORM_WINDOWS
  if (NumInstances == 0 && MetaXRAudioDllHandle) {
    FPlatformProcess::FreeDllHandle(MetaXRAudioDllHandle);
    MetaXRAudioDllHandle = nullptr;
  }
#endif
}

bool FMetaXRAudioLibraryManager::UpdatePluginContext(float DeltaTime) {
  QUICK_SCOPE_CYCLE_COUNTER(STAT_FMetaXRAudioLibraryManager_UpdatePluginContext);

  ovrAudioContext Context = GetPluginContext();

#ifdef META_WWISE_UNREAL_PLUGIN
  // Wwise provides positional information as listener-relative. We pass information here to convert to world-relative
  if (GEngine) {
    TArray<APlayerController*> players;
    GEngine->GetAllLocalPlayerControllers(players);
    if (!players.IsEmpty()) {
      FVector DeviceRight = FVector::RightVector;
      FVector DeviceFront = FVector::ForwardVector;
      FVector DevicePosition = FVector::ZeroVector;
      FQuat UEQuaternion;
      players[0]->GetAudioListenerPosition(DevicePosition, DeviceFront, DeviceRight);
      UEQuaternion = players[0]->GetControlRotation().Quaternion();
      // This function requests data in Wwise convention (left handed, x is right, y is up, z is forward)
      // Unreal convention is (x:forward, y:right, z:up)
      auto WwiseEndpoint_SetListenerTransformFunction = OVRA_CALL(WwiseEndpoint_SetListenerTransform);
      if (WwiseEndpoint_SetListenerTransformFunction != nullptr) {
        int result = WwiseEndpoint_SetListenerTransformFunction(
            (float)DevicePosition.Y,
            (float)DevicePosition.Z,
            (float)DevicePosition.X,
            (float)UEQuaternion.Y,
            (float)UEQuaternion.Z,
            (float)UEQuaternion.X,
            (float)UEQuaternion.W);
      }
    }
  }
#endif // META_WWISE_UNREAL_PLUGIN

  UMetaXRAudioRoomAcousticProperties* RoomAcoustics = UMetaXRAudioRoomAcousticProperties::GetActiveRoomAcoustics();
  if (RoomAcoustics && Context != nullptr) {
    RoomAcoustics->UpdateRoomModel(Context);
  }

  return true;
}

ovrAudioContext FMetaXRAudioLibraryManager::GetPluginContext() {
  if (CachedPluginContext == nullptr) {
    auto GetPluginContext = OVRA_CALL(ovrAudio_GetPluginContext);
    if (GetPluginContext != nullptr) {
      OVR_AUDIO_CHECK(GetPluginContext(&CachedPluginContext), "Failed to get Meta XR Audio context");

      // FMOD or Wwise plugins need to be informed about the unit scale (which is centimeters for UE)
      auto SetUnitScale = OVRA_CALL(ovrAudio_SetUnitScale);
      if (SetUnitScale != nullptr) {
        OVR_AUDIO_CHECK(SetUnitScale(CachedPluginContext, 0.01f), "Failed to set unit scale");
      }

      // Tick the scene from here since there is no listener
      auto TickDelegate = FTickerDelegate::CreateRaw(this, &FMetaXRAudioLibraryManager::UpdatePluginContext);
      TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
    }
  }
  return CachedPluginContext;
}
