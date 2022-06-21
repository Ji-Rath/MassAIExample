// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBase.h"

#include "RTSBuildingSubsystem.h"


// Sets default values
AResourceBase::AResourceBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SmartObjectComp = CreateDefaultSubobject<USmartObjectComponent>(TEXT("Smart Object Component"));
	SetRootComponent(SmartObjectComp);
}

// Called when the game starts or when spawned
void AResourceBase::BeginPlay()
{
	Super::BeginPlay();
}

void AResourceBase::AddToResourceQueue()
{
	FSmartObjectHandle SOHandle = SmartObjectComp->GetRegisteredHandle();
	GetWorld()->GetSubsystem<URTSBuildingSubsystem>()->AddResourceQueue(SOHandle);
}

// Called every frame
void AResourceBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

