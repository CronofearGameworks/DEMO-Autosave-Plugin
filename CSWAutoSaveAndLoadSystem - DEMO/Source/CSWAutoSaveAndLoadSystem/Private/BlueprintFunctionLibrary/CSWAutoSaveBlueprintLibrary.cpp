/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#include "BlueprintFunctionLibrary/CSWAutoSaveBlueprintLibrary.h"
#include "EngineUtils.h"	///actoriterator
#include "Serialization/BufferArchive.h"	///MemoryWriter
#include "Serialization/MemoryReader.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Kismet/KismetStringLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "SaveGame/CSWAutoSaveObject.h"
#include "ActorComponent/CSWAutoSaveComponent.h"
#include "Async/CSWAutoSaveAsyncTasks.h"
#include "SaveSystem/CSWSaveGameSystem.h"
#include "ActorComponent/CSWStorerComponent.h"
#include "Serialization/CustomVersion.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/Package.h"
#include "Engine/Engine.h"
#include "Misc/EngineVersion.h"


#define OUT

static const int UE4_SAVEGAME_FILE_TYPE_TAG = 0x53415647;		// "sAvG"

struct FSaveGameFileVersion
{
	enum Type
	{
		InitialVersion = 1,
		// serializing custom versions into the savegame data to handle that type of versioning
		AddedCustomVersions = 2,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};


#pragma region AUTO SAVE AND LOAD MAIN FUNCTIONS

bool UCSWAutoSaveBlueprintLibrary::CSWSaveGameToSlot(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bCompressFile /*= true*/, const bool bUseCustomPath /*= false*/, const FString& Path /*= ""*/)
{
	ICSWSaveGameSystem* CSWSaveSystem = ICSWPlatformFeaturesModule::Get().GetSaveGameSystem();
	// If we have a system and an object to save and a save name...
	if (CSWSaveSystem && SaveGameObject && (SlotName.Len() > 0))
	{
		TArray<uint8> ObjectBytes;
		FMemoryWriter MemoryWriter(ObjectBytes, true);

		// write file type tag. identifies this file type and indicates it's using proper versioning
		// since older UE4 versions did not version this data.
		int32 FileTypeTag = UE4_SAVEGAME_FILE_TYPE_TAG;
		MemoryWriter << FileTypeTag;

		// Write version for this file format
		int32 SavegameFileVersion = FSaveGameFileVersion::LatestVersion;
		MemoryWriter << SavegameFileVersion;

		// Write out engine and UE4 version information
		int32 PackageFileUE4Version = GPackageFileUE4Version;
		MemoryWriter << PackageFileUE4Version;
		FEngineVersion SavedEngineVersion = FEngineVersion::Current();
		MemoryWriter << SavedEngineVersion;

		// Write out custom version data
		ECustomVersionSerializationFormat::Type const CustomVersionFormat = ECustomVersionSerializationFormat::Latest;
		int32 CustomVersionFormatInt = static_cast<int32>(CustomVersionFormat);
		MemoryWriter << CustomVersionFormatInt;
		FCustomVersionContainer CustomVersions = FCustomVersionContainer::GetRegistered();
		CustomVersions.Serialize(MemoryWriter, CustomVersionFormat);

		// Write the class name so we know what class to load to
		FString SaveGameClassName = SaveGameObject->GetClass()->GetName();
		MemoryWriter << SaveGameClassName;

		// Then save the object state, replacing object refs and names with strings
		FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
		SaveGameObject->Serialize(Ar);

		/// Compress ObjectBytes
		if (bCompressFile)
		{
			TArray<uint8> CompressedObjectBytes;
			CompressArrayOfBytes(ObjectBytes, OUT CompressedObjectBytes);
			// Stuff that data into the save system with the desired file name
			return CSWSaveSystem->SaveGame(false, bUseCustomPath, bCompressFile, *Path, *SlotName, UserIndex, CompressedObjectBytes);
		}
		else
		{
			// Stuff that data into the save system with the desired file name
			return CSWSaveSystem->SaveGame(false, bUseCustomPath, bCompressFile, *Path, *SlotName, UserIndex, ObjectBytes);
		}
	}
	return false;
}

void UCSWAutoSaveBlueprintLibrary::CSWSaveGameToSlot_Async(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bCompressFile, const bool bUseCustomPath, const FString& Path, const FCSWOnSaveGameResponse& OnCompleted)
{
	(new FAutoDeleteAsyncTask<FCSWAsyncSaveGameToSlot>(SaveGameObject, SlotName, UserIndex, bCompressFile, bUseCustomPath, Path, OnCompleted))->StartBackgroundTask();
}

USaveGame* UCSWAutoSaveBlueprintLibrary::CSWLoadGameFromSlot(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed /*= true*/, const bool bUseCustomPath /*= false*/, const FString& Path /*= ""*/)
{
	ICSWSaveGameSystem* SaveSystem = ICSWPlatformFeaturesModule::Get().GetSaveGameSystem();
	// If we have a save system and a valid name..
	if (SaveSystem && (SlotName.Len() > 0) && SaveGameObject)
	{
		// Load raw data from slot
		TArray<uint8> ObjectBytes;
		bool bSuccess = SaveSystem->LoadGame(false, bUseCustomPath, bFileIsCompressed, *Path, *SlotName, UserIndex, OUT ObjectBytes);
		if (bSuccess == false) return nullptr;
		///Try to decompress data
		TArray<uint8> DecompressedObjectBytes;
		if (bFileIsCompressed)
		{
			DecompressArrayOfBytes(ObjectBytes, OUT DecompressedObjectBytes);
		}
		TArray<uint8>& ObjectBytesToUse = bFileIsCompressed ? DecompressedObjectBytes : ObjectBytes;
		///
		if (bSuccess)
		{
			FMemoryReader MemoryReader(ObjectBytesToUse, true);
			int32 FileTypeTag;
			MemoryReader << FileTypeTag;

			int32 SavegameFileVersion;
			if (FileTypeTag != UE4_SAVEGAME_FILE_TYPE_TAG)
			{
				// this is an old saved game, back up the file pointer to the beginning and assume version 1
				MemoryReader.Seek(0);
				SavegameFileVersion = FSaveGameFileVersion::InitialVersion;
			}
			else
			{
				// Read version for this file format
				MemoryReader << SavegameFileVersion;

				// Read engine and UE4 version information
				int32 SavedUE4Version;
				MemoryReader << SavedUE4Version;

				FEngineVersion SavedEngineVersion;
				MemoryReader << SavedEngineVersion;

				MemoryReader.SetUE4Ver(SavedUE4Version);
				MemoryReader.SetEngineVer(SavedEngineVersion);

				if (SavegameFileVersion >= FSaveGameFileVersion::AddedCustomVersions)
				{
					int32 CustomVersionFormat;
					MemoryReader << CustomVersionFormat;

					FCustomVersionContainer CustomVersions;
					CustomVersions.Serialize(MemoryReader, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));
					MemoryReader.SetCustomVersions(CustomVersions);
				}
			}

			// Get the class name
			FString SaveGameClassName;
			MemoryReader << SaveGameClassName;

			// Try and find it, and failing that, load it
			UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveGameClassName);
			if (SaveGameClass == NULL)
			{
				SaveGameClass = LoadObject<UClass>(NULL, *SaveGameClassName);
			}

			// If we have a class, try and load it.
			if (SaveGameClass != NULL)
			{
				/// Class is obtained from SaveGameObject input. SaveGameObject is already created.
				//SaveGameObject = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass); 

				FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
				SaveGameObject->Serialize(Ar);
			}
		}
		return SaveGameObject;
	}
	else
	{
		return nullptr;
	}
}

