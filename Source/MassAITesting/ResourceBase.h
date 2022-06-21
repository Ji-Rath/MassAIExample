// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SmartObjectComponent.h"
#include "GameFramework/Actor.h"
#include "BuildingBase.generated.h"

UCLASS()
class MASSAITESTING_API AResourceBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AResourceBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void AddToResourceQueue();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USmartObjectComponent* SmartObjectComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Floors = 1;
};
