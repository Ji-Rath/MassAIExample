// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassAITestingGameMode.h"
#include "MassAITestingCharacter.h"
#include "SmartObjectComponent.h"
#include "SmartObjectSubsystem.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AMassAITestingGameMode::AMassAITestingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AMassAITestingGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Iterate over all actors, can also supply a different base class if needed
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Follow iterator object to my actual actor pointer
		AActor* MyActor = *ActorItr;

		// Essentially a hacky fix until i can find a better way to get smart objects to work without being spawned at runtime
		// TODO: Find solution to non-runtime spawned smart objects not being found through FindSmartObject
		if(USmartObjectComponent* SmartObjectComp = MyActor->FindComponentByClass<USmartObjectComponent>())
		{
			if (USmartObjectSubsystem* SmartObjectSubsystem = GetWorld()->GetSubsystem<USmartObjectSubsystem>())
			{
				//SmartObjectSubsystem->UnregisterSmartObject(*SmartObjectComp);
				//SmartObjectSubsystem->RegisterSmartObject(*SmartObjectComp);
			}
		}
	} 
}