void UCSWAutoSaveBlueprintLibrary::CSWLoadGameFromSlot_Async(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed, const bool bUseCustomPath, const FString& Path, const FCSWOnLoadGameResponse& OnCompleted)
{
	(new FAutoDeleteAsyncTask<FCSWAsyncLoadGameFromSlot>(SaveGameObject, SlotName, UserIndex, bFileIsCompressed, bUseCustomPath, Path, OnCompleted))->StartBackgroundTask();
}

bool UCSWAutoSaveBlueprintLibrary::CSWDoesSaveGameExist(const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed /*= true*/, const bool bUseCustomPath /*= false*/, const FString& Path /*= ""*/)
{
	if (ICSWSaveGameSystem* SaveSystem = ICSWPlatformFeaturesModule::Get().GetSaveGameSystem())
	{
		return SaveSystem->DoesSaveGameExist(bUseCustomPath, bFileIsCompressed, *Path, *SlotName, UserIndex);
	}
	return false;
}

bool UCSWAutoSaveBlueprintLibrary::CSWDeleteSaveGameInSlot(const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed /*= true*/, const bool bUseCustomPath /*= false*/, const FString& Path /*= ""*/)
{
	if (ICSWSaveGameSystem* SaveSystem = ICSWPlatformFeaturesModule::Get().GetSaveGameSystem())
	{
		return SaveSystem->DeleteGame(false, bUseCustomPath, bFileIsCompressed, *Path, *SlotName, UserIndex);
	}
	return false;
}

void UCSWAutoSaveBlueprintLibrary::CSWGetSaveGames(TArray<FString>& SlotNames, const bool bFilesAreCompressed /*= true*/, const bool bUseCustomPath /*= false*/, const FString& Path /*= ""*/)
{
	/// Validation
	if (bUseCustomPath && Path.Len() <= 1) return;
	/// Init
	const FString Extension = bFilesAreCompressed ? ".csav" : ".sav";
	const FString LookForExt = TEXT("*") + Extension;
	const FString FinalPath = bUseCustomPath ? Path : GetPathSaveGames();
	/// Find files in folder with ext .csav or .sav
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFiles(OUT SlotNames, *FinalPath, *LookForExt);
	/// remove .sav or .csav extensions (so the names can be used to load save games directly
	for (FString& SlotName : SlotNames)
	{
		SlotName.RemoveFromEnd(*Extension);
	}
}

UCSWAutoSaveObject* UCSWAutoSaveBlueprintLibrary::AutoFillSaveGameObject(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	/// Validation
	if (!AutoSaveGameObject || LevelsWithAutosaveActors.Num() <= 0) return AutoSaveGameObject;
	///Remove save data in AutoSaveGameObject and fill it with empty data
	TryRemoveSavedDataFromLevels(AutoSaveGameObject, LevelsWithAutosaveActors);
	///Save all the actors for all the LevelNameArray into an array in LevelsWithAutosaveActors
	SaveActorsToArrayOfMaps(AutoSaveGameObject, LevelsWithAutosaveActors);
	///Return the AutoSaveGameObject
	return AutoSaveGameObject;
}

bool UCSWAutoSaveBlueprintLibrary::AutoLoadActorsDataFromSave(const UObject* WorldContextObject, UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	if (!WorldContextObject || !AutoSaveGameObject || LevelsWithAutosaveActors.Num() <= 0) return false;
	///Force Garbage Collection
	GEngine->ForceGarbageCollection(true);
	/// Load the data into each actor
	LoadActorDataFromArrayOfMapRecords(WorldContextObject, AutoSaveGameObject, LevelsWithAutosaveActors);
	/// return boolean
	return true;
}

void UCSWAutoSaveBlueprintLibrary::GetLevelsWithAutosaveActors(const UObject* WorldContextObject, const TArray<FName>& LevelNameArray, TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	/// Validation
	if (LevelNameArray.Num() <= 0 || !WorldContextObject) return;
	/// Get World
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World) return;
	/// Before starting we initialize an array of FCSWAutosaveActorsInLevel. This array will only contain level names for now
	for (const FName& levelName : LevelNameArray)
	{
		FCSWLevelWithAutosaveActors temp;
		temp.Name = levelName;
		LevelsWithAutosaveActors.Add(temp);
	}
	///For each actor in the world, if the actor has the component fill the array for both the OutActors and OutActorComponents references
	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;
		///Continue to the next Actor if this one is invalid
		if (!Actor || Actor->IsPendingKill()) continue;
		///Continue to the next Actor if its AutosaveComponent is invalid
		UCSWAutoSaveComponent* AutosaveComponent = Cast<UCSWAutoSaveComponent>(Actor->GetComponentByClass(UCSWAutoSaveComponent::StaticClass()));
		if (!AutosaveComponent) continue;
		/// Get the LevelName the current Actor belongs
		FName ActorLevelName = CSWGetLevelName(GetActorLevel(Actor));
		for (FCSWLevelWithAutosaveActors& levelWithAutosaveActors : LevelsWithAutosaveActors)
		{
			/// If Actor belongs to a level in LevelNameArray, add the Actor to LevelsWithAutosaveActors
			if (ActorLevelName == levelWithAutosaveActors.Name)
			{
				FCSWAutosaveActor temp;
				temp.Actor = Actor;
				temp.AutosaveComponent = AutosaveComponent;
				/// Now this level is holding a new AutosaveActor struct.
				levelWithAutosaveActors.AutosaveActors.Add(temp);
				break;
			}
		}
	}
}
#pragma endregion


#pragma region FUNCTIONS FOR CUSTOMIZING THE AUTO SAVE AND LOAD SYSTEM

void UCSWAutoSaveBlueprintLibrary::TryRemoveSavedDataFromLevels(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	if (!AutoSaveGameObject || LevelsWithAutosaveActors.Num() <= 0) return;

	for (const FCSWLevelWithAutosaveActors& level : LevelsWithAutosaveActors)
	{
		///Remove data from this Level first
		UCSWAutoSaveBlueprintLibrary::TryRemoveSavedDataFromLevel(AutoSaveGameObject, level.Name);
		///Create an empty newMapRecord with only the name of the level for now
		FCSWMapRecord NewMapRecord;
		NewMapRecord.Name = level.Name;
		AutoSaveGameObject->LevelsRecord.Add(NewMapRecord);
	}
}

bool UCSWAutoSaveBlueprintLibrary::TryRemoveSavedDataFromLevel(UCSWAutoSaveObject* AutoSaveGameObject, const FName& LevelName)
{
	if (!AutoSaveGameObject || AutoSaveGameObject->LevelsRecord.Num() <= 0) return false;

	///Search in AutoSaveGameObject->LevelsRecord if theres a LevelRecord with the name of LevelName
	///If it's remove the LevelRecord
	uint32 MapIndex = 0;
	bool bLevelDataWasRemoved = false;
	for (const FCSWMapRecord& MapRecord : AutoSaveGameObject->LevelsRecord)
	{
		if (MapRecord.Name == LevelName && AutoSaveGameObject->LevelsRecord.IsValidIndex(MapIndex))
		{
			AutoSaveGameObject->LevelsRecord.RemoveAt(MapIndex);
			bLevelDataWasRemoved = true;
			break;
		}
		MapIndex++;
	}
	///Return true or false wether if at least one level was removed or not
	return bLevelDataWasRemoved;
}

