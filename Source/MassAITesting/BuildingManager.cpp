// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingManager.h"

#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
ABuildingManager::ABuildingManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
	MidISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MidISM"));
	RoofISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoofISM"));
}

// Called when the game starts or when spawned
void ABuildingManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

