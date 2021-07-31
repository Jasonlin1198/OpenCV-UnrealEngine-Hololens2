// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ZXingLibrary : ModuleRules
{
	public ZXingLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Tell Unreal that this Module only imports Third-Party-Assets
		Type = ModuleType.External;

		LoadZXingLibrary(Target);
	}

	public bool LoadZXingLibrary(ReadOnlyTargetRules Target)
    {
		bool isLibrarySupported = false;

		// Check which platform Unreal is built for
		if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Hololens", "lib", "libzxing.lib"));

            // Add include directory path
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Hololens", "core", "src"));
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Hololens", "opencv", "src"));
        }
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Windows", "lib", "libzxing.lib"));

            // Add include directory path
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Windows", "core", "src"));
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Windows", "opencv", "src"));
		}

		return isLibrarySupported;

	}

	// ModuleDirectory points to the directory .uplugin is located
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

}