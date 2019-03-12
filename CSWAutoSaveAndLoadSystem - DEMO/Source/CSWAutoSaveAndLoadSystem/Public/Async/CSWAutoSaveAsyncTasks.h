/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once

#include "BlueprintFunctionLibrary/CSWAutoSaveBlueprintLibrary.h"
#include "Async/AsyncWork.h"

#define OUT

/**
* Async SaveGameToSlot().
* @See UCSWAutoSaveBlueprintLibrary
*/
class FCSWAsyncSaveGameToSlot : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FCSWAsyncSaveGameToSlot>;

private:
	USaveGame* SaveGameObject;
	const FString SlotName;
	const int32 UserIndex;
	const bool bCompressFile;
	const bool bUseCustomPath;
	const FString Path;

public:

	FCSWOnSaveGameResponse OnCompleted;

	/*Default constructor*/
	FCSWAsyncSaveGameToSlot(USaveGame* InSaveGameObject, const FString& InSlotName, const int32 InUserIndex, const bool bInCompressFile, const bool bInUseCustomPath, const FString& InPath, const FCSWOnSaveGameResponse& InOnCompleted)
		: SaveGameObject(InSaveGameObject)
		, SlotName(InSlotName)
		, UserIndex(InUserIndex)
		, bCompressFile(bInCompressFile)
		, bUseCustomPath(bInUseCustomPath)
		, Path(InPath)
		, OnCompleted(InOnCompleted)
	{}

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		///Save Game
		bool bResult = UCSWAutoSaveBlueprintLibrary::CSWSaveGameToSlot(SaveGameObject, SlotName, UserIndex, bCompressFile, bUseCustomPath, Path);
		///Execute OnCompleted
		OnCompleted.ExecuteIfBound(bResult);
	}

	/*This function is needed from the API of the engine.*/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FCSWAsyncSaveGameToSlot, STATGROUP_ThreadPoolAsyncTasks);
	}
};


/**
* Async LoadGameFromSlot().
* @See UCSWAutoSaveBlueprintLibrary
*/
class FCSWAsyncLoadGameFromSlot : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FCSWAsyncLoadGameFromSlot>;

private:
	USaveGame* SaveGameObject;
	const FString SlotName;
	const int32 UserIndex;
	const bool bCompressFile;
	const bool bUseCustomPath;
	const FString Path;

public:
	FCSWOnLoadGameResponse OnCompleted;

	/*Default constructor*/
	FCSWAsyncLoadGameFromSlot(USaveGame* InSaveGameObject, const FString& InSlotName, const int32 InUserIndex, const bool bInCompressFile, const bool bInUseCustomPath, const FString& InPath, const FCSWOnLoadGameResponse& InOnCompleted)
		: SaveGameObject(InSaveGameObject)
		, SlotName(InSlotName)
		, UserIndex(InUserIndex)
		, bCompressFile(bInCompressFile)
		, bUseCustomPath(bInUseCustomPath)
		, Path(InPath)
		, OnCompleted(InOnCompleted)
	{}

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		///Load Game
		USaveGame* OutSaveGameObject = UCSWAutoSaveBlueprintLibrary::CSWLoadGameFromSlot(SaveGameObject, SlotName, UserIndex, bCompressFile, bUseCustomPath, Path);
		///Execute OnCompleted
		OnCompleted.ExecuteIfBound(OutSaveGameObject);
	}

	/*This function is needed from the API of the engine.*/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FCSWAsyncLoadGameFromSlot, STATGROUP_ThreadPoolAsyncTasks);
	}
};

/**
* Async ConvertObjectToString().
* @See UCSWAutoSaveBlueprintLibrary
*/
class FCSWAsyncConvertObjectToString : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FCSWAsyncConvertObjectToString>;

private:
	UObject* Object;
	FString CompressedDataString;

public:
	FOnConvertObject OnConverted;

	/*Default constructor*/
	FCSWAsyncConvertObjectToString(UObject* InObject, const FOnConvertObject& InOnConverted)
		: Object(InObject)
		, OnConverted(InOnConverted)
	{}

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		UCSWAutoSaveBlueprintLibrary::ConvertObjectToString(Object, OUT CompressedDataString);
		OnConverted.ExecuteIfBound(CompressedDataString);
	}

	/*This function is needed from the API of the engine.*/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FCSWAsyncConvertObjectToString, STATGROUP_ThreadPoolAsyncTasks);
	}
};

/**
* Async RestoreObjectFromString().
* @See UCSWAutoSaveBlueprintLibrary
*/
class FCSWAsyncRestoreObjectFromString : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FCSWAsyncRestoreObjectFromString>;

private:
	UObject* Object;
	const FString CompressedDataString;

public:
	FOnRestoreObject OnRestore;

	/*Default constructor*/
	FCSWAsyncRestoreObjectFromString(UObject* InObject, const FString& InCompressedDataString, const FOnRestoreObject& InOnRestore)
		: Object(InObject)
		, CompressedDataString(InCompressedDataString)
		, OnRestore(InOnRestore)
	{
		///#CronofearStudyThis PassbyReference to Async Task (To contructor)
		///CompressedDataString = &InCompressedDataString; Doesn't work...
	}

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		UCSWAutoSaveBlueprintLibrary::RestoreObjectFromString(Object, CompressedDataString);
		OnRestore.ExecuteIfBound(Object);
	}

	/*This function is needed from the API of the engine.*/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FCSWAsyncRestoreObjectFromString, STATGROUP_ThreadPoolAsyncTasks);
	}
};
