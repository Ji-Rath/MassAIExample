// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingBase.h"

#include "RTSBuildingSubsystem.h"
#include "SmartObjectComponent.h"


// Sets default values
ABuildingBase::ABuildingBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABuildingBase::BeginPlay()
{
	Super::BeginPlay();

	// We want to set a small delay because the smart object does not register instantly
	FTimerDelegate TimerDelegate;
	FTimerHandle TimerHandle;
	TimerDelegate.BindLambda([this]()
	{
		const USmartObjectComponent* SOComponent = FindComponentByClass<USmartObjectComponent>();
		if(!ensure(SOComponent)) { return; }

		GetWorld()->GetSubsystem<URTSBuildingSubsystem>()->AddBuilding(SOComponent->GetRegisteredHandle(), Floors);
	});
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
	
	
}

// Called every frame
void ABuildingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

