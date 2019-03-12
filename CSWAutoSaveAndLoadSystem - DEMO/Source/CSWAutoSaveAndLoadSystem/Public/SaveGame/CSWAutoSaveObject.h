/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once

#include "GameFramework/SaveGame.h"
#include "Field/Struct/CSWAutoSaveStruct.h"
#include "CSWAutoSaveObject.generated.h"

/**
* Save Game Object that contains an array of FCSWMapRecord so it can be used to store data.
*
* This Save Game Object is responsible for storing the data of Actors (and components) inside their corresponding levels. The data is persistent even if the levels are changed or loaded/unloaded (level streaming).
* This Save Game Object is designed to store data from multiple levels (including level streaming) so all the data can be stored into a single file using the SaveGameToSlot() function.
*
* @see UCSWAutoSaveStruct
*/
UCLASS(meta = (DisplayName = "CSWAuto Save and Load Game Object"))
class CSWAUTOSAVEANDLOADSYSTEM_API UCSWAutoSaveObject : public USaveGame
{
	GENERATED_BODY()

public:
	/**
	* Array of Map records that contain each Map actor records (Each map contains an array of actor records)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "CSW|AutoSaveAndLoadSystem", meta = (DisplayName = "Levels Record Array"))
		TArray<FCSWMapRecord> LevelsRecord;

	/**
	* Return true if there's data stored inside this SaveGameObject
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutosaveObject")
		bool GetContainsSaveData() { if (LevelsRecord.Num() > 0) { return true; } else { return false; } }

	/**
	* Return the number of levels that are stored in this Save Object
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutosaveObject")
		int32 GetNumberOfLevelsStored() { return LevelsRecord.Num(); }

	/**
	* The Number of Actors that are stored in a Level (by index)
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutosaveObject")
		int32 GetNumberOfActorsStoredInLevel(int32 index)
	{
		if (LevelsRecord.IsValidIndex(index))
		{
			return LevelsRecord[index].ActorsRecord.Num();
		}
		return 0;
	}

	/**
	* The total Number of Actors that are stored in the save object
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|AutosaveObject")
		int32 GetTotalNumberOfActorsStored()
	{
		int32 NumberOfLevels = GetNumberOfLevelsStored();
		int32 TotalNumberOfActorsStored = 0;
		for (int32 i = 0; i < NumberOfLevels; i++)
		{
			TotalNumberOfActorsStored += GetNumberOfActorsStoredInLevel(i);
		}
		return TotalNumberOfActorsStored;
	}
};