void UCSWAutoSaveBlueprintLibrary::SaveActorsToArrayOfMaps(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	///Validation
	if (LevelsWithAutosaveActors.Num() <= 0  || !AutoSaveGameObject) return;

	///Foreach Level that contains Actors with AutosaveComponent
	for (const FCSWLevelWithAutosaveActors& levelWithAutoSaveActor : LevelsWithAutosaveActors)
	{
		int32 TotalActorsInCurrentLevel = levelWithAutoSaveActor.AutosaveActors.Num();
		///Since we already know the level name, we can use it to always know in which AutoSaveGameObject->LevelsRecord index save an Actor
		bool bMatchFound = false; // Not needed since always will match, but we're going to use it just in case (Because, AutosaveGame has at least an empty record with the name of the level)
		uint32 LevelRecordIndex = GetLevelRecordIndexSave(AutoSaveGameObject, levelWithAutoSaveActor, OUT bMatchFound);
		///Save all Actors from the current loaded level (levelWithAutoSaveActor) into AutoSaveGameObject->LevelRecords[LevelRecordIndex]
		//Don't save this level if the Level doesn't have any Actors to save
		if (bMatchFound && TotalActorsInCurrentLevel > 0)
		{
			SaveAllActorsInLevel(AutoSaveGameObject, levelWithAutoSaveActor, LevelRecordIndex);
		}
	}
}

void UCSWAutoSaveBlueprintLibrary::LoadActorDataFromArrayOfMapRecords(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	LoadActorDataFromArrayOfMapRecords_Internal(WorldContextObject, AutoSaveGameObject, LevelsWithAutosaveActors, false);
}

void UCSWAutoSaveBlueprintLibrary::LoadActorDataFromArrayOfMapRecords_Internal(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors, const bool bLoadInEditorTime)
{
	if (!WorldContextObject || !AutoSaveGameObject || LevelsWithAutosaveActors.Num() <= 0) return;
	///In this case we use level data from AutoSaveGameObject to start the loading
	for (const FCSWMapRecord& levelRecord : AutoSaveGameObject->LevelsRecord)
	{
		///Since we already know the level name, we can use it to always know in which level to Load these Actors
		bool bMatchFound = false; // We use this so we know that we load data from AutoSaveGameObject->LevelRecords into a Level that exist and is loaded.
		uint32 LevelRecordIndex = GetLevelRecordIndexLoad(levelRecord, LevelsWithAutosaveActors, bMatchFound);
		///Load all Actors from the current levelRecord into its Level
		if (bMatchFound)
		{
			///Since we already know the level we want to load, we can iterate trough only the Actors from this Level
			if (LevelsWithAutosaveActors.IsValidIndex(LevelRecordIndex))
			{
				TArray<FCSWAutosaveActor> AutosaveActorsInLevel;
				AutosaveActorsInLevel = LevelsWithAutosaveActors[LevelRecordIndex].AutosaveActors;
				///Counters
				int32 TotalActorsInCurrentLevel = AutosaveActorsInLevel.Num();
				int32 TotalActorsInLevelRecord = levelRecord.ActorsRecord.Num();
				//Load Actors in Level if there are ActorsRecords to load or if there Actors in the Level
				//In case there are only Actors in the Level and no ActorRecords to Load, we still try to load so the Actors can be destroyed in case they weren't saved before
				if (TotalActorsInCurrentLevel > 0 || TotalActorsInLevelRecord > 0)
				{
					///Load All Actors in Level
					LoadAllActorsInLevel(WorldContextObject, AutoSaveGameObject, levelRecord, AutosaveActorsInLevel, bLoadInEditorTime);
					///Try to destroy actors in case they weren't updated
					CSWTryDestroyActors(AutoSaveGameObject, AutosaveActorsInLevel);
				}
			}
		}
	}
	GEngine->ForceGarbageCollection(true);
}

int32 UCSWAutoSaveBlueprintLibrary::GetTotalAutosaveActors(const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors)
{
	///Validation
	if (LevelsWithAutosaveActors.Num() <= 0) return 0;
	///Count Num of Total Actors
	int32 TotalActors = 0;
	for (const FCSWLevelWithAutosaveActors& level : LevelsWithAutosaveActors)
	{
		TotalActors = TotalActors + level.AutosaveActors.Num();
	}
	return TotalActors;
}

#pragma endregion


#pragma region COMPRESSION UTILITIES

void UCSWAutoSaveBlueprintLibrary::CompressArrayOfBytes(TArray<uint8>& DataArray, TArray<uint8>& CompressedDataArray)
{
	if (DataArray.Num() <= 0) return;

	CompressedDataArray.Empty();
	///Init Compressor
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(CompressedDataArray, ECompressionFlags::COMPRESS_ZLIB);

	if (Compressor.GetError())
	{
		UE_LOG(LogTemp, Error, TEXT("CompressArrayOfBytes Error compressing data"));
		return;
	}
	///Compress the data array
	Compressor << DataArray;
	///Send compressed data to CompressedDataArray
	Compressor.Flush();
	///Clean
	Compressor.FlushCache();
	Compressor.Close();
}

void UCSWAutoSaveBlueprintLibrary::DecompressArrayOfBytes(TArray<uint8>& DataArray, TArray<uint8>& DecompressedDataArray)
{
	if (DataArray.Num() <= 0) return;
	///DECOMPRESS FILE
	FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(DataArray, ECompressionFlags::COMPRESS_ZLIB);

	if (Decompressor.GetError())
	{
		UE_LOG(LogTemp, Error, TEXT("DecompressArrayOfBytes Error decompressing data"));
		return;
	}
	/// Decompress data array
	Decompressor << DecompressedDataArray;
	///Clean
	Decompressor.FlushCache();
	Decompressor.Close();
}

TArray<uint8> UCSWAutoSaveBlueprintLibrary::CSWStringToBytes(const FString& InString, const bool bUseUtf8 /*= true*/)
{
	TArray<uint8> OutBytes;
	if (InString.Len() <= 0) return OutBytes;

	if (bUseUtf8)
	{
		///Convert String to UTF8 String
		FTCHARToUTF8 ToUtf8Converter(InString.GetCharArray().GetData());
		FString InStringUtf = ToUtf8Converter.Get();
		///Convert String to Bytes
		int32 MaxBufferSize = InStringUtf.Len();
		OutBytes.AddUninitialized(MaxBufferSize);
		StringToBytes(InStringUtf, OUT OutBytes.GetData(), MaxBufferSize);
	}
	else
	{
		///Convert String to Bytes
		int32 MaxBufferSize = InString.Len();
		OutBytes.AddUninitialized(MaxBufferSize);
		StringToBytes(InString, OUT OutBytes.GetData(), MaxBufferSize);
	}

	return OutBytes;
}

FString UCSWAutoSaveBlueprintLibrary::CSWBytesToString(const TArray<uint8>& DataArray)
{
	FString OutString;
	if (DataArray.Num() <= 0) return OutString;

	///Convert BytesToString
	OutString = BytesToString(DataArray.GetData(), DataArray.Num());

	return OutString;
}

void UCSWAutoSaveBlueprintLibrary::ConvertObjectToString(UObject* Object, FString& ObjectAsString)
{
	///Check if Object reference is valid
	if (!Object) return;
	///Serialize the Object into DataArray
	TArray<uint8> DataArray;
	FMemoryWriter MemoryWriter(DataArray, true);
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	Object->Serialize(Ar);
	///Now we have an array of bytes inside DataArray representing the object
	TArray<uint8> CompressedDataArray;
	///We use the CompressedDataArray where we're going to store the Compression result of DataArray
	CompressArrayOfBytes(DataArray, OUT CompressedDataArray);
	///Now we have a CompressedDataArray representing the Object
	///Let's convert this CompressedDataArray into a String
	ObjectAsString = BytesToString(CompressedDataArray.GetData(), CompressedDataArray.Num());
	///Clean
	DataArray.Empty();
	CompressedDataArray.Empty();
	MemoryWriter.FlushCache();
	MemoryWriter.Close();
}

