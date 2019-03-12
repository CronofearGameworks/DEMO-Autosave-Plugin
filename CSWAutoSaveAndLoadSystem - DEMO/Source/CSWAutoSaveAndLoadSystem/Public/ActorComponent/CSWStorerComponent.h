/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#pragma once

#include "Components/ActorComponent.h"
#include "CSWStorerComponent.generated.h"

/**
* Aux component, part of the CSW::AutoSaveModule.
* By default, this component is used to store primitive values (booleans, strings, etc.) into arrays by using GetSaved...() and SetSaved...() functions.
* This can be convenient as it eliminates the necessity of creating variables that will only be used for saving/loading values.
* BUT, as this method uses arrays, it can be hard to read.
*
* This component is also VERY USEFUL for saving blueprint structures:
* - Create a Blueprint Component based on this class.
* - Open the BP Component and add BP structures variables or any variable you want to save.
* - Add the BP Component to an Actor. Use the component to access the BP structure variables, modify them, etc.
* - When the game is saved and loaded, the BP component will have the ability to also save/load the BP structures automatically.
*/
UCLASS(Blueprintable, BlueprintType, ClassGroup = ("CSW::AutoSaveModule"), meta = (BlueprintSpawnableComponent, DisplayName = "CSWStorerComponent"))
class CSWAUTOSAVEANDLOADSYSTEM_API UCSWStorerComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region BaseFunctions
protected:
	// Sets default values for this component's properties
	UCSWStorerComponent();
#pragma endregion

private:
	///****************************************************************************************************************************************************
	///	CONVENIENT WAY OF SAVING AND LOADING VARIABLES
	///****************************************************************************************************************************************************

	/**
	* This Array of Booleans can be used to save and load booleans from the owner Actor of this component calling the functions GetSavedBooleans() and SetSavedBooleans()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Booleans is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBooleans() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Booleans"))
		TArray<bool> Bools;

	/**
	* This Array of Bytes can be used to save and load bytes from the owner Actor of this component calling the functions GetSavedBytes() and SetSavedBytes()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Bytes is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBytes() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Bytes"))
		TArray<uint8> Bytes;

	/**
	* This Array of Integers can be used to save and load Integers from the owner Actor of this component calling the functions GetSavedIntegers() and SetSavedIntegers()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Integers is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedIntegers() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Integers"))
		TArray<int32> Ints;

	/**
	* This Array of Floats can be used to save and load Floats from the owner Actor of this component calling the functions GetSavedFloats() and SetSavedFloats()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Floats is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedFloats() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Floats"))
		TArray<float> Floats;

	/**
	* This Array of Names can be used to save and load Names from the owner Actor of this component calling the functions GetSavedNames() and SetSavedNames()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Names is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedNames() can be used to store the data.
	*/
	UPROPERTY(SaveGame)
		TArray<FName> Names;

	/**
	* This Array of Strings can be used to save and load Strings from the owner Actor of this component calling the functions GetSavedStrings() and SetSavedStrings()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Strings is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedStrings() can be used to store the data.
	*/
	UPROPERTY(SaveGame)
		TArray<FString> Strings;

	/**
	* This Array of Texts can be used to save and load Texts from the owner Actor of this component calling the functions GetSavedTexts() and SetSavedTexts()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Texts is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTexts() can be used to store the data.
	*/
	UPROPERTY(SaveGame)
		TArray<FText> Texts;

	/**
	* This Array of Vectors can be used to save and load Vectors from the owner Actor of this component calling the functions GetSavedVectors() and SetSavedVectors()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Vectors is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedVectors() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Vectors"))
		TArray<FVector> Vects;

	/**
	* This Array of Rotators can be used to save and load Rotators from the owner Actor of this component calling the functions GetSavedRotators() and SetSavedRotators()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Rotators is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedRotators() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Rotators"))
		TArray<FRotator> Rots;

	/**
	* This Array of Transforms can be used to save and load Transforms from the owner Actor of this component calling the functions GetSavedTransforms() and SetSavedTransforms()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Transforms is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTransforms() can be used to store the data.
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Transforms"))
		TArray<FTransform> Xforms;

	/**
	* This Array of PersistentReferences can be used to save and load PersistentReferences from the owner Actor of this component calling the functions GetSavedPersistentReferences() and SetSavedPersistentReferences()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n PersistentReferences is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedPersistentReferences() can be used to store the data.
	*
	* Caution: Saving runtime generated references is not recommended. This array is intended to be used to save Persistent References only (Example: Actors placed in the world while in Editor Mode)
	*/
	UPROPERTY(SaveGame, meta = (DisplayName = "Persistent Object References"))
		TArray<UObject*> Refs;

