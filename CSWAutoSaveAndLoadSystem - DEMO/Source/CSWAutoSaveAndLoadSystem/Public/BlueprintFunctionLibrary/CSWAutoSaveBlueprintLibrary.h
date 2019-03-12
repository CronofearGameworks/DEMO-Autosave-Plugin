/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once

#include "Kismet/GameplayStatics.h"
#include "Field/Struct/CSWAutoSaveStruct.h"
#include "CSWAutoSaveBlueprintLibrary.generated.h"

/**
* Structure that holds an Actor reference and its respective AutosaveComponent
*/
USTRUCT(BlueprintType)
struct FCSWAutosaveActor
{
	GENERATED_USTRUCT_BODY()
	/**
	* The array of Actor references
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor", meta = (DisplayName = "Actor"))
		AActor* Actor;
	/**
	* The array of CSWAutosaveComponent references
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSW", meta = (DisplayName = "AutosaveComponent"))
		UCSWAutoSaveComponent* AutosaveComponent;

	FCSWAutosaveActor()
	{

	}
};
/**
* Structure that holds a Level Name, each Level Names has an array AutosaveActors Associated.
*/
USTRUCT(BlueprintType)
struct FCSWLevelWithAutosaveActors
{
	GENERATED_USTRUCT_BODY()
	/**
	* The name of the level (can be many levels in a single file)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name", meta = (DisplayName = "Level Name"))
		FName Name;
	/**
	* The array of AutosaveActors
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutosaveActor", meta = (DisplayName = "Autosave Actors"))
		TArray<FCSWAutosaveActor> AutosaveActors;

	FCSWLevelWithAutosaveActors()
	{

	}
};

/// Save and load to disk
DECLARE_DYNAMIC_DELEGATE_OneParam(FCSWOnSaveGameResponse, const bool, bWasSuccesful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCSWOnLoadGameResponse, USaveGame*, SaveObject);
/// Convert and restore object
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRestoreObject, UObject*, ObjectFromString);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnConvertObject, FString, ObjectAsString);

 /**
 * GameplayStatics Blueprint Function Library that contains all the statics functions related to AutoSave and AutoLoad system
 *
 * This Blueprint Function Library is responsible for the global functionality of the Save and Load System (mainly AutoFillSaveGameObject(), AutoLoadActorsDataFromSave(), CSWSaveGameToSlot() and CSWLoadGameFromSlot()).
 * This Blueprint Function Library is designed to be called from anywhere.
 * 
 * @see UCSWAutoSaveComponent, UCSWAutoSaveStruct, AutoSaveObject, USaveDummyObject
 */
UCLASS()
class CSWAUTOSAVEANDLOADSYSTEM_API UCSWAutoSaveBlueprintLibrary : public UGameplayStatics
{
	GENERATED_BODY()
	
public:
#pragma region AUTO SAVE AND LOAD MAIN FUNCTIONS
	/**
	*	Save the contents of the SaveGameObject to a slot.
	*	@param SaveGameObject	Object that contains data about the save game that we want to write out
	*	@param SlotName			Name of save game slot to save to.
	*   @param UserIndex		For some platforms, master user index to identify the user doing the saving.
	*	@param bCompressFile	Compressed files have a .csav extension
	*   @param bUseCustomPath	Use "Path" as a custom save directory?
	*	@param Path				Custom Path where the .sav file will be stored. (ex. GetPathSaveGames())
	*	@return					Whether we successfully saved this information
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Save Game To Slot", AdvancedDisplay = "Path,bUseCustomPath,bCompressFile"))
		static bool CSWSaveGameToSlot(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bCompressFile = true, const bool bUseCustomPath = false, const FString& Path = "");
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Async Save Game To Slot", AutoCreateRefTerm = "OnCompleted", AdvancedDisplay = "Path,bUseCustomPath,bCompressFile", bCompressFile = "true", bUseCustomPath = "false"))
		static void CSWSaveGameToSlot_Async(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bCompressFile, const bool bUseCustomPath, const FString& Path, const FCSWOnSaveGameResponse& OnCompleted);


	/**
	*	Load the contents from a given slot.
	*	@param SaveGameObject		Object containing loaded game state.
	*	@param SlotName				Name of the save game slot to load from.
	*   @param UserIndex			For some platforms, master user index to identify the user doing the loading.
	*	@param bFileIsCompressed	Compressed files have a .csav extension
	*   @param bUseCustomPath		Use "Path" as a custom load directory?
	*	@param Path					Custom Path where the .sav file will be stored. (ex. GetPathSaveGames())
	*	@return						Return loaded USaveGame object.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Load Game From Slot", DeterminesOutputType = "SaveGameObject", AdvancedDisplay = "Path,bUseCustomPath,bFileIsCompressed"))
		static USaveGame* CSWLoadGameFromSlot(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed = true, const bool bUseCustomPath = false, const FString& Path = "");
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Async Load Game From Slot", AutoCreateRefTerm = "OnCompleted", AdvancedDisplay = "Path,bUseCustomPath,bFileIsCompressed", bFileIsCompressed = "true", bUseCustomPath = "false"))
		static void CSWLoadGameFromSlot_Async(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed, const bool bUseCustomPath, const FString& Path, UPARAM(DisplayName = "OnCompleted (Use Delay of 0)") const FCSWOnLoadGameResponse& OnCompleted);

	/**
	*  Check if there's a save game file with the specified name. Works for compressed files too.
	*  @param SlotName				Name of save game slot.
	*  @param UserIndex				For some platforms, master user index to identify the user doing the saving.
	*  @param bFileIsCompressed		Was the Game saved using compression? Very important as compressed files have a .csav extension.
	*  @param bUseCustomPath		Search in a custom "Path"?
	*  @param Path					Custom Path to search the save file.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Does Save Game Exist", AdvancedDisplay = "Path,bUseCustomPath,bFileIsCompressed"))
		static bool CSWDoesSaveGameExist(const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed = true, const bool bUseCustomPath = false, const FString& Path = "");

	/**
	*  Delete the file for the corresponding slot. Works for compressed files too.
	*  @param SlotName				Name of save game slot.
	*  @param UserIndex				For some platforms, master user index to identify the user doing the saving.
	*  @param bFileIsCompressed		Was the Game saved using compression? Very important as compressed files have a .csav extension.
	*  @param bUseCustomPath		Search in a custom "Path"?
	*  @param Path					Custom Path to search the save file.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Delete Save Game In Slot", AdvancedDisplay = "Path,bUseCustomPath,bFileIsCompressed"))
		static bool CSWDeleteSaveGameInSlot(const FString& SlotName, const int32 UserIndex, const bool bFileIsCompressed = true, const bool bUseCustomPath = false, const FString& Path = "");

	/**
	*  Get a list of .sav or .csav file names that exist inside a directory.
	*  @param bFileIsCompressed		Search for .sav or .csav files?
	*  @param bUseCustomPath		Search in a custom "Path"?
	*  @param Path					Custom Path to search the save files.
	*  @param SlotNames				List of names of the save files found.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Get Save Games In Directory", AdvancedDisplay = "Path,bUseCustomPath,bFilesAreCompressed"))
		static void CSWGetSaveGames(TArray<FString>& SlotNames, const bool bFilesAreCompressed = true, const bool bUseCustomPath = false, const FString& Path = "");

	/**
	*	Auto Fill the SaveGameObject, it needs an already Created "AutoSaveObject". Use CreateSaveGame() Node to create one.
	*	Auto Fill means that the AutoSaveObject will be populated with all the actors that have a UCSWAutoSaveComponent of the levels in LevelNameArray.
	*	Use SaveGameToSlot() to actually save the SaveGameObject into a file.
	*	@param AutoSaveGameObject				The UCSWAutoSaveObject reference, it can be created using the CreateSaveGameObject() node.
	*	@param LevelNameArray					Filter the save using a level name array (use GetLevels() to obtain the list of levels).
	*	@param ActorsToSerialize				Actors that will be serialized into the SaveGameObject (use GetActorsWithComponent()).
	*	@param AutoSaveAndLoadComponentArray	AutoSaveAndLoadComponents that belong to the ActorsToSerialize (use GetActorsWithComponent()).
	*	@return									The UCSWAutoSaveObject filled with all the actors data from all the maps loaded.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Auto Fill Save Game Object"))
		static UCSWAutoSaveObject* AutoFillSaveGameObject(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);

	/**
	*	Auto Load the Game and updates the data for all the actors (and components) that were saved.
	*	If an actor was saved and don't exist in the world anymore, the actors will be recreated (if the option is enabled in the CSWAutoSaveComponent).
	*	This method loads the data for all the levels in LevelNameArray.
	*	Use LoadGameFromSlot() to obtain an AutoSaveObject from file.
	*	@param AutoSaveGameObject				The UCSWAutoSaveObject reference, needs to carry data from saved Actors.
	*	@param LevelNameArray					Filter the save using a level name array (use GetLevels() to obtain the list of levels).
	*	@param ActorsToSerialize				Actors that will be serialized into the SaveGameObject (use GetActorsWithComponent()).
	*	@param AutoSaveAndLoadComponentArray	AutoSaveAndLoadComponents that belong to the ActorsToSerialize (use GetActorsWithComponent()).
	*	@return									Whether if the Load was successful or Not.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Auto Load Actors Data From Save", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static bool AutoLoadActorsDataFromSave(const UObject* WorldContextObject, UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);

	/**
	* Get an array of struct of type FCSWLevelWithAutosaveActors.
	* This struct contains a "Level Name" and an array of AutosaveActors (Actors with their respective AutosaveComponent reference).
	* @param LevelNameArray				Array of Level Names. Use GetLevelNames() to obtain it.
	* @param LevelsWithAutosaveActors	Array of FCSWLevelWithAutosaveActors
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Main", meta = (DisplayName = "CSW::Get Levels with AutosaveActors", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void GetLevelsWithAutosaveActors(const UObject* WorldContextObject, const TArray<FName>& LevelNameArray, TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);
#pragma endregion

#pragma region FUNCTIONS FOR CUSTOMIZING THE AUTO SAVE AND LOAD SYSTEM
	/**
	*	Used for customizing the functionality.
	*	Remove the MapRecords from the AutoSaveGameObject that match the name of the LevelsNameArray input.
	*	Use CSW::GetLevelsNames() function to obtain the array of maps currently loaded.
	*	@param	AutoSaveGameObject		The CSWAutoSaveObject.
	*	@param	LevelsNameArray			Array of levels names. Use CSW::GetLevelsNames() function to obtain the array of maps currently loaded (it supports level streaming).
	*	@return							True if at least one MapRecords was removed.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Custom", meta = (DisplayName = "CSW::Remove Save Data From Levels with Autosave Actors"))
		static void TryRemoveSavedDataFromLevels(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors); ///#CronofearTODO Remove it?
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Custom", meta = (DisplayName = "CSW::Remove Save Data From Level"))
		static bool TryRemoveSavedDataFromLevel(UCSWAutoSaveObject* AutoSaveGameObject, const FName& LevelName);

	/**
	*	Used for customizing the functionality.
	*	Save an array of Actors into the AutoSaveGameObject. Used to save Actors that attaches the CSWAutosaveComponent.
	*	NOTE: This doesn't delete data from levels inside the AutoSaveGameObject. Use of TryRemoveCurrentMapSavedData() will prevent duplication of data.
	*	@param AutoSaveGameObject						The save object where the data will be stored.
	*	@param ActorsArray								The array of actors that will be serialized.
	*	@param AutoSaveAndLoadComponentArray			The array of components that belongs to the actors.
	*	@param LevelsNameArray							The array of Map Names in order to filter which maps have to be saved.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Custom", meta = (DisplayName = "CSW::Save Actors into Save Object"))
		static void SaveActorsToArrayOfMaps(UCSWAutoSaveObject* AutoSaveGameObject, const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);

	/**
	*	Used for customizing the functionality.
	*	Load the Actors Data from an Array of MapRecords (that is inside the AutoSaveGameObject).
	*	@param AutoSaveGameObject						The save object where the data is stored.
	*	@param LevelsNameArray							The array of LevelsNames Loaded (or filtered). Use GetLevelsNames().
	*	@param ActorsArray								The array of the actors in the world that will be updated on load. Use GetActorsWithComponent(CSWAutoSaveComponent).
	*   @param AutoSaveAndLoadComponentArray			The array of AutoSaveAndLoadComponent that belongs to the actors. Use GetActorsWithComponent(CSWAutoSaveComponent).
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Custom", meta = (DisplayName = "CSW::Load Actors Data From Save", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void LoadActorDataFromArrayOfMapRecords(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);
	UFUNCTION()
		static void LoadActorDataFromArrayOfMapRecords_Internal(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, UPARAM(ref) TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors, const bool bLoadInEditorTime = false);

	/**
	* Get total number of Actors with an Autosavecomponent
	* @param LevelsWithAutosaveActors				Struct of Levels containing Actors with AutosaveComponents.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Custom", meta = (DisplayName = "CSW::Get Total Num Of Autosave Actors"))
		static int32 GetTotalAutosaveActors(const TArray<FCSWLevelWithAutosaveActors>& LevelsWithAutosaveActors);

#pragma endregion


#pragma region COMPRESSION UTILITIES

	/**	
	* Compress an array of Bytes using ZLIB.
	* Use Length() node to see the difference in size.
	* @param DataArray				Array of Bytes that will be compressed.
	* @param CompressedDataArray	The Compressed version of the DataArray.
	* @return						Is the CompressedDataArray result less in size than the DataArray? When the DataArray input is too small (less than 100 bytes), the result is usually bigger.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Compress Array Of Bytes"))
		static void CompressArrayOfBytes(UPARAM(ref) TArray<uint8>& DataArray, TArray<uint8>& CompressedDataArray);

	/**
	* Decompress an array of Bytes using ZLIB.
	* Use Length() node to see the difference in size.
	* @param DataArray					Array of Bytes that will be compressed.
	* @param DeCompressedDataArray		The Decompressed Version of the Array Of Bytes.
	* @return							Decompressed Array of Bytes.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Decompress Array Of Bytes", Keywords = "Uncompress"))
		static void DecompressArrayOfBytes(UPARAM(ref) TArray<uint8>& DataArray, TArray<uint8>& DecompressedDataArray);

	/**
	* Convert a String into an Array of Bytes. Useful for sending data through the network.
	* @param InString	The String to be converted.
	* @param bUseUtf8	Convert the text to Standard UTF-8 before compressing it. Useful for sending data through the network.
	* @return			An Array of Bytes representing the text.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::String To Bytes"))
		static TArray<uint8> CSWStringToBytes(const FString& InString, const bool bUseUtf8 = true);

	/**
	* Convert an Array of Bytes into String. Useful for sending data through the network.
	* @param DataArray	The Array of Bytes to be converted.
	* @return			A String based on the array of bytes.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Bytes To String"))
		static FString CSWBytesToString(const TArray<uint8>& DataArray);

	/**
	* Serialize an Object into a Compressed String
	* Useful for sending objects data trough the network
	* Note: Object is not deleted in the process
	* @param Object						The Object that will be converted.
	* @param CompressedDataString		The String Result.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Serialize Object To Compressed String"))
		static void ConvertObjectToString(UObject* Object, FString& ObjectAsString);
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (AutoCreateRefTerm = "OnConvertObject", DisplayName = "CSW::Async Serialize Object To Compressed String"))
		static void ConvertObjectToString_Async(UObject* Object, const FOnConvertObject& OnConvertObject);
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Serialize Object To Bytes"))
		static void ConvertObjectToBytes(UObject* Object, TArray<uint8>& ObjectAsBytes);
	
	/**
	* Restore and Object from a String Input.
	* Caution: The String must belong to the same type of the object previously serialized.
	* Note: CompressedDataString is not deleted in the process
	* @param Object						The Object that will be Restored.
	* @param CompressedDataString		The String Input.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Restore Object From Compressed String", DeterminesOutputType = "Object"))
		static UObject* RestoreObjectFromString(UObject* Object, const FString& ObjectAsString);
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (AutoCreateRefTerm = "OnRestoreObject", DisplayName = "CSW::Async Restore Object From Compressed String", DeterminesOutputType = "Object"))
		static void RestoreObjectFromString_Async(UObject* Object, const FString& ObjectAsString, const FOnRestoreObject& OnRestoreObject);
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Compress", meta = (DisplayName = "CSW::Restore Object From Bytes", DeterminesOutputType = "Object"))
		static UObject* RestoreObjectFromBytes(UObject* Object, const TArray<uint8>& ObjectAsBytes);

#pragma endregion


#pragma region GENERAL UTILITIES

	/**
	*	Given a Source String, obtain a Substring between FirstString and EndString
	*   Example, in: "This is a -hello world/ string example" - If FirstString is "-" and EndString is "/" the result will be "hello world".
	*	@param	SearchIn				SourceString where the search will be performed.
	*	@param	FirstString				The first string to find (Using FindSubstring()).
	*	@param	EndString				The end string to find (Using FindSubstring()).
	*	@param	SubstringResult			Substring Result between the FirstString and the EndString.
	*	@param	bFirstUseCase			The search of the FirstString will be case sensitive?
	*	@param	bEndUseCase				The search of the EndString will be case sensitive?
	*	@param	bFirstSearchFromEnd		The search of the FirstString will be performed starting from the end of the SourceString?
	*	@param	bEndSearchFromEnd		The search of the EndString will be performed starting from the end of the SourceString?
	*	@param	FirstStartPos			The position to start the search from FirstString.
	*	@param	EndStartPos				The position to start the search from EndString.
	*	@param	bIncludeFirstString		Include the FirstString in the SubstringResult?
	*	@param	bIncludeEndString		Include the EndString in the SubstringResult?
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Substring Between", AdvancedDisplay = "bFirstUseCase,bEndUseCase,bFirstSearchFromEnd,bEndSearchFromEnd,FirstStartPos,EndStartPos,bIncludeFirstString,bIncludeEndString"))
		static void GetSubstringBetween(const FString& SearchIn, const FString& FirstString, const FString& EndString, FString& SubstringResult,
			const bool bFirstUseCase = false, const bool bEndUseCase = false, const bool bFirstSearchFromEnd = false, const bool bEndSearchFromEnd = false, 
			const int32 FirstStartPos = -1, const int32 EndStartPos = -1, const bool bIncludeFirstString = false, const bool bIncludeEndString = false);

	/**
	*	Find all Actors in the world with the specified component.
	*	This is a slow operation, use with caution e.g. do not use every frame.
	*	@param	Component			Component to find. Must be specified or result array will be empty.
	*	@param	OutActors			Output array of Actors that have specified component.
	*	@param	OutActorComponents	Output array of Actor Components that belongs to the array of actors.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get All Actors With Component", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DeterminesOutputType = "Component", DynamicOutputParam = "OutActorComponents"))
		static void GetAllActorsWithComponent(const UObject* WorldContextObject, const TSubclassOf<UActorComponent> Component, TArray<AActor*>& OutActors, TArray<UActorComponent*>& OutActorComponents);

	/**
	*	Find all Actors in the world with the specified component (filtered by level names)
	*	This is a slow operation, use with caution e.g. do not use every frame.
	*	@param	Component			Component to find. Must be specified or result array will be empty.
	*	@param	OutActors			Output array of Actors that have specified component.
	*	@param	OutActorComponents	Output array of Actor Components that belongs to the array of actors.
	*	@param	LevelNameArray		Array of level names used to filter the search (use GetLevelsNames()).
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get All Actors With Component In Levels", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DeterminesOutputType = "Component", DynamicOutputParam = "OutActorComponents"))
		static void GetActorsWithComponentInLevels(const UObject* WorldContextObject, const TSubclassOf<UActorComponent> Component, const TArray<FName>& LevelNameArray, TArray<AActor*>& OutActors, TArray<UActorComponent*>& OutActorComponents);

	/**
	*	Get an actor by IDName and Class in the world (hover the mouse in the world outliner to see the actor ID Name).
	*	This is a slow operation, use with caution e.g. do not use every frame.
	*	@param	IDName				ID Name to find. Must be specified or the result will be nullptr.
	*	@param	ActorClass			Class Filter of the actor, determines the OutputType.
	*	@return						The Actor found.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Actor By ID Name and Class", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DeterminesOutputType = "ActorClass"))
		static AActor* GetActorByIDNameAndClass(const UObject* WorldContextObject, const FName IDName, const TSubclassOf<AActor> ActorClass);

	/**
	*	Get an actor by IDName from a given Array of Actors.
	*	@param	IDName				ID Name to find. Must be specified or the result will be nullptr.
	*	@param	ArrayOfActors		The Array of Actors in which the search will be performed.
	*	@param	Actor				The Actor found.
	*	@return						The Index of the array where the Actor was found (it's -1 if no actor was found).
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Actor By ID Name From Array Of Actors"))
		static int32 GetActorByIDNameFromAnArrayOfActors(const FName IDName, const TArray<AActor*>& ArrayOfActors, AActor*& Actor);

	/**
	* Get an array of FNames based on the levels currently loaded (including sublevels in level streaming).
	* Names also contain the name of the path of the levels. Ex: "/Game/Content/Levels/level01"
	* If a Level is moved to another location, the name will be changed.
	* @param CurrentMapNameArray - Array of Level Names.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Levels Names", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static void GetLevelsNames(const UObject* WorldContextObject, TArray<FName>& CurrentLevelNameArray);

	/**
	*	Get the Level name (path/name) from the ULevel input reference (i.e obtained from CSW::GetLevelOwnerFromActor()).
	*	You can use this name to load a level, stream load/unload, etc.
	*	@param Level	The Level reference.
	*	@return			The name of the Level ("path/name").
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Name From Level"))
		static FName CSWGetLevelName(ULevel* Level);

	/**
	*	Get the Level reference where the actor is loaded (supports level streaming).
	*	@param Actor	The actor reference.
	*	@return			The Level reference where the actor belongs (supports level streaming).
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Level Owner From Actor"))
		static ULevel* GetActorLevel(AActor* Actor);

	/**
	* Get ULevel Reference based on the Name of the level. Iterate over UWord::GetLevels() and see if any of the Levels match.
	* Names are sanitated by using CSWGetLevelName();
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Level From Name", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
		static ULevel* GetLevelReferenceFromName(const UObject* WorldContextObject, const FName& NameOfTheLevel);

	/**
	* Get an actor of this class, only if there's only one actor of this class in the world.
	* This is an slow operation. Use this function with caution, e.g do not use it every frame.
	* @param ActorClass		Class of the actor to return
	* @return				The reference of the actor, can be nullptr if there are none or two+ actors in the world.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true", DisplayName = "CSW::Get Unique Actor Of Class", DeterminesOutputType = "ActorClass"))
		static class AActor* GetUniqueActorOfClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass);

	/**
	*	Spawn an Actor in the world with a given Name and Class.
	*	CAUTION: Use it wisely, don't give an ID Name that already exists in the world or the previous Actor will be deleted.
	*	@param Class				The class of the actor that will be spawned.
	*	@param SpawnTransformm		The transform data.
	*	@param NameID				The Given Name.
	*	@return						The Spawned Actor Reference.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "CSW::Spawn Actor With a Name ID From Class", DeterminesOutputType = "Class"))
		static AActor* SpawnActorWithIDNameFromClass(const UObject* WorldContextObject, const TSubclassOf<AActor> Class, const FTransform& SpawnTransform, FName NameID, ULevel* OwnerLevel);
	UFUNCTION()
		static AActor* SpawnActorWithIDNameFromClass_Internal(const UObject* WorldContextObject, const TSubclassOf<AActor> Class, const FTransform& SpawnTransform, FName NameID, ULevel* OwnerLevel, bool bLoadInEditorTime = false);

	/**
	* Verify if a directory exists.
	* @param Path				Path to check.
	* @return					True if the directory exists.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Does Directory Exist"))
		static bool CSWDoesDirectoryExists(const FString& Path);

	/**
	* Attempt to create a directory tree.
	* Doesn't do anything if the directory already exists, in which case it returns false.
	* Ex1: GetPathSaved()+"Test/" will attempt to create a "Test/" directory inside ".../Project/Saved/".
	* Ex2: GetPathProject()+"Test/a/b/" will attempt to create a "Test/a/b/" directory TREE inside ".../Project/".
	* @param Path				Path that should also contain non-existent directories.
	* @return					True if the directory is successfully created.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Create Directory"))
		static bool CSWMakeDirectory(const FString& Path);

	/**
	* Get the absolute path of the ".../Project/" folder.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Path of /Project"))
		static FString GetPathProject();
	/**
	* Get the absolute path of the ".../Project/Saved/" folder.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Path of /Saved"))
		static FString GetPathSaved();
	/**
	* Get the absolute path of the ".../Project/Saved/SaveGames/" folder.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutoSaveAndLoadSystem::Utils", meta = (DisplayName = "CSW::Get Path of /SaveGames"))
		static FString GetPathSaveGames();
#pragma endregion


#pragma region PRIVATE (NOT DIRECTLY EXPOSED TO BLUEPRINTS)

public:
	/**
	* Save All Actors in a Level
	* Each Actor call FullSaveActorIntoRecord()
	*/
	UFUNCTION()
		static void SaveAllActorsInLevel(UCSWAutoSaveObject* AutoSaveGameObject, const FCSWLevelWithAutosaveActors& LevelWithAutosaveActors, uint32 LevelRecordIndex);
	/**
	* Call OnBeginSave, FullSaveActorInRecord and OnEndSave
	*/
	UFUNCTION()
		static void SaveActorInLevel(const FCSWAutosaveActor &AutosaveActor, UCSWAutoSaveObject* AutoSaveGameObject, uint32 LevelRecordIndex);
	/**
	* Full save Actor into a Record.
	* Call SaveActor() and SaveActorComponents()
	*/
	UFUNCTION()
		static void FullSaveActorIntoRecord(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutosaveComponent);
	/**
	* Save an Actor Data into a CSWActorStruct.
	*/
	UFUNCTION()
		static void SaveActor(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent);
	/**
	* Save the components of an Actor into an Array of CSWActorComponentArrayRecords
	*/
	UFUNCTION()
		static void SaveActorComponents(FCSWActorRecord& ActorRecord, AActor* Actor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent);
	/**
	* Save the components of an Actor into an Array of CSWActorComponentArrayRecords
	*/
	UFUNCTION()
		static void SaveActorComponent(FCSWActorRecord& ActorRecord, UActorComponent* ActorComponent, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent, FCSWAutoSaveComponentOption& ComponentOptions);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