void UCSWAutoSaveBlueprintLibrary::ConvertObjectToString_Async(UObject* Object, const FOnConvertObject& OnConvertObject)
{
	(new FAutoDeleteAsyncTask<FCSWAsyncConvertObjectToString>(Object, OnConvertObject))->StartBackgroundTask();
}

void UCSWAutoSaveBlueprintLibrary::ConvertObjectToBytes(UObject* Object, TArray<uint8>& ObjectAsBytes)
{
	///Check if Object reference is valid
	if (!Object) return;
	///Serialize the Object into DataArray
	FMemoryWriter MemoryWriter(ObjectAsBytes, true);
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	Object->Serialize(Ar);
	MemoryWriter.FlushCache();
	MemoryWriter.Close();
}

UObject* UCSWAutoSaveBlueprintLibrary::RestoreObjectFromString(UObject* Object, const FString& ObjectAsString)
{
	///We check both if the Object and the ObjectAsString are valid
	if (!Object || ObjectAsString.Len() < 1)return nullptr;
	///Convert String to Bytes
	TArray<uint8> CompressedDataArray;
	int32 MaxBufferSize = ObjectAsString.Len();
	CompressedDataArray.AddUninitialized(MaxBufferSize);
	StringToBytes(ObjectAsString, OUT CompressedDataArray.GetData(), MaxBufferSize);
	///Now, the String was converted to CompressedDataArray, but the data is compressed so we need to decompress it before serializing it to the object
	///We create a DecompressedDataArray, where the Decompressed Data will be stored
	TArray<uint8> DecompressedDataArray;
	DecompressArrayOfBytes(CompressedDataArray, OUT DecompressedDataArray);
	///Now we have a DecompressedDataArray, let's serialize this data into the Object
	FMemoryReader MemoryReader(DecompressedDataArray, true);
	MemoryReader.Seek(0);
	FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
	Object->Serialize(Ar);
	///Ok, now the object is restored. We don't need the DataArrays now so let's clean it
	///NOTE: ObjectAsString is not removed in this function
	MemoryReader.FlushCache();
	MemoryReader.Close();
	CompressedDataArray.Empty();
	DecompressedDataArray.Empty();
	///Also, return the Object (which will be casted to the same type as the input object)
	return Object;
}

void UCSWAutoSaveBlueprintLibrary::RestoreObjectFromString_Async(UObject* Object, const FString& ObjectAsString, const FOnRestoreObject& OnRestoreObject)
{
	(new FAutoDeleteAsyncTask<FCSWAsyncRestoreObjectFromString>(Object, ObjectAsString, OnRestoreObject))->StartBackgroundTask();
}

UObject* UCSWAutoSaveBlueprintLibrary::RestoreObjectFromBytes(UObject* Object, const TArray<uint8>& ObjectAsBytes)
{
	///Now we have a DecompressedDataArray, let's serialize this data into the Object
	FMemoryReader MemoryReader(ObjectAsBytes, true);
	MemoryReader.Seek(0);
	FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
	Object->Serialize(Ar);
	///Clean
	MemoryReader.FlushCache();
	MemoryReader.Close();
	///Return object
	return Object;
}

#pragma endregion


#pragma region GENERAL UTILITIES

void UCSWAutoSaveBlueprintLibrary::GetSubstringBetween(const FString& SearchIn, const FString& FirstString, const FString& EndString, FString& SubstringResult, const bool bFirstUseCase /*= false*/, const bool bEndUseCase /*= false*/, const bool bFirstSearchFromEnd /*= false*/, const bool bEndSearchFromEnd /*= false*/, const int32 FirstStartPos /*= -1*/, const int32 EndStartPos /*= -1*/, const bool bIncludeFirstString /*= false*/, const bool bIncludeEndString /*= false*/)
{
	/// Get the Index where the First String and the End String Was Found
	/// Example if the SourceString is "/Game/File.asdas", The FirstString is "/" starting from end, and the EndString is "."
	/// The index result for "/Game/File.asdas" will be, BeginIndex: 5 and EndInxes: 10
	int32 BeginIndex = UKismetStringLibrary::FindSubstring(SearchIn, FirstString, bFirstUseCase, bFirstSearchFromEnd, FirstStartPos);
	int32 EndIndex = UKismetStringLibrary::FindSubstring(SearchIn, EndString, bEndUseCase, bEndSearchFromEnd, EndStartPos);
	/// Include First and EndString in the result, IF the booleans are true
	if (bIncludeFirstString == false)
	{
		BeginIndex = BeginIndex + FirstString.Len();
	}
	if (bIncludeEndString == true)
	{
		EndIndex = EndIndex + EndString.Len();
	}

	/// Get the substring between those subindex, in the previous example, the result for index 5 and 10, in the SourceString "/Game/File.asdas" is "File"
	SubstringResult = UKismetStringLibrary::GetSubstring(SearchIn, BeginIndex, EndIndex - BeginIndex /*Length*/);
}

void UCSWAutoSaveBlueprintLibrary::GetAllActorsWithComponent(const UObject* WorldContextObject, const TSubclassOf<UActorComponent> Component, TArray<AActor*>& OutActors, TArray<UActorComponent*>& OutActorComponents)
{
	if (!Component || !WorldContextObject) return;

	OutActors.Empty();
	OutActorComponents.Empty();

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	/// We do nothing if no component class is provided, rather than giving ALL actors!
	///For each actor in the world, if the actor has the component fill the array for both the OutActors and OutActorComponents references
	if (Component && World)
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			UActorComponent* ActorComponent = Actor->GetComponentByClass(Component);
			if (Actor && !Actor->IsPendingKill() && ActorComponent)
			{
				OutActors.Add(Actor);
				OutActorComponents.Add(ActorComponent);
			}
		}
	}
}

void UCSWAutoSaveBlueprintLibrary::GetActorsWithComponentInLevels(const UObject* WorldContextObject, const TSubclassOf<UActorComponent> Component, const TArray<FName>& LevelNameArray, TArray<AActor*>& OutActors, TArray<UActorComponent*>& OutActorComponents)
{
	if (!Component || !WorldContextObject) return;

	OutActors.Empty();
	OutActorComponents.Empty();

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	/// We do nothing if no component class is provided, rather than giving ALL actors!
	///For each actor in the world, if the actor has the component fill the array for both the OutActors and OutActorComponents references
	if (Component && World)
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			UActorComponent* ActorComponent = Actor->GetComponentByClass(Component);
			if (Actor && !Actor->IsPendingKill() && ActorComponent)
			{
				FName ActorLevelName = CSWGetLevelName(GetActorLevel(Actor));
				for (const FName& LevelName : LevelNameArray)
				{
					if (LevelName == ActorLevelName)
					{
						OutActors.Add(Actor);
						OutActorComponents.Add(ActorComponent);
						break;
					}
				}
				
			}
		}
	}
}

AActor* UCSWAutoSaveBlueprintLibrary::GetActorByIDNameAndClass(const UObject* WorldContextObject, const FName IDName, const TSubclassOf<AActor> ActorClass)
{
	if (!WorldContextObject || !ActorClass) return nullptr;

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	/// We do nothing if not class provided, rather than giving ALL actors!
	///For each Actor in the world that inherits from the ActorClass, if the Name of the actors matches the IDName, then return the Actor.
	if (ActorClass && World)
	{
		for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor->IsPendingKill() && Actor->GetFName() == IDName)
			{
				return Actor;
			}
		}
	}
	return nullptr;
}

