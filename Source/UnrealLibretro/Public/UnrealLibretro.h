#pragma once

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

UNREALLIBRETRO_API DECLARE_LOG_CATEGORY_EXTERN(Libretro, Log, All);

class FUnrealLibretroModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	/** Path Resolution */
	template <typename... PathTypes>
	static FString IfRelativeResolvePathRelativeToThisPluginWithPathExtensions(const FString& Path, PathTypes&&... InPaths)
	{
		if (!FPaths::IsRelative(Path)) return Path;
		check(IsInGameThread()); // For IPluginManager
		auto UnrealLibretro = IPluginManager::Get().FindPlugin("UnrealLibretro");
		check(UnrealLibretro.IsValid());

		return FPaths::Combine(UnrealLibretro->GetBaseDir(), InPaths..., Path);
	}

	static FString ResolveCorePath(const FString& UnresolvedCorePath)
	{
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

private:
	void* SDLHandle;

#ifdef PLATFORM_WINDOWS
	FString RedistDirectory;
#endif
};