	UFUNCTION()
		static void LoadAllActorsInLevel(const UObject* WorldContextObject, const UCSWAutoSaveObject* AutoSaveGameObject, const FCSWMapRecord& levelRecord, TArray<FCSWAutosaveActor>& AutosaveActorsInLevel, const bool bLoadInEditorTime);

	UFUNCTION()
		static void LoadActorInLevel(const FCSWActorRecord &ActorRecord, TArray<FCSWAutosaveActor> &AutosaveActorsInLevel, const UCSWAutoSaveObject* AutoSaveGameObject, const UObject* WorldContextObject, const FCSWMapRecord &levelRecord, const bool bLoadInEditorTime);

	/**
	*	Load a FCSWActorRecord data into the corresponding actor (the actor components too).
	*/
	UFUNCTION()
		static void FullLoadActorFromRecord(const FCSWActorRecord& ActorRecord, AActor* Actor, UCSWAutoSaveComponent* AutoSaveAndLoadComponent, const bool bLoadActorComponents = true);
	/**
	* Load an Actor from a FCSWActorRecord.
	*/
	UFUNCTION()
		static void LoadActor(const FCSWActorRecord& ActorRecord, AActor* DynamicActor);
	/**
	* Load the components of an Actor from an array of CSWActorComponentArrayRecords
	*/
	UFUNCTION()
		static void LoadActorComponents(const FCSWActorRecord& ActorRecord, AActor* DynamicActor, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent);

