// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingBase.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Building_Constructed, "Building.Constructed")

// Sets default values
ABuildingBase::ABuildingBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ABuildingBase::BeginPlay()
{
	Super::BeginPlay();
}

