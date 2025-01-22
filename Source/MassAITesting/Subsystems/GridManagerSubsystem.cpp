// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UGridManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TArray<AActor*> GridActors;
	UGameplayStatics::GetAllActorsWithTag(&InWorld, FName("GridActor"), GridActors);
	if (GridActors.IsEmpty()) { return; }

	GridMesh = GridActors[0]->GetComponentByClass<UInstancedStaticMeshComponent>();
	check(GridMesh);
}

void UGridManagerSubsystem::GetNearbyNodes(const FVector& Position, TArray<int32>& OutNodes, float Range) const
{
	if (!GridMesh) return;
	OutNodes.Empty();

	auto Area = FBox(Position-Range+(FVector::DownVector*2000.f),Position+Range+(FVector::UpVector*2000.f));
	OutNodes = GridMesh->GetInstancesOverlappingBox(Area, true);
}

void UGridManagerSubsystem::AddClaimedNode(int32 Node)
{
	ClaimedNodes.Add(Node);
}

void UGridManagerSubsystem::RemoveClaimedNode(int32 Node)
{
	ClaimedNodes.Remove(Node);
}
