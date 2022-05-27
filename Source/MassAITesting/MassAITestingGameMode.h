// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MassAITestingGameMode.generated.h"

UCLASS(minimalapi)
class AMassAITestingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMassAITestingGameMode();

	void BeginPlay() override;
};



