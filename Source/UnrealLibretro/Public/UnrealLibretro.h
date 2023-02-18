#pragma once

#include "LibretroCoreInstance.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/NameTypes.h"

UNREALLIBRETRO_API DECLARE_LOG_CATEGORY_EXTERN(Libretro, Log, All);

#if PLATFORM_WINDOWS && PLATFORM_64BITS
#	define PLATFORM_INDEX 0
#elif PLATFORM_ANDROID_ARM
#   define PLATFORM_INDEX 1
#elif PLATFORM_ANDROID_ARM64
#	define PLATFORM_INDEX 2
#endif

static const struct { FString DistributionPath; FString Extension; FString BuildbotPath; FName ImageName; } CoreLibMetadata[] =
{
	{ TEXT("Win64/"),                  "_libretro.dll",           "https://buildbot.libretro.com/nightly/windows/x86_64/latest/"     ,   "Launcher.Platform_Windows.Large"      },
	{ TEXT("Android/armeabi-v7a/"),    "_libretro_android.so",    "https://buildbot.libretro.com/nightly/android/latest/armeabi-v7a/",   "Launcher.Platform_Android.Large"      },
	{ TEXT("Android/arm64-v8a/"),      "_libretro_android.so",    "https://buildbot.libretro.com/nightly/android/latest/arm64-v8a/"  ,   "Launcher.Platform_Android.Large"      },
};

class FUnrealLibretroModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	/** Path Resolution */
	static bool IsCoreName(const FString &CorePath)
	{
		return FPaths::GetExtension(CorePath).IsEmpty() && !CorePath.IsEmpty() && FPaths::GetPath(CorePath).IsEmpty();
	}

	template <typename... PathTypes>
	static FString IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(const FString& Path, PathTypes&&... InPaths)
	{
		if (!FPaths::IsRelative(Path)) return Path;
		check(IsInGameThread()); // For IPluginManager
		auto UnrealLibretro = IPluginManager::Get().FindPlugin("UnrealLibretro");
		check(UnrealLibretro.IsValid());

		return FPaths::Combine(UnrealLibretro->GetBaseDir(), InPaths..., Path);
	}

	static FString ResolveCorePath(FString UnresolvedCorePath)
	{
		if (IsCoreName(UnresolvedCorePath)) 
		{
			UnresolvedCorePath = CoreLibMetadata[PLATFORM_INDEX].DistributionPath + *UnresolvedCorePath + CoreLibMetadata[PLATFORM_INDEX].Extension;
		}

		return IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(UnresolvedCorePath, TEXT("MyCores"));
	}

	static FString ResolveROMPath(const FString& UnresolvedRomRom)
	{
		return IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(UnresolvedRomRom, TEXT("MyROMs"));
	}

	static FString ResolveSaveStatePath(const FString& UnresolvedRomPath, const FString& UnresolvedSavePath)
	{
		return IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(UnresolvedSavePath, TEXT("Saves"), TEXT("SaveStates"), FPaths::GetCleanFilename(UnresolvedRomPath));
	}

	static FString ResolveSRAMPath(const FString& UnresolvedRomPath, const FString& UnresolvedSavePath)
	{
		return IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(UnresolvedSavePath, TEXT("Saves"), TEXT("SRAM"), FPaths::GetCleanFilename(UnresolvedRomPath));
	}

	static void EnvironmentParseControllerInfo(const retro_controller_info* controller_info,
		                                       TStaticArray<TArray<FLibretroControllerDescription>, PortCount>& ControllerDescriptions)
	{
		for (int port = 0; controller_info[port].types != NULL && port < PortCount; port++) {
			for (unsigned t = 0; t < controller_info[port].num_types; t++) {
				if (controller_info[port].types[t].desc == nullptr) break; // Not part of Libretro API but needed check for some cores

				retro_controller_description controller_description = controller_info[port].types[t];
				ControllerDescriptions[port].Add({ controller_description.desc,
												   controller_description.id });
			}
		}
	}

private:
	void* SDLHandle;

#ifdef PLATFORM_WINDOWS
	FString RedistDirectory;
#endif
};

#undef PLATFORM_INDEX