// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassAITestingGameMode.h"
#include "MassAITestingCharacter.h"
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
