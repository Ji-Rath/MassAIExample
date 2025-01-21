// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UGridManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TArray<AActor*> GridActors;
	UGameplayStatics::GetAllActorsWithTag(&InWorld, FName("GridActor"), GridActors);
	check(!GridActors.IsEmpty());

	GridMesh = GridActors[0]->GetComponentByClass<UInstancedStaticMeshComponent>();
	check(GridMesh);
}

void UGridManagerSubsystem::GetNearbyNodes(const FVector& Position, TArray<int32>& OutNodes) const
{
	if (!GridMesh) return;
	OutNodes.Empty();

	auto Area = FBox(Position-300.f+(FVector::DownVector*2000.f),Position+300.f+(FVector::UpVector*2000.f));
	OutNodes = GridMesh->GetInstancesOverlappingBox(Area, true);
}