int32 UCSWAutoSaveBlueprintLibrary::GetActorByIDNameFromAnArrayOfActors(const FName IDName, const TArray<AActor*>& ArrayOfActors, AActor*& Actor)
{
	if (ArrayOfActors.Num() <= 0) return -1;

	int32 Index = 0;
	///For each Actor in the ArrayOfActors, if the Name of the actors matches the IDName, then return the Actor and the Index where the Actor was found
	for (AActor* actor : ArrayOfActors)
	{
		if (!actor->IsPendingKill() && actor->GetFName() == IDName)
		{
			Actor = actor;
			return Index;
		}
		Index++;
	}
	///Return Index -1 if no Actor was found
	return -1;
}

void UCSWAutoSaveBlueprintLibrary::GetLevelsNames(const UObject* WorldContextObject, TArray<FName>& CurrentLevelNameArray)
{
	if (!WorldContextObject) return;

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	TArray<ULevel*> Levels = World->GetLevels();

	///For each level in the Levels array, fill the CurrentMapNameArray with the level name
	for (ULevel* level : Levels)
	{
		CurrentLevelNameArray.Add(CSWGetLevelName(level));
	}
}

FName UCSWAutoSaveBlueprintLibrary::CSWGetLevelName(ULevel* Level)
{
	///We get the full name for the level, which will come in a similar format like this: /Game/UEDPIE_0_level1.level1:PersistentLevel
	///We need to obtain a more readable name, one that can be used in level streaming, like this: /Game/level1
	FString FullName = Level->GetPathName();
	/// First, we create a PathName where the path will be stored, i.e: /Game/
	/// And a FileName where the name of the level will be stores, i.e: level1
	FString PathName, FileName;
	/// Get the Path of the Level
	GetSubstringBetween(FullName, TEXT("/"), TEXT("/"), PathName, /*UseCase*/ false, false, /*SearchFromEnd*/ false, true, -1, -1, /*IncludeString*/ true, true);
	/// Get the name of the level
	GetSubstringBetween(FullName, TEXT("."), TEXT(":"), FileName, /*UseCase*/ false, false, /*SearchFromEnd*/ false, true, -1, -1, /*IncludeString*/ false, false);

	/// In case of the PathName and the FileName doesn't return a value. Use the FullName
	if (PathName.Len() > 0 && FileName.Len() > 0)
	{
		FString NewFullName = PathName + FileName;
		return FName(*NewFullName);
	}
	else
	{
		return FName(*FullName);
	}
}

ULevel* UCSWAutoSaveBlueprintLibrary::GetActorLevel(AActor* Actor)
{
	if (!Actor) return nullptr;

	return Actor->GetLevel();
}

class AActor* UCSWAutoSaveBlueprintLibrary::GetUniqueActorOfClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
{
	if (!WorldContextObject || !ActorClass) return nullptr;
	///GetWorld
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	///Iterate Actors of Class
	AActor* Actor = nullptr;
	uint8 ActorCounter = 0;
	if (ActorClass && World)
	{
		for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
		{
			if (ActorCounter <= 1)
			{
				Actor = *It;
				if (!Actor->IsPendingKill())
				{
					ActorCounter++;
				}
			}
			else
			{
				break;
			}
		}
	}
	///If there's only one actor in the world
	if (ActorCounter == 1)
	{
		return Actor;
	}
	else
	{
		if (ActorCounter == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("CSW: GetUniqueActorOfClass() No Actor Found"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("CSW: GetUniqueActorOfClass() There are more than 1 actor of this class in the world"));
		}
		return nullptr;
	}
}

AActor* UCSWAutoSaveBlueprintLibrary::SpawnActorWithIDNameFromClass(const UObject* WorldContextObject, const TSubclassOf<AActor> Class, const FTransform& SpawnTransform, FName NameID, ULevel* OwnerLevel)
{
	return SpawnActorWithIDNameFromClass_Internal(WorldContextObject, Class, SpawnTransform, NameID, OwnerLevel, false);
}

AActor* UCSWAutoSaveBlueprintLibrary::SpawnActorWithIDNameFromClass_Internal(const UObject* WorldContextObject, const TSubclassOf<AActor> Class, const FTransform& SpawnTransform, FName NameID, ULevel* OwnerLevel, bool bLoadInEditorTime /*= false*/)
{
	if (!WorldContextObject || !Class || !GEngine) return nullptr;
	///GetWorld
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World) return nullptr;
	/// Create Spawn Parameters
	FActorSpawnParameters SpawnInfo;
	if (OwnerLevel) SpawnInfo.OverrideLevel = OwnerLevel;
	SpawnInfo.bAllowDuringConstructionScript = bLoadInEditorTime;
	///If the name is not empty
	if (NameID.ToString().Len() > 0 && NameID.ToString() != "None")
	{
		///Give a name to the SpawnInfo
		SpawnInfo.Name = NameID;
	}
	///And spawn an Actor with the given name (if no name was provided, the name will be random as usual)
	AActor* SpawnedActor = World->SpawnActor<AActor>(Class, SpawnTransform, SpawnInfo);
	return SpawnedActor;
}

bool UCSWAutoSaveBlueprintLibrary::CSWDoesDirectoryExists(const FString& Path)
{
	IPlatformFile& PlatformFileManager = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFileManager.DirectoryExists(*Path);
}

bool UCSWAutoSaveBlueprintLibrary::CSWMakeDirectory(const FString& Path)
{
	IPlatformFile& PlatformFileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFileManager.DirectoryExists(*Path))
	{
		return PlatformFileManager.CreateDirectory(*FPaths::GetPath(Path));
	}
	return false;
}

FString UCSWAutoSaveBlueprintLibrary::GetPathProject()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
}

FString UCSWAutoSaveBlueprintLibrary::GetPathSaved()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
}

FString UCSWAutoSaveBlueprintLibrary::GetPathSaveGames()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + TEXT("SaveGames/"));
}



#pragma endregion


#pragma region PRIVATE::AUTO SAVE AND LOAD FUNCTIONS

void UCSWAutoSaveBlueprintLibrary::SaveActor(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent)
{
	///Save actor
	ActorRecord.Name = Actor->GetFName();
	ActorRecord.Class = Actor->GetClass();
	ActorRecord.XForm = Actor->GetTransform();
	ActorRecord.bLoadRandomID = AutoSaveAndLoadComponent->GetLoadActorWithRandomIDName();

	FMemoryWriter MemoryWriter(ActorRecord.Data, true);
	/// Use a wrapper archive that converts FNames and UObject*'s to strings that can be read back in
	FCSWSaveGameArchive Ar(MemoryWriter, false);
	/// Serialize the object
	Actor->Serialize(Ar);
	///Clean
	MemoryWriter.FlushCache();
	MemoryWriter.Close();
}

void UCSWAutoSaveBlueprintLibrary::SaveActorComponents(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent)
{
	UActorComponent* input = nullptr;
	UCSWAutoSaveComponent* test = Cast<UCSWAutoSaveComponent>(input);
	auto test2 = input->GetClass()->StaticClass();
	///Return if there are no components to save 
	if (AutoSaveAndLoadComponent->GetSaveComponents() == false && AutoSaveAndLoadComponent->GetComponentOptions().Num() < 1) return;

	TArray<UActorComponent*> ActorComponentsArray;
	Actor->GetComponents(ActorComponentsArray);

	for (UActorComponent* actorcomponent : ActorComponentsArray)
	{
		///Find if there are custom options for this component
		FCSWAutoSaveComponentOption componentOptions;
		bool bSaveThisComponent = GetComponentSaveAndLoadConditions(AutoSaveAndLoadComponent, actorcomponent, OUT componentOptions);
		///If this component can be saved
		if (bSaveThisComponent)
		{
			SaveActorComponent(ActorRecord, actorcomponent, AutoSaveAndLoadComponent, componentOptions);
		}
	}
}

