/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

/**
* Custom Save Game System based on "\Engine\Source\Runtime\Engine\Public\SaveGameSystem.h"
* This custom system adds:
* - Save/load data in custom paths/folders.
* - Custom ".csav" extension for compressed files.
*/

#pragma once

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * Interface for platform feature modules
 */

/** Defines the interface to platform's save game system (or a generic file based one) */
class ICSWSaveGameSystem
{
public:

	// Possible result codes when using DoesSaveGameExist.
	// Not all codes are guaranteed to be returned on all platforms.
	enum class ESaveExistsResult
	{
		OK,						// Operation on the file completely successfully.
		DoesNotExist,			// Operation on the file failed, because the file was not found / does not exist.
		Corrupt,				// Operation on the file failed, because the file was corrupt.
		UnspecifiedError		// Operation on the file failed due to an unspecified error.
	};

	/** Returns true if the platform has a native UI (like many consoles) */
	virtual bool PlatformHasNativeUI() = 0;

	/** Return true if the named savegame exists (probably not useful with NativeUI */
	virtual bool DoesSaveGameExist(const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) = 0;

	/** Similar to DoesSaveGameExist, except returns a result code with more information. */
	virtual ESaveExistsResult DoesSaveGameExistWithResult(const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) = 0;

	/** Saves the game, blocking until complete. Platform may use FGameDelegates to get more information from the game */
	virtual bool SaveGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex, const TArray<uint8>& Data) = 0;

	/** Loads the game, blocking until complete */
	virtual bool LoadGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex, TArray<uint8>& Data) = 0;

	/** Delete an existing save game, blocking until complete */
	virtual bool DeleteGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) = 0;
};


/** A generic save game system that just uses IFileManager to save/load with normal files */
class FGenericCSWSaveGameSystem : public ICSWSaveGameSystem
{
public:
	virtual bool PlatformHasNativeUI() override
	{
		return false;
	}

	virtual ESaveExistsResult DoesSaveGameExistWithResult(const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) override
	{
		///Check if returns "null"
		FString FullPath = GetSaveGamePath(bUseCustomPath, bCompressFile, FilePath, FileName);
		if (FullPath == "null") return ESaveExistsResult::DoesNotExist;
		///
		if (IFileManager::Get().FileSize(*FullPath) >= 0)
		{
			return ESaveExistsResult::OK;
		}
		return ESaveExistsResult::DoesNotExist;
	}

	virtual bool DoesSaveGameExist(const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) override
	{
		return ESaveExistsResult::OK == DoesSaveGameExistWithResult(bUseCustomPath, bCompressFile, FilePath, FileName, UserIndex);
	}

	virtual bool SaveGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex, const TArray<uint8>& Data) override
	{
		///Check if returns "null"
		FString FullPath = GetSaveGamePath(bUseCustomPath, bCompressFile, FilePath, FileName);
		if (FullPath == "null") return false;
		///
		return FFileHelper::SaveArrayToFile(Data, *FullPath);
	}

	virtual bool LoadGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex, TArray<uint8>& Data) override
	{
		///Check if returns "null"
		FString FullPath = GetSaveGamePath(bUseCustomPath, bCompressFile, FilePath, FileName);
		if (FullPath == "null") return false;
		///
		return FFileHelper::LoadFileToArray(Data, *FullPath);
	}

	virtual bool DeleteGame(bool bAttemptToUseUI, const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName, const int32 UserIndex) override
	{
		///Check if returns "null"
		FString FullPath = GetSaveGamePath(bUseCustomPath, bCompressFile, FilePath, FileName);
		if (FullPath == "null") return false;
		///
		return IFileManager::Get().Delete(*FullPath, true, false, !bAttemptToUseUI);
	}

protected:

	/** Get the path to save game file for the given name, a platform _may_ be able to simply override this and no other functions above */
	virtual FString GetSaveGamePath(const bool bUseCustomPath, const bool bCompressFile,  const TCHAR* FilePath, const TCHAR* FileName)
	{
		/// Declare FString wether to use .sav or .csav (compressed)
		FString extension;
		if (bCompressFile)
		{
			extension = "csav";
		}
		else
		{
			extension = "sav";
		}
		/// Save in custom directory. note: If directory doesn't exist there will be an error message.
		if (bUseCustomPath)
		{
			if (!FPaths::DirectoryExists(FilePath))
			{
				UE_LOG(LogTemp, Warning, TEXT("CSWError: Directory \"%s\" doesn't exists."), FilePath);
				return "null";
			}
			/// Return FullPathName if everything was correct. Also, add a ".sav" string at the end
			return FString::Printf(TEXT("%s%s.%s"), FilePath, FileName, *extension);
		}
		/// Save in default "Saved/SaveGames/" folder
		else
		{
			return FString::Printf(TEXT("%sSaveGames/%s.%s"), *FPaths::ProjectSavedDir(), FileName, *extension);
		}
	}
};

class CSWAUTOSAVEANDLOADSYSTEM_API ICSWPlatformFeaturesModule : public IModuleInterface
{
public:

	static ICSWPlatformFeaturesModule& Get()
	{
		static ICSWPlatformFeaturesModule* StaticModule = NULL;
		// first time initialization
		if (!StaticModule)
		{
			const TCHAR* PlatformModuleName = FPlatformMisc::GetPlatformFeaturesModuleName();
			if (PlatformModuleName)
			{
				StaticModule = &FModuleManager::LoadModuleChecked<ICSWPlatformFeaturesModule>(PlatformModuleName);
			}
			else
			{
				// if the platform doesn't care about a platform features module, then use this generic almost empty implementation
				StaticModule = new ICSWPlatformFeaturesModule;
			}
		}

		return *StaticModule;
	}

	virtual class ICSWSaveGameSystem* GetSaveGameSystem();
};