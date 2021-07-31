// Copyright Epic Games, Inc. All Rights Reserved.
using System.IO;

using UnrealBuildTool;

public class OpenCV : ModuleRules
{
	public OpenCV(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Include C++/WinRT files in build
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			// These parameters are mandatory for winrt support
			bEnableExceptions = true;
			bUseUnity = false;
			CppStandard = CppStandardVersion.Cpp17;
			PublicSystemLibraries.AddRange(new string[] { "shlwapi.lib", "runtimeobject.lib" });
			PrivateIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,
												"Include",
												Target.WindowsPlatform.WindowsSdkVersion,
												"cppwinrt"));
		}

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				PrivateIncludePath,
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"RHI",
				"RenderCore",
				"Media",
				"MediaAssets",
				"Projects",
				"AugmentedReality",
				"OpenCVLibrary",
				"ZBarLibrary",
				"ZXingLibrary",
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
	private string PrivateIncludePath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "Private")); }
	}
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}
}