void UCSWAutoSaveBlueprintLibrary::SaveActorComponent(FCSWActorRecord& ActorRecord, UActorComponent* ActorComponent, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent, FCSWAutoSaveComponentOption& ComponentOptions)
{
	FCSWActorComponentRecord ActorComponentRecord;
	///Save ActorComponent Name, Class and Transform (if it's a scene component)
	ActorComponentRecord.Name = ActorComponent->GetFName();

	/// IF COMPONENT IS CHILD OF CSWStorerComponent, save its state completely
	if (ActorComponent->GetClass()->IsChildOf(UCSWStorerComponent::StaticClass()))
	{
		ConvertObjectToBytes(ActorComponent, ActorComponentRecord.Data);
		/// Add the ActorComponentRecord to the ActorRecord
		ActorRecord.ComponentsRecord.Add(ActorComponentRecord);
	}
	/// Else, save SAVEGAME flagged variables and custom data only
	else
	{
		///Save relative transform if the components is a scene component
		/// NOTE: componentOptions.Name == "None" means that this component doesn't have custom options so it will use the default options. 
		/// If it's different than "None" the component has custom options like componentOptions.bSaveLocation
		if (ActorComponent->GetClass()->IsChildOf(USceneComponent::StaticClass()))
		{
			USceneComponent* sceneComponent = Cast<USceneComponent>(ActorComponent);
			///Save Relative Location
			if ((AutoSaveAndLoadComponent->GetSaveComponentsLocation() && ComponentOptions.Name == "None") || (ComponentOptions.Name != "None" && ComponentOptions.bSaveLoc))
			{
				ActorComponentRecord.Loc = sceneComponent->RelativeLocation;
			}
			///Save Relative Rotation
			if ((AutoSaveAndLoadComponent->GetSaveComponentsRotation() && ComponentOptions.Name == "None") || (ComponentOptions.Name != "None" && ComponentOptions.bSaveRot))
			{
				ActorComponentRecord.Rot = sceneComponent->RelativeRotation;
			}
			///SetRelative Scale
			if ((AutoSaveAndLoadComponent->GetSaveComponentsScale() && ComponentOptions.Name == "None") || (ComponentOptions.Name != "None" && ComponentOptions.bSaveScale))
			{
				ActorComponentRecord.Scale = sceneComponent->RelativeScale3D;
			}
			else
			{
				ActorComponentRecord.Scale = FVector(1.0, 1.0, 1.0);
			}
		}
		///Save Physics Simulation if it's a Primitive Component
		if (ActorComponent->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
		{
			UPrimitiveComponent* primitiveComponent = Cast<UPrimitiveComponent>(ActorComponent);
			if ((AutoSaveAndLoadComponent->GetSaveComponentsLinearVelocity() && ComponentOptions.Name == "None") || (ComponentOptions.Name != "None" && ComponentOptions.bSaveLVel))
			{
				ActorComponentRecord.LinearVel = primitiveComponent->GetPhysicsLinearVelocity();
			}
			if ((AutoSaveAndLoadComponent->GetSaveComponentsAngularVelocity() && ComponentOptions.Name == "None") || (ComponentOptions.Name != "None" && ComponentOptions.bSaveAVel))
			{
				ActorComponentRecord.AngularVel = primitiveComponent->GetPhysicsAngularVelocityInDegrees();
			}
		}
		///Save Actor Component Data
		FMemoryWriter MemoryWriter(ActorComponentRecord.Data, true);
		/// Use a wrapper archive that converts FNames and UObject*'s to strings that can be read back in
		FCSWSaveGameArchive Ar(MemoryWriter, false);
		ActorComponent->Serialize(Ar);
		/// Add the ActorComponentRecord to the ActorRecord
		ActorRecord.ComponentsRecord.Add(ActorComponentRecord);
		///Clean
		MemoryWriter.FlushCache();
		MemoryWriter.Close();
	}
}

void UCSWAutoSaveBlueprintLibrary::LoadAllActorsInLevel(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, const FCSWMapRecord& levelRecord, TArray<FCSWAutosaveActor>& AutosaveActorsInLevel, const bool bLoadInEditorTime)
{
	if (!WorldContextObject) return;

	for (const FCSWActorRecord& ActorRecord : levelRecord.ActorsRecord)
	{
		LoadActorInLevel(ActorRecord, AutosaveActorsInLevel, AutoSaveGameObject, WorldContextObject, levelRecord, bLoadInEditorTime);
	}
}

void UCSWAutoSaveBlueprintLibrary::LoadActorInLevel(const FCSWActorRecord &ActorRecord, TArray<FCSWAutosaveActor> &AutosaveActorsInLevel, const UCSWAutoSaveObject* AutoSaveGameObject, const UObject* WorldContextObject, const FCSWMapRecord &levelRecord, const bool bLoadInEditorTime)
{
	AActor* LoadedActor = nullptr;
	UCSWAutoSaveComponent* AutosaveComponent = nullptr;
	///Try to get a Loaded Actor in case the Actor already exists in the level (so we update it instead of creating a new one)
	int32 ActorIndex = GetActorByIDFromAutosaveActors(ActorRecord.Name, AutosaveActorsInLevel, OUT LoadedActor);
	///If Actor was found, we remove the data in the index so the iteration will be faster next time
	if (ActorIndex != -1 && AutosaveActorsInLevel.IsValidIndex(ActorIndex))
	{
		AutosaveActorsInLevel.RemoveAt(ActorIndex);
	}
	//Route if Actor exists in the Level and needs to be updated
	if (LoadedActor)
	{
		LoadActor_Internal(ActorRecord, AutoSaveGameObject, AutosaveComponent, LoadedActor, false);
	}
	//Route if Actor doesn't exist in the Level and needs to be created
	else
	{
		///Spawn in this level
		ULevel* LevelOwner = GetLevelReferenceFromName(WorldContextObject, levelRecord.Name);
		//Route if Load With Random Name is Enabled, then create the actor with a random name (spawn it normally)
		if (ActorRecord.bLoadRandomID)
		{
			LoadedActor = SpawnActorWithIDNameFromClass_Internal(WorldContextObject, ActorRecord.Class, ActorRecord.XForm, FName(""), LevelOwner, bLoadInEditorTime);
		}
		//Route if the Actor will be loaded with a given ID
		else
		{
			LoadedActor = SpawnActorWithIDNameFromClass_Internal(WorldContextObject, ActorRecord.Class, ActorRecord.XForm, ActorRecord.Name, LevelOwner, bLoadInEditorTime);
		}
		//Load data for LoadedActor
		if (LoadedActor)
		{
			LoadActor_Internal(ActorRecord, AutoSaveGameObject, AutosaveComponent, LoadedActor, true);
		}
	}
}

void UCSWAutoSaveBlueprintLibrary::FullLoadActorFromRecord(const FCSWActorRecord& ActorRecord, AActor* Actor, UCSWAutoSaveComponent* AutoSaveAndLoadComponent, const bool bLoadActorComponents /*= true*/)
{
	if (!Actor || !AutoSaveAndLoadComponent) return;
	///Load the data from the AutoSaveAndLoadComponent first, so the options will match the options from the savefile
	for (const FCSWActorComponentRecord& actorComponentRecord : ActorRecord.ComponentsRecord)
	{
		///For each component that this actors has, verifies if the name is the same an then loads the data that corresponds
		if (actorComponentRecord.Name == AutoSaveAndLoadComponent->GetFName())
		{
			FCSWAutoSaveComponentOption componentOptions;
			LoadActorComponent(actorComponentRecord, OUT AutoSaveAndLoadComponent, componentOptions, AutoSaveAndLoadComponent);
			break;
		}
	}
	///Load the Actor and the components only if the component is enabled on this actor
	if (AutoSaveAndLoadComponent->GetEnableComponent())
	{
		LoadActor(ActorRecord, Actor);
		if (bLoadActorComponents)
		{
			LoadActorComponents(ActorRecord, Actor, AutoSaveAndLoadComponent);
		}
	}
}

void UCSWAutoSaveBlueprintLibrary::LoadActor(const FCSWActorRecord& ActorRecord, AActor* DynamicActor)
{
	FMemoryReader MemoryReader(ActorRecord.Data, true);
	FCSWSaveGameArchive Ar(MemoryReader, true);
	DynamicActor->Serialize(Ar);
	///Clean
	MemoryReader.FlushCache();
	MemoryReader.Close();
}

void UCSWAutoSaveBlueprintLibrary::LoadActorComponents(const FCSWActorRecord& ActorRecord, AActor* DynamicActor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent)
{
	///Return if there are no components to load or if the AutoSaveAndLoadComponent is nullptr
	if (AutoSaveAndLoadComponent->GetSaveComponents() == false && AutoSaveAndLoadComponent->GetComponentOptions().Num() < 1) return;

	TArray<UActorComponent*> ActorComponentsArray;
	DynamicActor->GetComponents(ActorComponentsArray);
	///This method is faster than above, Serializes the data of all the components of an actor
	int i = 0;
	for (UActorComponent* actorcomponent : ActorComponentsArray)
	{
		if (actorcomponent != AutoSaveAndLoadComponent)
		{
			///Find if there are custom options for this component and determine if this component can be loaded
			FCSWAutoSaveComponentOption componentOptions;
			bool bLoadThisComponent = GetComponentSaveAndLoadConditions(AutoSaveAndLoadComponent, actorcomponent, OUT componentOptions);
			///IF THIS COMPONENT CAN BE LOADED
			if (bLoadThisComponent)
			{
				///Try to load the actor in order first (when it works, it loads the actor components faster)
				if (ActorRecord.ComponentsRecord.IsValidIndex(i))
				{
					const FCSWActorComponentRecord& actorComponentRecordOut = ActorRecord.ComponentsRecord[i];
					if (actorComponentRecordOut.Name == actorcomponent->GetFName())
					{
						LoadActorComponent(actorComponentRecordOut, actorcomponent, componentOptions, AutoSaveAndLoadComponent);
						i++;
						continue;
					}
				}
				///If the actors components weren't loaded at this point, try to search an actor components that matches the name and load it!
				for (const FCSWActorComponentRecord& actorComponentRecord : ActorRecord.ComponentsRecord)
				{
					///For each component that this actors has, verifies if the name is the same an then loads the data that corresponds
					if (actorComponentRecord.Name == actorcomponent->GetFName())
					{
						LoadActorComponent(actorComponentRecord, actorcomponent, componentOptions, AutoSaveAndLoadComponent);
						i++;
						break;
					}
				}
			}
		}
		i++;
	}
}

void UCSWAutoSaveBlueprintLibrary::LoadActorComponent(const FCSWActorComponentRecord &actorComponentRecord, UActorComponent* actorcomponent, const FCSWAutoSaveComponentOption &componentOptions, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent)
{
	/// IF COMPONENT IS CHILD OF CSWStorerComponent, restore its state completely
	if (actorcomponent->GetClass()->IsChildOf(UCSWStorerComponent::StaticClass()))
	{
		RestoreObjectFromBytes(actorcomponent, actorComponentRecord.Data);
	}
	/// Load an Actor Component, using the FCSWSaveGameArchive that will load the SaveGame flagged variables
	else
	{
		FMemoryReader MemoryReader(actorComponentRecord.Data, true);
		FCSWSaveGameArchive Ar(MemoryReader, true);
		actorcomponent->Serialize(Ar);
		///Load Transform if it's an scene component
		if (actorcomponent->GetClass()->IsChildOf(USceneComponent::StaticClass()))
		{
			USceneComponent* sceneComponent = Cast<USceneComponent>(actorcomponent);
			///Load Relative Location, Rotation and Scale
			/// NOTE: componentOptions.Name == "None" means that this component doesn't have custom options so it will use the default options. 
			/// If it's different than "None" the component has custom options like componentOptions.bSaveLocation
			if ((AutoSaveAndLoadComponent->GetSaveComponentsLocation() && componentOptions.Name == "None") || (componentOptions.Name != "None" && componentOptions.bSaveLoc))
			{
				sceneComponent->SetRelativeLocation(actorComponentRecord.Loc, false, nullptr, ETeleportType::TeleportPhysics);
			}
			if ((AutoSaveAndLoadComponent->GetSaveComponentsRotation() && componentOptions.Name == "None") || (componentOptions.Name != "None" && componentOptions.bSaveRot))
			{
				sceneComponent->SetRelativeRotation(actorComponentRecord.Rot, false, nullptr, ETeleportType::TeleportPhysics);
			}
			if ((AutoSaveAndLoadComponent->GetSaveComponentsScale() && componentOptions.Name == "None") || (componentOptions.Name != "None" && componentOptions.bSaveScale))
			{
				sceneComponent->SetRelativeScale3D(actorComponentRecord.Scale);
			}
		}
		///Load Physics Simulation if it's a Primitive Component
		if (actorcomponent->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
		{
			UPrimitiveComponent* primitiveComponent = Cast<UPrimitiveComponent>(actorcomponent);
			if ((AutoSaveAndLoadComponent->GetSaveComponentsLinearVelocity() && componentOptions.Name == "None") || (componentOptions.Name != "None" && componentOptions.bSaveLVel))
			{
				primitiveComponent->SetPhysicsLinearVelocity(actorComponentRecord.LinearVel);
			}
			if ((AutoSaveAndLoadComponent->GetSaveComponentsAngularVelocity() && componentOptions.Name == "None") || (componentOptions.Name != "None" && componentOptions.bSaveAVel))
			{
				primitiveComponent->SetPhysicsAngularVelocityInDegrees(actorComponentRecord.AngularVel);
			}
		}
		///Clean
		MemoryReader.FlushCache();
		MemoryReader.Close();
	}
}

bool UCSWAutoSaveBlueprintLibrary::GetComponentSaveAndLoadConditions(const UCSWAutoSaveComponent* AutoSaveAndLoadComponent, const  UActorComponent* actorcomponent, FCSWAutoSaveComponentOption &componentOptions)
{
	bool bComponentOptionsFound = false;
	for (FCSWAutoSaveComponentOption& options : AutoSaveAndLoadComponent->GetComponentOptions())
	{
		if (options.Name == actorcomponent->GetFName())
		{
			bComponentOptionsFound = true;
			componentOptions = options;
			break;
		}
	}
	///Determine if this component can be saved/loaded
	bool bSaveLoadThisComponent = false;
	/// When save components is enabled by default, save only if no ComponentOptions where found OR if a component option was found and save component is checked in the options
	if (AutoSaveAndLoadComponent->GetSaveComponents())
	{
		bSaveLoadThisComponent = (bComponentOptionsFound == false) || (bComponentOptionsFound && componentOptions.bSave);
	}
	/// When save components is not enabled by default, save only if a component option was found and save component is checked in the options
	else
	{
		bSaveLoadThisComponent = bComponentOptionsFound && componentOptions.bSave;
	}
	return bSaveLoadThisComponent;
}

void UCSWAutoSaveBlueprintLibrary::CSWTryDestroyActors(const UCSWAutoSaveObject* AutoSaveGameObject, TArray<FCSWAutosaveActor>& AutosaveActorsInLevel)
{
	if (!AutoSaveGameObject || AutosaveActorsInLevel.Num() <= 0) return;
	
	for (FCSWAutosaveActor& autosaveActor : AutosaveActorsInLevel)
	{
		TryDestroyActor(autosaveActor, AutoSaveGameObject);
	}
}

void UCSWAutoSaveBlueprintLibrary::TryDestroyActor(FCSWAutosaveActor &autosaveActor, const UCSWAutoSaveObject* AutoSaveGameObject)
{
	if (autosaveActor.Actor && autosaveActor.AutosaveComponent && !autosaveActor.Actor->IsPendingKill())
	{
		if (autosaveActor.AutosaveComponent && autosaveActor.AutosaveComponent->GetDestroyActorOnLoadGameIfWasNotSaved() && autosaveActor.AutosaveComponent->GetWasSaved() == false)
		{
			///#Call the Event OnBeginDestroyUnsavedActor() so we give a chance to abort destruction 
			autosaveActor.AutosaveComponent->OnBeginDestroyUnsavedActor(AutoSaveGameObject);
			///Destroy the actor
			autosaveActor.Actor->Destroy();
			autosaveActor.Actor->ConditionalBeginDestroy();
			autosaveActor.Actor = nullptr;
		}
		else
		{
			///#Call the Event OnUnchangedActor() so we tell that this actor wasn't changed at all when the game was loaded
			autosaveActor.AutosaveComponent->OnUnchangedActor(AutoSaveGameObject);
		}
	}
}

uint32 UCSWAutoSaveBlueprintLibrary::GetLevelRecordIndexSave(UCSWAutoSaveObject* AutoSaveGameObject, const FCSWLevelWithAutosaveActors& LevelWithAutosaveActors, bool& bMatchFound)
{
	bMatchFound = false;
	uint32 LevelRecordIndex = 0;
	for (const FCSWMapRecord& mapRecord : AutoSaveGameObject->LevelsRecord)
	{
		if (LevelWithAutosaveActors.Name == mapRecord.Name)
		{
			bMatchFound = true;
			break;
		}
		LevelRecordIndex++;
	}
	return LevelRecordIndex;
}

uint32 UCSWAutoSaveBlueprintLibrary::GetLevelRecordIndexLoad(const FCSWMapRecord& LevelRecord, const TArray<FCSWLevelWithAutosaveActors>& LevelSWithAutosaveActors, bool& bMatchFound)
{
	bMatchFound = false;
	uint32 LevelRecordIndex = 0;
	for (const FCSWLevelWithAutosaveActors& levelWithAutoSaveActors : LevelSWithAutosaveActors)
	{
		if (levelWithAutoSaveActors.Name == LevelRecord.Name)
		{
			bMatchFound = true;
			break;
		}
		LevelRecordIndex++;
	}
	return LevelRecordIndex;
}

void UCSWAutoSaveBlueprintLibrary::SaveAllActorsInLevel(UCSWAutoSaveObject* AutoSaveGameObject, const FCSWLevelWithAutosaveActors& LevelWithAutosaveActors, uint32 LevelRecordIndex)
{
	for (const FCSWAutosaveActor& AutosaveActor : LevelWithAutosaveActors.AutosaveActors)
	{
		SaveActorInLevel(AutosaveActor, AutoSaveGameObject, LevelRecordIndex);
	}
}

void UCSWAutoSaveBlueprintLibrary::SaveActorInLevel(const FCSWAutosaveActor &AutosaveActor, UCSWAutoSaveObject* AutoSaveGameObject, uint32 LevelRecordIndex)
{
	AActor* Actor = AutosaveActor.Actor;
	UCSWAutoSaveComponent* AutosaveComponent = AutosaveActor.AutosaveComponent;

	if (Actor && AutosaveComponent && !Actor->IsPendingKill() && AutosaveComponent->GetEnableComponent())
	{
		///#CALL The event OnSaveStart (Before the actor is saved)
		AutosaveComponent->OnSaveStart(AutoSaveGameObject);
		///Create an ActorRecord for Store ActorName, ActorClass, ActorTransform and ActorData (SaveGame flagged Variables)
		///#CronofearNiceStuffHere Breakpoint here to see how much data an actor is saving (components included).
		FCSWActorRecord NewActorRecord;
		FullSaveActorIntoRecord(NewActorRecord, Actor, AutosaveComponent);
		///Save NewActorRecord into the AutoSaveGameObject
		if (AutoSaveGameObject->LevelsRecord.IsValidIndex(LevelRecordIndex))
		{
			AutoSaveGameObject->LevelsRecord[LevelRecordIndex].ActorsRecord.Add(NewActorRecord);
			///#CALL the Event OnSaveEnd (After the actor is saved)
			AutosaveComponent->OnSaveEnd(AutoSaveGameObject);
		}
	}
}



void UCSWAutoSaveBlueprintLibrary::FullSaveActorIntoRecord(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutosaveComponent)
{
	SaveActor(ActorRecord, Actor, AutosaveComponent);
	SaveActorComponents(ActorRecord, Actor, AutosaveComponent);
}

int32 UCSWAutoSaveBlueprintLibrary::GetActorByIDFromAutosaveActors(const FName IDName, const TArray<FCSWAutosaveActor>& AutosaveActorsInLevel, AActor*& Actor)
{
	if (AutosaveActorsInLevel.Num() <= 0) return -1;

	int32 Index = 0;
	///For each Actor in the ArrayOfActors, if the Name of the actors matches the IDName, then return the Actor and the Index where the Actor was found
	for (const FCSWAutosaveActor& autosaveActor : AutosaveActorsInLevel)
	{
		if (autosaveActor.Actor && !autosaveActor.Actor->IsPendingKill() && autosaveActor.Actor->GetFName() == IDName)
		{
			Actor = autosaveActor.Actor;
			return Index;
		}
		Index++;
	}
	///Return Index -1 if no Actor was found
	return -1;
}

void UCSWAutoSaveBlueprintLibrary::LoadActor_Internal(const FCSWActorRecord& ActorRecord, const UCSWAutoSaveObject* AutoSaveGameObject,  UCSWAutoSaveComponent* AutosaveComponent , AActor* LoadedActor, bool bDestroyActorIfAutosaveComponentDisabled)
{
	AutosaveComponent = Cast<UCSWAutoSaveComponent>(LoadedActor->GetComponentByClass(UCSWAutoSaveComponent::StaticClass()));
	if (AutosaveComponent && AutosaveComponent->GetEnableComponent())
	{
		///#CALL OnLoadStart Event
		AutosaveComponent->OnLoadStart(AutoSaveGameObject);
		///Load Actor and ActorComponent Data
		FullLoadActorFromRecord(ActorRecord, LoadedActor, AutosaveComponent, true);
		///#CALL OnLoadEnd Event
		AutosaveComponent->OnLoadEnd(AutoSaveGameObject);
	}
	else if (bDestroyActorIfAutosaveComponentDisabled)
	{
		///Destroy a recreated actor if it doesn't have a SaveAndLoadComponent or if it's component is disabled
		LoadedActor->Destroy();
		LoadedActor->ConditionalBeginDestroy();
		LoadedActor = nullptr;
	}
}

ULevel* UCSWAutoSaveBlueprintLibrary::GetLevelReferenceFromName(const UObject* WorldContextObject, const FName& NameOfTheLevel)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World) return nullptr;

	TArray<ULevel*> Levels = World->GetLevels();

	///For each level in the Levels array, fill the CurrentMapNameArray with the level name
	for (ULevel* level : Levels)
	{
		if (CSWGetLevelName(level) == NameOfTheLevel)
		{
			return level;
		}
	}
	return nullptr;
}

#pragma endregion
