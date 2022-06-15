// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingBase.h"

#include "RTSBuildingSubsystem.h"


// Sets default values
ABuildingBase::ABuildingBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SmartObjectComp = CreateDefaultSubobject<USmartObjectComponent>(TEXT("Smart Object Component"));
	SetRootComponent(SmartObjectComp);
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
		UE_LOG(LogTemp, Error, TEXT("Handle Valid: %s"), SmartObjectComp->GetRegisteredHandle().IsValid() ? TEXT("TRUE") : TEXT("FALSE"));
		GetWorld()->GetSubsystem<URTSBuildingSubsystem>()->AddBuilding(SmartObjectComp->GetRegisteredHandle(), 4);
	});
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
	
	
}

// Called every frame
void ABuildingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

