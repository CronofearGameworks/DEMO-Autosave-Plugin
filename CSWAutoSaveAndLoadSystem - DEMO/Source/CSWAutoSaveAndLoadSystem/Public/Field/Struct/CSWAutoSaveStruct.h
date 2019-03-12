/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "CSWAutoSaveStruct.generated.h"


/**
* Custom GameArchive that allows to serialize SAVEGAME flagged variables
*/
struct FCSWSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FCSWSaveGameArchive(FArchive& InInnerArchive, bool bInLoadIfFindFails) :FObjectAndNameAsStringProxyArchive(InInnerArchive, bInLoadIfFindFails)
	{
		ArIsSaveGame = true;
		ArNoDelta = true;
	}
};

/**
* The structure where the actor components data will be stored.
* This structure is used to save the components data for each actor for each level loaded.
* The data can be used then to restore the state of the components.
* 
* Each FCSWActorComponentRecord belongs to a FCSWActorRecord.
*/
USTRUCT(BlueprintType)
struct FCSWActorComponentRecord
{
	GENERATED_USTRUCT_BODY()
	
	/**
	* The name of the component
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Name", meta = (DisplayName = "Actor Component Name"))
		FName Name;

	/**
	* The serialized data from the component (SAVEGAME flagged variables mainly)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "ComponentData", meta = (DisplayName = "Actor Component Data"))
		TArray<uint8> Data;

	/**
	* The Location of the component (Only for components that inherits from SceneComponents)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (DisplayName = "Scene Component Location"))
		FVector Loc;
	/**
	* The Rotation of the component (Only for components that inherits from SceneComponents)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (DisplayName = "Scene Component Rotation"))
		FRotator Rot;
	/**
	* The Scale of the component (Only for components that inherits from SceneComponents)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (DisplayName = "Scene Component Scale"))
		FVector Scale;

	/**
	* The Linear Velocity of the component (Only for components that inherits from PrimitiveComponents)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Primitive Component Linear Velocity"))
		FVector LinearVel;
	/**
	* The Angular Velocity of the component (Only for components that inherits from PrimitiveComponents)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Primitive Component Linear Velocity"))
		FVector AngularVel;

	FCSWActorComponentRecord()
	{

	}
};

/**
* The structure where the actors data will be stored.
* This structure is used to save the actors data for each level loaded.
* The data can be used then to restore the state of the actors.
* 
* Each FCSWActorRecord has an array of FCSWActorComponentRecord.
* Each FCSWActorRecord belongs to a FCSWMapRecord.
*/
USTRUCT(BlueprintType)
struct FCSWActorRecord
{
	GENERATED_USTRUCT_BODY()

	/**
	* The name of the Actor
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Name", meta = (DisplayName = "Actor Name"))
		FName Name;
	/**
	* The Class of the Actor
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (DisplayName = "Actor Class"))
		UClass*	Class;
	/**
	* The Transform of the Actor
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (DisplayName = "Actor Transform"))
		FTransform XForm;
	/**
	* Determines if the Actor will be loaded with a random name or not
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Bool", meta = (DisplayName = "Load with Random ID Name?"))
		bool bLoadRandomID;
	/**
	* The data of the Actor (mainly SAVEGAME flagged variables)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "ActorData", meta = (DisplayName = "Actor Data"))
		TArray<uint8> Data;
	/**
	* The array of FCSWActorComponentRecord that this FCSWActorRecord has
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "ComponentData", meta = (DisplayName = "Actor Component Records"))
		TArray<FCSWActorComponentRecord> ComponentsRecord;

	FCSWActorRecord()
	{

	}
};

/**
* The structure where the map data will be stored.
* This structure is used to save the levels loaded data.
* The data can be used then to restore the state of the levels.
* 
* Each FCSWMapRecord has an array of FCSWActorRecord.
*/
USTRUCT(BlueprintType)
struct FCSWMapRecord
{
	GENERATED_USTRUCT_BODY()
	/**
	* The name of the level (can be many levels in a single file)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Name", meta = (DisplayName = "Level Name"))
		FName Name;
	/**
	* The array of FCSWActorRecord
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "ActorRecord", meta = (DisplayName = "Actor Record Array"))
		TArray<FCSWActorRecord> ActorsRecord;

	FCSWMapRecord()
	{

	}
};

/**
* Options used to customize the auto save and load functionality from certain components in an actor that has the CSWAutoSaveComponent attached.
* Print the object name from the component to be certain that the name is correct.
* 
* This structure is used in the CSWAutoSaveComponent.
* These options override the Default options that can be seen in the CSWAutoSaveComponent Property Panel.
*/
USTRUCT(BlueprintType)
struct FCSWAutoSaveComponentOption
{
	GENERATED_USTRUCT_BODY()
	/**
	* The name of the component to be customized (print the object name from the component to be certain that the name is correct)
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "ActorComponent", meta = (DisplayName = "Component Name"))
		FName Name="";
	/**
	* If true, the component will be saved (and the functionality can be customized).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "SceneComponent", meta = (DisplayName = "Save Component?"))
		uint32 bSave : 1;
	/**
	* If true, the location of the component will be saved (if it's a SceneComponent).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "SceneComponent", meta = (EditCondition = "bSave", DisplayName = "Save Component Location?"))
		bool bSaveLoc = true;
	/**
	* If true, the rotation of the component will be saved (if it's a SceneComponent).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "SceneComponent", meta = (EditCondition = "bSave", DisplayName = "Save Component Rotation?"))
		bool bSaveRot = true;
	/**
	* If true, the scale of the component will be saved (if it's a SceneComponent).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "SceneComponent", meta = (EditCondition = "bSave", DisplayName = "Save Component Scale?"))
		bool bSaveScale = true;
	/**
	* If true, the Linear Velocity of the component will be saved (if it's a PrimitiveComponent).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "PrimitiveComponent", meta = (EditCondition = "bSave", DisplayName = "Save Component Linear Velocity?"))
		bool bSaveLVel = true;
	/**
	* If true, the Angular Velocity of the component will be saved (if it's a PrimitiveComponent).
	*/
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "PrimitiveComponent", meta = (EditCondition = "bSave", DisplayName = "Save Component Angular Velocity?"))
		bool bSaveAVel = true;

	FCSWAutoSaveComponentOption()
	{

	}
};