	/**
	* Load a component of an actor from a CSWActorComponentRecord
	*/
	UFUNCTION()
		static void LoadActorComponent(const FCSWActorComponentRecord &actorComponentRecord, UActorComponent* actorcomponent, const FCSWAutoSaveComponentOption &componentOptions, const UCSWAutoSaveComponent* AutoSaveAndLoadComponent);

	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	/**
	* Try to destroy an array of actors if their corresponding AutoSaveAndLoadComponents have the bDestroyActorOnLoadGameIfWasNotSaved set to true
	*/
	UFUNCTION()
		static void CSWTryDestroyActors(const UCSWAutoSaveObject* AutoSaveGameObject, TArray<FCSWAutosaveActor>& AutosaveActorsInLevel);
	UFUNCTION()
		static void TryDestroyActor(FCSWAutosaveActor &autosaveActor, const UCSWAutoSaveObject* AutoSaveGameObject);


	/**
	* Check if the component can be saved or not. If it can be saved, returns the componentOptions that can be used to customize the functionality
	*/
	UFUNCTION()
		static bool GetComponentSaveAndLoadConditions(const UCSWAutoSaveComponent* AutoSaveAndLoadComponent, const UActorComponent* actorcomponent, FCSWAutoSaveComponentOption &componentOptions);

