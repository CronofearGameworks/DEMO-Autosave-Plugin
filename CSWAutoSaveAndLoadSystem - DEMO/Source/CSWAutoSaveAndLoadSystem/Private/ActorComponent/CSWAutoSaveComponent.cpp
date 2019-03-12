/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

#include "ActorComponent/CSWAutoSaveComponent.h"

// Sets default values for this component's properties
UCSWAutoSaveComponent::UCSWAutoSaveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UCSWAutoSaveComponent::OnSaveStart(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	SetWasSaved(true);
	EventOnSaveStart.Broadcast(CSWAutoSaveObject);
}

void UCSWAutoSaveComponent::OnSaveEnd(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	EventOnSaveEnd.Broadcast(CSWAutoSaveObject);
}

void UCSWAutoSaveComponent::OnLoadStart(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	EventOnLoadStart.Broadcast(CSWAutoSaveObject);
}

void UCSWAutoSaveComponent::OnLoadEnd(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	EventOnLoadEnd.Broadcast(CSWAutoSaveObject);
}

void UCSWAutoSaveComponent::OnBeginDestroyUnsavedActor(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	EventBeginDestroyOnLoad.Broadcast(CSWAutoSaveObject);
}

void UCSWAutoSaveComponent::OnUnchangedActor(const UCSWAutoSaveObject* CSWAutoSaveObject)
{
	EventUnchangedOnLoad.Broadcast(CSWAutoSaveObject);
}
