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

FVector UGridManagerSubsystem::GetClosestNode(const FVector& Position) const
{
	if (!GridMesh) return FVector::ZeroVector;

	TArray<int32> GridNodes = GridMesh->GetInstancesOverlappingSphere(Position, 1000.f, true);

	float ClosestDistance = 10000.f;
	FVector ClosestPosition;
	for (int32 GridNode : GridNodes)
	{
		FTransform NodeTransform;
		GridMesh->GetInstanceTransform(GridNode, NodeTransform, true);
		float Dist = FVector::Dist(NodeTransform.GetLocation(), Position);
		if (Dist < ClosestDistance)
		{
			ClosestPosition = NodeTransform.GetLocation();
			ClosestDistance = Dist;
		}
	}

	return ClosestPosition;
}
