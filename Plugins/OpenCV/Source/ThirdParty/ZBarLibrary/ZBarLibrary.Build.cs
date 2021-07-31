// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ZBarLibrary : ModuleRules
{
	public ZBarLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Tell Unreal that this Module only imports Third-Party-Assets
		Type = ModuleType.External;

		LoadZBarLibrary(Target);
	}

	public bool LoadZBarLibrary(ReadOnlyTargetRules Target)
    {
		bool isLibrarySupported = false;

		// Check which platform Unreal is built for
		if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Hololens", "lib", "libzbar.lib"));

            // Add include directory path
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Hololens", "include"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("libzbar.dll");
            RuntimeDependencies.Add("$(TargetOutputDir)/libzbar.dll", Path.Combine(PluginDirectory, "Binaries/ThirdParty/OpenCVLibrary/Win64/libzbar.dll"));

        }
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Windows", "lib", "libzbar64-0.lib"));

			// Add include directory path
			PublicIncludePaths.Add(Path.Combine(ModulePath, "Windows","include"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("libconv.dll");
		}

		return isLibrarySupported;

	}

	// ModuleDirectory points to the directory .uplugin is located
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

}