public:
	///****************************************************************************************************************************************************
	///	CONVENIENT WAY OF SAVING AND LOADING VARIABLES
	///****************************************************************************************************************************************************

	/**
	* Get the array of SavedBooleans from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Booleans can be used to save and load booleans from the owner Actor of this component calling the functions GetSavedBooleans() and SetSavedBooleans()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Booleans is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBooleans() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedBooleans(TArray<bool>& SavedBooleans);
	/**
	* Get the array of SavedBytes from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Bytes can be used to save and load bytes from the owner Actor of this component calling the functions GetSavedBytes() and SetSavedBytes()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Bytes is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBytes() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedBytes(TArray<uint8>& SavedBytes);
	/**
	* Get the array of SavedIntegers from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Integers can be used to save and load Integers from the owner Actor of this component calling the functions GetSavedIntegers() and SetSavedIntegers()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Integers is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedIntegers() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedIntegers(TArray<int32>& SavedIntegers);
	/**
	* Get the array of SavedFloats from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Floats can be used to save and load Floats from the owner Actor of this component calling the functions GetSavedFloats() and SetSavedFloats()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Floats is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedFloats() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedFloats(TArray<float>& SavedFloats);
	/**
	* Get the array of SavedNames from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Names can be used to save and load Names from the owner Actor of this component calling the functions GetSavedNames() and SetSavedNames()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Names is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedNames() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedNames(TArray<FName>& SavedNames);
	/**
	* Get the array of SavedStrings from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Strings can be used to save and load Strings from the owner Actor of this component calling the functions GetSavedStrings() and SetSavedStrings()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Strings is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedStrings() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedStrings(TArray<FString>& SavedStrings);
	/**
	* Get the array of SavedTexts from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Texts can be used to save and load Texts from the owner Actor of this component calling the functions GetSavedTexts() and SetSavedTexts()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Texts is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTexts() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedTexts(TArray<FText>& SavedTexts);
	/**
	* Get the array of SavedVectors from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Vectors can be used to save and load Vectors from the owner Actor of this component calling the functions GetSavedVectors() and SetSavedVectors()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Vectors is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedVectors() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedVectors(TArray<FVector>& SavedVectors);
	/**
	* Get the array of SavedRotators from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Rotators can be used to save and load Rotators from the owner Actor of this component calling the functions GetSavedRotators() and SetSavedRotators()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Rotators is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedRotators() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedRotators(TArray<FRotator>& SavedRotators);
	/**
	* Get the array of SavedTransforms from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Transforms can be used to save and load Transforms from the owner Actor of this component calling the functions GetSavedTransforms() and SetSavedTransforms()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Transforms is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTransforms() can be used to store the data.
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedTransforms(TArray<FTransform>& SavedTransforms);
	/**
	* Get the array of SavedReferences from this component (these variables already have the SaveGame flag Enabled).
	* This Array of PersistentReferences can be used to save and load PersistentReferences from the owner Actor of this component calling the functions GetSavedPersistentReferences() and SetSavedPersistentReferences()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n PersistentReferences is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedPersistentReferences() can be used to store the data.
	*
	* Caution: Saving runtime generated references is not recommended. This array is intended to be used to save Persistent References only (Example: Actors placed in the world while in Editor Mode)
	*/
	UFUNCTION(BlueprintPure, Category = "CSW|StorerComponent")
		void GetSavedPersistentReferences(TArray<UObject*>& SavedReferences);

	/**
	* Set the array of SavedBooleans from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Booleans can be used to save and load booleans from the owner Actor of this component calling the functions SetSavedBooleans() and SetSavedBooleans()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Booleans is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBooleans() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "BooleansToSave"))
		void SetSavedBooleans(const TArray<bool>& BooleansToSave);
	/**
	* Set the array of SavedBytes from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Bytes can be used to save and load bytes from the owner Actor of this component calling the functions SetSavedBytes() and SetSavedBytes()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Bytes is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedBytes() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "BytesToSave"))
		void SetSavedBytes(const TArray<uint8>& BytesToSave);
	/**
	* Set the array of SavedIntegers from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Integers can be used to save and load Integers from the owner Actor of this component calling the functions SetSavedIntegers() and SetSavedIntegers()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Integers is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedIntegers() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "IntegersToSave"))
		void SetSavedIntegers(const TArray<int32>& IntegersToSave);
	/**
	* Set the array of SavedFloats from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Floats can be used to save and load Floats from the owner Actor of this component calling the functions SetSavedFloats() and SetSavedFloats()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Floats is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedFloats() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "FloatsToSave"))
		void SetSavedFloats(const TArray<float>& FloatsToSave);
	/**
	* Set the array of SavedNames from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Names can be used to save and load Names from the owner Actor of this component calling the functions SetSavedNames() and SetSavedNames()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Names is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedNames() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "NamesToSave"))
		void SetSavedNames(const TArray<FName>& NamesToSave);
	/**
	* Set the array of SavedStrings from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Strings can be used to save and load Strings from the owner Actor of this component calling the functions SetSavedStrings() and SetSavedStrings()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Strings is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedStrings() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "StringToSaves"))
		void SetSavedStrings(const TArray<FString>& StringToSaves);
	/**
	* Set the array of SavedTexts from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Texts can be used to save and load Texts from the owner Actor of this component calling the functions SetSavedTexts() and SetSavedTexts()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Texts is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTexts() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "TextsToSave"))
		void SetSavedTexts(const TArray<FText>& TextsToSave);
	/**
	* Set the array of SavedVectors from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Vectors can be used to save and load Vectors from the owner Actor of this component calling the functions SetSavedVectors() and SetSavedVectors()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Vectors is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedVectors() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "VectorsToSave"))
		void SetSavedVectors(const TArray<FVector>& VectorsToSave);
	/**
	* Set the array of SavedRotators from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Rotators can be used to save and load Rotators from the owner Actor of this component calling the functions SetSavedRotators() and SetSavedRotators()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Rotators is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedRotators() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "RotatorsToSave"))
		void SetSavedRotators(const TArray<FRotator>& RotatorsToSave);
	/**
	* Set the array of SavedTransforms from this component (these variables already have the SaveGame flag Enabled).
	* This Array of Transforms can be used to save and load Transforms from the owner Actor of this component calling the functions SetSavedTransforms() and SetSavedTransforms()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n Transforms is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedTransforms() can be used to store the data.
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "TransformsToSave"))
		void SetSavedTransforms(const TArray<FTransform>& TransformsToSave);
	/**
	* Set the array of SavedReferences from this component (these variables already have the SaveGame flag Enabled).
	* This Array of PersistentReferences can be used to save and load PersistentReferences from the owner Actor of this component calling the functions SetSavedPersistentReferences() and SetSavedPersistentReferences()
	* The use of this Array is optional, but very convenient as it allows to keep things cleaner in the Blueprint Editor.
	* Example, if saving n PersistentReferences is needed, Instead of creating n variables inside the owner Actor of this component, SetSavedPersistentReferences() can be used to store the data.
	*
	* Caution: Saving runtime generated references is not recommended. This array is intended to be used to save Persistent References only (Example: Actors placed in the world while in Editor Mode)
	*/
	UFUNCTION(BlueprintCallable, Category = "CSW|StorerComponent", meta = (AutoCreateRefTerm = "ReferencesToSave"))
		void SetSavedPersistentReferences(const TArray<UObject*>& ReferencesToSave);
};
