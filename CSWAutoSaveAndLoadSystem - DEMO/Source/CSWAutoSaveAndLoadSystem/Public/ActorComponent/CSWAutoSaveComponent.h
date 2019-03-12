/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once

#include "Components/ActorComponent.h"
#include "Field/Struct/CSWAutoSaveStruct.h"
#include "CSWAutoSaveComponent.generated.h"

/**
* Delegates for Executing Events in the blueprint owner of this component. Returns the UCSWAutoSaveObject used to save/load the game.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSWAutoSaveComponentDelegate, const UCSWAutoSaveObject*, AutoSaveLoadObject);

/**
* This component can be attached to any Actor and will allow to auto save and load the Actors data. This component can be customized in the Blueprint Property panel.
* The functions: AutoFillSaveGameObject() and AutoLoadActorsDataFromSave() from UCSWAutoSaveBlueprinLibrary will save and load Actors that have this component.
* 
* This component is responsible for telling the CSW Auto Save and Load System which Actors are part of the system (so they should be saved and loaded).
* This component is designed to allow custom behavior for each Actor (Even in runtime). See the options in the Blueprint Property panel.
*
* @see UCSWAutoSaveBlueprinLibrary
*/
UCLASS( ClassGroup=("CSW::AutoSaveModule"), meta=(BlueprintSpawnableComponent, DisplayName = "CSWAuto Save And Load Component") )
class CSWAUTOSAVEANDLOADSYSTEM_API UCSWAutoSaveComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region BaseFunctions
protected:
	// Sets default values for this component's properties
	UCSWAutoSaveComponent();
#pragma endregion 

