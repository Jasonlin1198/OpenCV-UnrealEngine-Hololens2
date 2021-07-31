// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class OpenCVLibrary : ModuleRules
{
	public OpenCVLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Tell Unreal that this Module only imports Third-Party-Assets
		Type = ModuleType.External;

		LoadOpenCVLibrary(Target);
	}

	public bool LoadOpenCVLibrary(ReadOnlyTargetRules Target)
	{
        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
            isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Hololens", "lib", "arm64", "opencv_world452.lib"));

            // Add include directory path
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Hololens", "include"));
            PublicIncludePaths.Add(Path.Combine(ModulePath, "Hololens", "sources"));

            PublicDelayLoadDLLs.Add("opencv_world452.dll");
            RuntimeDependencies.Add("$(TargetOutputDir)/opencv_world452.dll", Path.Combine(PluginDirectory, "Binaries/ThirdParty/OpenCVLibrary/Win64/opencv_world452.dll"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
            isLibrarySupported = true;

            // Add the static import library 
            PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Windows", "x64", "vc15", "lib", "opencv_world451.lib"));

			// Add include directory path
			PublicIncludePaths.Add(Path.Combine(ModulePath, "Windows", "include"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("opencv_videoio_ffmpeg451_64.dll");
			PublicDelayLoadDLLs.Add("opencv_world451.dll");

            RuntimeDependencies.Add("$(TargetOutputDir)/opencv_world451.dll", Path.Combine(PluginDirectory, "Binaries/Win64/opencv_world451.dll"));
            RuntimeDependencies.Add("$(TargetOutputDir)/opencv_videoio_ffmpeg451_64.dll", Path.Combine(PluginDirectory, "Binaries/Win64/opencv_videoio_ffmpeg451_64.dll"));
        }

        return isLibrarySupported;
	}

	// ModuleDirectory points to the directory .uplugin is located
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

}