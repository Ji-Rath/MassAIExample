// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "StateTreeCharacter.generated.h"

UCLASS()
class STATETREEEXAMPLE_API AStateTreeCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AStateTreeCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(EditDefaultsOnly)
	FGenericTeamId TeamId;
public:
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override { TeamId = TeamID; }
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
};
