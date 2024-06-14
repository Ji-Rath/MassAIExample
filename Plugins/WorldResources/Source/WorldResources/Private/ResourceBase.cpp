// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBase.h"

#include "ResourceData.h"
#include "SmartObjectComponent.h"
#include "SmartObjectTypes.h"


// Sets default values
AResourceBase::AResourceBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AResourceBase::BeginPlay()
{
	if (auto Resource = ResourceData.GetRow<FResourceData>(TEXT("Resource Base Set SO")))
	{
		SOComponent->SetDefinition(Resource->SODefinition);
	}
	Super::BeginPlay();
}

void AResourceBase::AddToResourceQueue()
{
	FSmartObjectHandle SOHandle = SOComponent->GetRegisteredHandle();
	//GetWorld()->GetSubsystem<URTSBuildingSubsystem>()->AddResourceQueue(SOHandle);
}