#pragma region CustomFunction
public:
	/**
	* Event Triggered At the Start of Saving (The Moment When AutoFillSaveGameObject() is called) for every actor that has this component
	* Changes in the owner actor of this component (and all of its components) will BE saved if done in this event.
	* Use this event for saving custom data (example: call the functions SetSaved...()).
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On Begin Save"))
		void OnSaveStart(const UCSWAutoSaveObject* CSWAutoSaveObject);

	/**
	* Event Triggered At the End of Saving (After Serializing all the actors data)
	* Changes in the owner actor of this component (and all of its components) will NOT BE saved if done in this event
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On End Save"))
		void OnSaveEnd(const UCSWAutoSaveObject* CSWAutoSaveObject);

	/**
	* Event Triggered At the Start of Loading (SaveGame variables and components DOES NOT have updated values yet).
	* At this moment, the owner actor of this component DOES NOT have the updated values yet but the actor already exists.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On Begin Load"))
		void OnLoadStart(const UCSWAutoSaveObject* CSWAutoSaveObject);

	/**
	* Event Triggered At the End of Loading
	* At this moment, the owner actor of this component DOES have the updated values and the load is Completed.
	* Use this event for loading custom data  (example: call the functions GetSaved...()).
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On End Load"))
		void OnLoadEnd(const UCSWAutoSaveObject* CSWAutoSaveObject);

	/**
	* Event Triggered when loading the game (Function AutoLoadActorsDataFromSave()) AND if the owner Actor of this component wasn't previosly saved AND if bDestroyActorOnLoadGameIfWasNotSaved property was set to TRUE
	* This event can be used to abort the destruction of the owner Actor of this component by setting bDestroyActorOnLoadGameIfWasNotSaved back to FALSE
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On Destroy Actor On Load If Wasn't Saved"))
		void OnBeginDestroyUnsavedActor(const UCSWAutoSaveObject* CSWAutoSaveObject);

	/**
	* Event Triggered when loading the game (Function AutoLoadActorsDataFromSave()) AND if the owner Actor of this component wasn't previosly saved AND if bDestroyActorOnLoadGameIfWasNotSaved property was set to FALSE
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "On Actor Unchanged After Load"))
		void OnUnchangedActor(const UCSWAutoSaveObject* CSWAutoSaveObject);

#pragma endregion

#pragma region Variables
private:
	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Global
	///****************************************************************************************************************************************************
	/**
	* If unchecked, the owner Actor of this component will not be saved or loaded.
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Global", meta = (DisplayName = "Enable Component?"))
		bool bEnable = true; ///bEnableComponent
	/**
	* If the the owner Actor of this component is saved into a file this value will be true
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Was Saved?"))
		bool bSaved = false; ///bWasSaved
	
	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Actor
	///****************************************************************************************************************************************************
	/**
	*	Destroy the owner Actor of this component on Load Game (when CSW::AutoLoadActorsDataFromSave() is called) if the Actor wasn't saved into a SaveFile
	*	For Actors that were spawned in the world and weren't saved
	*	By default the actors will not be destroyed, this is because it can affect the experience while designing a level.
	*	This value can be set to true in runtime, so runtime generated Actors will be destroyed if weren't saved into a SaveFile
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Actor", meta = (DisplayName = "Destroy Actor On Load If Wasn't Saved?"))
		bool bDestroyOnLoad = false; ///bDestroyActorOnLoadGameIfWasNotSaved
	/**
	* If checked, the owner Actor of this component will load with a random ID Name ONLY if there are not actors in the world that match it's IDName (use it with caution)
	* In case these Actors loads and there is an Actor that match it's ID, this option will do nothing as the Actor that match the ID will be updated
	* Useful for loading Actors that were destroyed and if the Garbage Collector isn't doing the job removing them completely.
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Actor", meta = (DisplayName = "Load Actor with Random ID?"))
		bool bRandomID = false;

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Default Components
	///****************************************************************************************************************************************************

	/**
	* If checked, all the components of the owner Actor of this component will be saved by default
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (DisplayName = "Save Components?"))
		bool bSaveComps = false; ///bSaveComponents
	/**
	* If checked, the owner Actor of this component will auto save and load the Location for all its SceneComponents
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (EditCondition = "bSaveComps", DisplayName = "Save Components Locations?"))
		bool bSaveLocs = true; ///bSaveComponentsLocations
	/**
	* If checked, the owner Actor of this component will auto save and load the Rotation for all its SceneComponents
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (EditCondition = "bSaveComps", DisplayName = "Save Components Rotations?"))
		bool bSaveRots = true; ///bSaveComponentsRotations
	/**
	* If checked, the owner Actor of this component will auto save and load the Scale3D Data for all its SceneComponents
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (EditCondition = "bSaveComps", DisplayName = "Save Components Scales?"))
		bool bSaveScals = true; ///bSaveComponentsScales
	/**
	* If checked, the owner Actor of this component will auto save and load the Linear Velocity for all its PrimitiveComponents
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (EditCondition = "bSaveComps", DisplayName = "Save Components Linear Velocities?"))
		bool bSaveLVel = true; ///bSaveComponentsLinearVelocities
	/**
	* If checked, the owner Actor of this component will auto save and load the Angular Velocity for all its PrimitiveComponents
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Default Components", meta = (EditCondition = "bSaveComps", DisplayName = "Save Components Angular Velocities?"))
		bool bSaveAVel = true; ///bSaveComponentsAngularVelocities

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Custom Components
	///****************************************************************************************************************************************************

	/**
	* Custom Options for an specific component of the owner Actor of this component.
	* These custom options will override the Default Components Options.
	*
	* Example: One Character can save by default all it's components data,
	* But if a custom Option is assigned to it's root CollisionCylinder component (aka CapsuleComponent),
	* then custom behavior can be assigned to only the CollisionCylinder (even in runtime).
	* 
	* These Array of Component options are the same as the Default Components Options.
	*/
	UPROPERTY(EditAnywhere, SaveGame, Category = "CSWAutoSaveAndLoadSystem::Custom Components", meta = (EditCondition = "bSC", DisplayName = "Custom Component Options"))
		TArray<FCSWAutoSaveComponentOption> Optns; ///Component Custom Options