	/**
	* Get the LevelRecordIndex to use in the AutoSaveGameObject->LevelRecords array. The index is based on the match of LevelsWithAutosaveActors.Name and AutoSaveGameObject->LevelRecords
	*/
	UFUNCTION()
		static uint32 GetLevelRecordIndexSave(UCSWAutoSaveObject* AutoSaveGameObject, const FCSWLevelWithAutosaveActors& LevelWithAutosaveActors, bool& bMatchFound);
	UFUNCTION()
		static uint32 GetLevelRecordIndexLoad(const FCSWMapRecord& LevelRecord, const TArray<FCSWLevelWithAutosaveActors>& LevelSWithAutosaveActors, bool& bMatchFound);

	/**
	* Get the index and the Actor reference where the Actor was found by Name inside the AutosaveActorsInLevel array
	*/
	UFUNCTION()
		static int32 GetActorByIDFromAutosaveActors(const FName IDName, const TArray<FCSWAutosaveActor>& AutosaveActorsInLevel, AActor*& Actor);
	/**
	* Internal Load Actor
	*/
	UFUNCTION()
		static void LoadActor_Internal(const FCSWActorRecord& ActorRecord, const UCSWAutoSaveObject* AutoSaveGameObject, UCSWAutoSaveComponent* AutosaveComponent, AActor* LoadedActor, bool bDestroyActorIfAutosaveComponentDisabled);
#pragma endregion
};
