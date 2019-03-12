/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#include "ActorComponent/CSWStorerComponent.h"

UCSWStorerComponent::UCSWStorerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSWStorerComponent::GetSavedNames(TArray<FName>& SavedNames)
{
	SavedNames = Names;
}

void UCSWStorerComponent::GetSavedStrings(TArray<FString>& SavedStrings)
{
	SavedStrings = Strings;
}

void UCSWStorerComponent::GetSavedTexts(TArray<FText>& SavedTexts)
{
	SavedTexts = Texts;
}

void UCSWStorerComponent::GetSavedFloats(TArray<float>& SavedFloats)
{
	SavedFloats = Floats;
}

void UCSWStorerComponent::GetSavedIntegers(TArray<int32>& SavedIntegers)
{
	SavedIntegers = Ints;
}

void UCSWStorerComponent::GetSavedBooleans(TArray<bool>& SavedBooleans)
{
	SavedBooleans = Bools;
}

void UCSWStorerComponent::GetSavedBytes(TArray<uint8>& SavedBytes)
{
	SavedBytes = Bytes;
}

void UCSWStorerComponent::GetSavedVectors(TArray<FVector>& SavedVectors)
{
	SavedVectors = Vects;
}

void UCSWStorerComponent::GetSavedRotators(TArray<FRotator>& SavedRotators)
{
	SavedRotators = Rots;
}

void UCSWStorerComponent::GetSavedTransforms(TArray<FTransform>& SavedTransforms)
{
	SavedTransforms = Xforms;
}

void UCSWStorerComponent::GetSavedPersistentReferences(TArray<UObject*>& SavedReferences)
{
	SavedReferences = Refs;
}

void UCSWStorerComponent::SetSavedNames(const TArray<FName>& NamesToSave)
{
	Names = NamesToSave;
}

void UCSWStorerComponent::SetSavedStrings(const TArray<FString>& StringToSaves)
{
	Strings = StringToSaves;
}

void UCSWStorerComponent::SetSavedTexts(const TArray<FText>& TextsToSave)
{
	Texts = TextsToSave;
}

void UCSWStorerComponent::SetSavedFloats(const TArray<float>& FloatsToSave)
{
	Floats = FloatsToSave;
}

void UCSWStorerComponent::SetSavedIntegers(const TArray<int32>& IntegersToSave)
{
	Ints = IntegersToSave;
}

void UCSWStorerComponent::SetSavedBooleans(const TArray<bool>& BooleansToSave)
{
	Bools = BooleansToSave;
}

void UCSWStorerComponent::SetSavedBytes(const TArray<uint8>& BytesToSave)
{
	Bytes = BytesToSave;
}

void UCSWStorerComponent::SetSavedVectors(const TArray<FVector>& VectorsToSave)
{
	Vects = VectorsToSave;
}

void UCSWStorerComponent::SetSavedRotators(const TArray<FRotator>& RotatorsToSave)
{
	Rots = RotatorsToSave;
}

void UCSWStorerComponent::SetSavedTransforms(const TArray<FTransform>& TransformsToSave)
{
	Xforms = TransformsToSave;
}

void UCSWStorerComponent::SetSavedPersistentReferences(const TArray<UObject*>& ReferencesToSave)
{
	Refs = ReferencesToSave;
}