public:

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Global
	///****************************************************************************************************************************************************

	/**
	* Get the value of bEnableComponent
	* If false, the owner Actor of this component will not be saved or loaded.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetEnableComponent() const { return bEnable; }
	/**
	* Set the value of bEnableComponent
	* If false, the owner Actor of this component will not be saved or loaded.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetEnableComponent(bool bValue) { bEnable = bValue; }
	/**
	* Get the value of bWasSaved
	* If the Actor is saved into a file this value will be true
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetWasSaved() const { return bSaved; }
	/**
	* Set the value of bWasSaved
	* If the Actor is saved into a file this value will be true
	* This value is automatically set to true if the owner Actor of this component was saved
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetWasSaved(bool bValue) { bSaved = bValue; }
	/**
	*	Get the value of bDestroyActorOnLoadGameIfWasNotSaved
	*	If true, Destroy the owner Actor of this component on Load Game (when CSW::AutoLoadActorsDataFromSave() is called) if the Actor wasn't saved into a SaveFile
	*	For Actors that were spawned in the world and weren't saved
	*	By default the actors will not be destroyed, this is because it can affect the experience while designing a level.
	*	This value can be set to true in runtime, so runtime generated Actors will be destroyed if weren't saved into a SaveFile
	*/

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Actor
	///****************************************************************************************************************************************************

	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetDestroyActorOnLoadGameIfWasNotSaved() const { return bDestroyOnLoad; }
	/**
	*	Set the value of bDestroyActorOnLoadGameIfWasNotSaved
	*	If true, Destroy the owner Actor of this component on Load Game (when CSW::AutoLoadActorsDataFromSave() is called) if the Actor wasn't saved into a SaveFile
	*	For Actors that were spawned in the world and weren't saved
	*	By default the actors will not be destroyed, this is because it can affect the experience while designing a level.
	*	This value can be set to true in runtime, so runtime generated Actors will be destroyed if weren't saved into a SaveFile
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetDestroyActorOnLoadGameIfWasNotSaved(bool bValue) { bDestroyOnLoad = bValue; }
	/**
	* Get the value of bLoadActorWithRandomIDName
	* If true, the owner Actor of this component will load with a random ID Name ONLY if there are not actors in the world that match it's IDName (use it with caution)
	* In case these Actors loads and there is an Actor that match it's ID, this option will do nothing as the Actor that match the ID will be updated
	* Useful for loading Actors that were destroyed and if the Garbage Collector isn't doing the job removing them completely.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetLoadActorWithRandomIDName() const { return bRandomID; }
	/**
	* Set the value of bLoadActorWithRandomIDName
	* If true, the owner Actor of this component will load with a random ID Name ONLY if there are not actors in the world that match it's IDName (use it with caution)
	* In case these Actors loads and there is an Actor that match it's ID, this option will do nothing as the Actor that match the ID will be updated
	* Useful for loading Actors that were destroyed and if the Garbage Collector isn't doing the job removing them completely.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetLoadActorWithRandomIDName(bool bValue) { bRandomID = bValue; }

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Default Components
	///****************************************************************************************************************************************************

	/**
	* Get the value of bSaveComponents
	* If true, all the components of the owner Actor of this component will be saved by default
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponents() const { return bSaveComps; }
	/**
	* Set the value of bSaveComponents
	* If true, all the components of the owner Actor of this component will be saved by default
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponents(bool bValue) { bSaveComps = bValue; }
	/**
	* Get the value of bSaveComponentsLocation
	* If true, the owner Actor will auto save and load the Location for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponentsLocation() const { return bSaveLocs; }
	/**
	* Set the value of bSaveComponentsLocation
	* If true, the owner Actor will auto save and load the Location for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponentsLocation(bool bValue) { bSaveLocs = bValue; }
	/**
	* Get the value of bSaveComponentsRotation
	* If true, the owner Actor will auto save and load the Rotation for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponentsRotation() const { return bSaveRots; }
	/**
	* Set the value of bSaveComponentsRotation
	* If true, the owner Actor will auto save and load the Rotation for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponentsRotation(bool bValue) { bSaveRots = bValue; }
	/**
	* Get the value of bSaveComponentsScale
	* If true, the owner Actor will auto save and load the Scale3D Data for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponentsScale() const { return bSaveScals; }
	/**
	* Set the value of bSaveComponentsScale
	* If true, the owner Actor will auto save and load the Scale3D Data for all its SceneComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponentsScale(bool bValue) { bSaveScals = bValue; }
	/**
	* Get the value of bSaveComponentsLinearVelocity
	* If true, the owner Actor will auto save and load the Linear Velocity for all its PrimitiveComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponentsLinearVelocity() const { return bSaveLVel; }
	/**
	* Set the value of bSaveComponentsLinearVelocity
	* If true, the owner Actor will auto save and load the Linear Velocity for all its PrimitiveComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponentsLinearVelocity(bool bValue) { bSaveLVel = bValue; }
	/**
	* Get the value of bSaveComponentsAngularVelocity
	* If true, the owner Actor will auto save and load the Angular Velocity for all its PrimitiveComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		bool GetSaveComponentsAngularVelocity() const { return bSaveAVel; }
	/**
	* Set the value of bSaveComponentsAngularVelocity
	* If true, the owner Actor will auto save and load the Angular Velocity for all its PrimitiveComponents
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetSaveComponentsAngularVelocity(bool bValue) { bSaveAVel = bValue; }

	///****************************************************************************************************************************************************
	///	CATEGORY CSWAutoSaveAndLoadSystem::Custom Components
	///****************************************************************************************************************************************************

	/**
	* Get the Array of ComponentOptions for the owner Actor of this component.
	* 
	* Custom Options for an specific component of the owner Actor.
	* These custom options will override the Default Components Options.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		TArray<FCSWAutoSaveComponentOption> GetComponentOptions() const { return Optns; }
	/**
	* Set the Array of ComponentOptions for the owner Actor of this component.
	*
	* Custom Options for an specific component of the owner Actor.
	* These custom options will override the Default Components Options.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|AutosaveComponent")
		void SetComponentOptions(TArray<FCSWAutoSaveComponentOption> Options) { Optns = Options; }
#pragma endregion


#pragma region Enums|Structs|Events|Others
protected:
	/**
	* Event Triggered At the Start of Saving (The Moment When AutoFillSaveGameObject() is called) for every actor that has this component
	* Changes in the owner actor of this component (and all of its components) will BE saved if done in this event.
	* Use this event for saving custom data (example: call the functions SetSaved...()).
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On Begin Save"))
		FCSWAutoSaveComponentDelegate EventOnSaveStart;

	/**
	* Event Triggered At the End of Saving (After Serializing all the actors data)
	* Changes in the owner actor of this component (and all of its components) will NOT BE saved if done in this event
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On End Save"))
		FCSWAutoSaveComponentDelegate EventOnSaveEnd;

	/**
	* Event Triggered At the Start of Loading (SaveGame variables and components DOES NOT have updated values).
	* At this moment, the owner actor of this component DOES NOT have the updated values yet but the actor already exists
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On Begin Load"))
		FCSWAutoSaveComponentDelegate EventOnLoadStart;

	/**
	* Event Triggered At the End of Loading
	* At this moment, the owner actor of this component DOES have the updated values and the load is Completed.
	* Use this event for loading custom data  (example: call the functions GetSaved...()).
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On End Load"))
		FCSWAutoSaveComponentDelegate EventOnLoadEnd;

	/**
	* Event Triggered when loading the game (Function AutoLoadActorsDataFromSave()) AND if the owner Actor of this component wasn't previosly saved AND if bDestroyActorOnLoadGameIfWasNotSaved property was set to TRUE 
	* This event can be used to abort the destruction of the owner Actor of this component by setting bDestroyActorOnLoadGameIfWasNotSaved back to FALSE
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On Destroy Actor On Load If Wasn't Saved"))
		FCSWAutoSaveComponentDelegate EventBeginDestroyOnLoad;

	/**
	* Event Triggered when loading the game (Function AutoLoadActorsDataFromSave()) AND if the owner Actor of this component wasn't previosly saved AND if bDestroyActorOnLoadGameIfWasNotSaved property was set to FALSE
	*/
	UPROPERTY(BlueprintAssignable, Category = "CSW|AutosaveComponent", meta = (DisplayName = "CSW::On Actor Unchanged After Load"))
		FCSWAutoSaveComponentDelegate EventUnchangedOnLoad;
#pragma endregion
};
