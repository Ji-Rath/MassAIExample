// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericSmartObject.h"
#include "GameFramework/Actor.h"
#include "ResourceBase.generated.h"

UCLASS()
class WORLDRESOURCES_API AResourceBase : public AGenericSmartObject
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FDataTableRowHandle ResourceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Floors = 1;
};
