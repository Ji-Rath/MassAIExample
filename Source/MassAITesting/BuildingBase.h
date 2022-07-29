// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericSmartObject.h"
#include "BuildingBase.generated.h"

UCLASS()
class MASSAITESTING_API ABuildingBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildingBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Floors = 1;

	int CurrentFloor = 0;
};
