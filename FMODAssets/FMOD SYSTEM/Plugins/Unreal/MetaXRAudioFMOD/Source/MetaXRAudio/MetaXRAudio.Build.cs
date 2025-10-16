// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace UnrealBuildTool.Rules
{
    public class MetaXRAudio : ModuleRules
    {
        public MetaXRAudio(ReadOnlyTargetRules Target) : base(Target)
        {
            AddCommonDependencies();

            string WwisePluginDir = System.IO.Path.Combine(PluginDirectory, "..", "Wwise");
            if (!System.IO.Directory.Exists(WwisePluginDir))
                return;

            if (GetMetaXRWisePluginPath(WwisePluginDir, Target, out string MetaWwisePluginPath))
            {
                PublicDefinitions.Add($"METAXR_WWISE_PLUGIN_PATH=\"{MetaWwisePluginPath}\"");
            }
        }

        public void AddCommonDependencies()
        {
            PrivateIncludePathModuleNames.AddRange(new string[] { "TargetPlatform" });

            PrivateIncludePaths.AddRange(
                new string[]
                {
                    "MetaXRAudio/Private",
                    "MetaXRAudio/Private/LibMetaXRAudio/include",
                    System.IO.Path.Combine(GetModuleDirectory("AudioMixer"), "Private")
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "AudioExtensions",
                    "AudioMixer",
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Landscape",
                    "PhysicsCore",
                    "Projects",
                    "SignalProcessing",
                    "SoundFieldRendering",
                    "RenderCore",
                    "UMG",
                    "Json"
                }
             );

            ReadOnlyTargetRules TargetRules = Target;
            UnrealTargetPlatform CurrentPlatform = TargetRules.Platform;

            if (CurrentPlatform == UnrealTargetPlatform.Win64)
            {
                // Automatically copy DLL to packaged builds
                RuntimeDependencies.Add(System.IO.Path.Combine(PluginDirectory, "Binaries", "Win64", "metaxraudio64.dll"));
                AddEngineThirdPartyPrivateStaticDependencies(TargetRules, "DX11Audio");
            }

            if (CurrentPlatform == UnrealTargetPlatform.Android)
            {
                AdditionalPropertiesForReceipt.Add("AndroidPlugin", System.IO.Path.Combine(PluginDirectory, "Source", "MetaXRAudio", "MetaXRAudio_APL.xml"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(PluginDirectory, "Source", "MetaXRAudio", "Private", "LibMetaXRAudio", "lib", "arm64-v8a", "libmetaxraudio64.so"));
            }

            if (TargetRules.bBuildEditor)
            {
                PrivateDependencyModuleNames.Add("SourceControl");
                PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }

        // Retreives path to MetaXR Wwise plugin (IF it exists).
        private static bool GetMetaXRWisePluginPath(string WwisePluginDir, ReadOnlyTargetRules Target, out string MetaWwisePluginPath)
        {
            // Below we use reflection to grab WwiseSoundEngineVersion and WwiseUEPlatform
            // This logic exists like this to avoid mentioning any wwise classes directly.
            // We do this to avoid creating a direct dependency on Wwise classes.
            // IF user of plugin is using FMod or native integration, they will likely not have these wwise classes in their project.
            // Therefore, we avoid build system compiler errors IF Wwise is not installed in same VS project as MetaXR plugin.

            MetaWwisePluginPath = string.Empty;
            string ThirdpartyDir = System.IO.Path.Combine(WwisePluginDir, "ThirdParty");
            if (!System.IO.Directory.Exists(ThirdpartyDir))
                return false;

            Assembly CurrentAssembly = Assembly.GetExecutingAssembly();
            Type[] AllTypes = CurrentAssembly.GetTypes();

            if (!GetWwiseSoundEngineVersionObjRef(AllTypes, WwisePluginDir, out object WwiseSoundEngineVersionRef))
                return false;

            if (!GetWwiseVersionFromSoundEngineVersionRef(WwiseSoundEngineVersionRef, out string WwiseEngineVersion))
                return false;


            List<string> DynamicLibs = GetWwiseThirdpartyDynamicLibs(AllTypes, Target, WwiseEngineVersion, ThirdpartyDir);

            return GetMetaWwisePluginPathInternal(DynamicLibs, out MetaWwisePluginPath);


            Type FindSoundEngineClass(Type[] Types)
            {
                foreach (Type type in Types)
                {
                    if (type.Name == "WwiseSoundEngineVersion")
                        return type;
                }
                return null;
            }

            Type FindWwiseUEPlatformClass(Type[] Types)
            {
                foreach (Type type in Types)
                {
                    if (type.Name == "WwiseUEPlatform")
                        return type;
                }
                return null;
            }

            bool GetWwiseSoundEngineVersionObjRef(Type[] Types, string WwisePluginDirectory, out object SoundEngineVersionRef)
            {
                SoundEngineVersionRef = null;
                Type WwiseSoundEngineVersionType = FindSoundEngineClass(Types);
                if (WwiseSoundEngineVersionType == null)
                    return false;

                MethodInfo MethodInfo = WwiseSoundEngineVersionType.GetMethod("GetInstance", BindingFlags.Public | BindingFlags.Static);
                if (MethodInfo == null)
                    return false;

                object[] GetInstanceArgs = { WwisePluginDirectory };
                SoundEngineVersionRef = MethodInfo.Invoke(null, GetInstanceArgs);
                return SoundEngineVersionRef != null;
            }

            bool GetWwiseVersionFromSoundEngineVersionRef(object SoundEngineVersionRef, out string SoundEngineVersion)
            {
                SoundEngineVersion = string.Empty;
                Type WwiseSoundEngineVersionType = SoundEngineVersionRef.GetType();

                // major version
                FieldInfo MajorVersionFieldInfo = WwiseSoundEngineVersionType.GetField("Major", BindingFlags.Public | BindingFlags.Instance);
                if (MajorVersionFieldInfo == null)
                    return false;

                int? MajorVersion = MajorVersionFieldInfo.GetValue(WwiseSoundEngineVersionRef) as int?;
                if (MajorVersion == null)
                    return false;

                // minor version
                FieldInfo MinorVersionFieldInfo = WwiseSoundEngineVersionType.GetField("Minor", BindingFlags.Public | BindingFlags.Instance);
                int? MinorVersion = MinorVersionFieldInfo.GetValue(WwiseSoundEngineVersionRef) as int?;
                if (MinorVersion == null)
                    return false;

                SoundEngineVersion = MajorVersion.Value + "_" + MinorVersion.Value;
                return true;
            }

            List<string> GetWwiseThirdpartyDynamicLibs(Type[] AssemblyTypes, ReadOnlyTargetRules TargetRules, string WwiseVersion, string WwiseThirdpartyDir)
            {
                Type WwiseUEPlatformType = FindWwiseUEPlatformClass(AssemblyTypes);
                MethodInfo WwiseUEPlatformMethodInfo = WwiseUEPlatformType.GetMethod("GetWwiseUEPlatformInstance", BindingFlags.Public | BindingFlags.Static);
                if (WwiseUEPlatformMethodInfo == null)
                    return new List<string>();

                object[] GetWwiseUEPlatformInstanceArgs = { TargetRules, WwiseVersion, WwiseThirdpartyDir };
                object WwisePlatformRef = WwiseUEPlatformMethodInfo.Invoke(null, GetWwiseUEPlatformInstanceArgs);
                Type WwisePlatformType = WwisePlatformRef.GetType();
                MethodInfo GetRuntimeDepMethodInfo = WwisePlatformType.GetMethod("GetRuntimeDependencies", BindingFlags.Public | BindingFlags.Instance);
                if (GetRuntimeDepMethodInfo == null)
                    return new List<string>();

                return GetRuntimeDepMethodInfo.Invoke(WwisePlatformRef, new object[] { }) as List<string>;
            }

            bool GetMetaWwisePluginPathInternal(List<string> DynamicLibs, out string OutMetaWwisePluginPath)
            {
                const string MetaWwiseDLLName = "MetaXRAudioWwise.dll";
                OutMetaWwisePluginPath = string.Empty;

                foreach (string CurrentDLLPath in DynamicLibs)
                {
                    string CurrentDLL = System.IO.Path.GetFileName(CurrentDLLPath);
                    if (CurrentDLL == MetaWwiseDLLName && System.IO.File.Exists(CurrentDLLPath))
                    {
                        OutMetaWwisePluginPath = CurrentDLLPath.Replace("\\", "\\\\");
                        return true;
                    }
                }

                return false;
            }
        }
    }
}

