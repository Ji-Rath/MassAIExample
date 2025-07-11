// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "DrawDebugHelpers.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntityBuilder.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassObserverNotificationTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSFormationProcessors.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Unit/UnitFragments.h"

void URTSFormationSubsystem::DestroyEntity(UMassAgentComponent* Entity)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySubsystem);
	
	EntitySubsystem->GetEntityManager().Defer().DestroyEntity(Entity->GetEntityHandle());
}

void URTSFormationSubsystem::CalculateNewPositions(const FMassEntityHandle& UnitHandle, TMap<int, FVector>& OutNewPositions)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);
	
	// Empty NewPositions Map to make room for new calculations
	OutNewPositions.Empty(UnitFragment.Entities.Num());
	
	// Calculate entity positions for new destination
	// This is the logic that can change formation types
	const FVector CenterOffset = FVector((UnitFragment.Entities.Num()/UnitFragment.FormationLength/2) * UnitFragment.BufferDistance, (UnitFragment.FormationLength/2) * UnitFragment.BufferDistance, 0.f);
	int PlacedUnits = 0;
	int PosIndex = 0;
	while (PlacedUnits < UnitFragment.Entities.Num())
	{
		float w = PosIndex / UnitFragment.FormationLength;
		float l = PosIndex % UnitFragment.FormationLength;

		// Hollow formation logic (2 layers)
		if (UnitFragment.bHollow && UnitFragment.Formation == EFormationType::Rectangle)
		{
			int Switch = UnitFragment.Entities.Num() - UnitFragment.FormationLength*2;
			if (w != 0 && w != 1 && !(PlacedUnits >= Switch)
				&& l != 0 && l != 1 && l != UnitFragment.FormationLength-1 && l != UnitFragment.FormationLength-2)
			{
				PosIndex++;
				continue;
			}
		}

		// Circle formation
		if (UnitFragment.Formation == EFormationType::Circle)
		{
			int AmountPerRing = UnitFragment.Entities.Num() / UnitFragment.Rings;
			float Angle = PosIndex * PI * 2 / AmountPerRing;
			float Radius = UnitFragment.FormationLength + (PosIndex / AmountPerRing * 1.5f);
			w = FMath::Cos(Angle) * Radius;
			l = FMath::Sin(Angle) * Radius;
		}
		
		PlacedUnits++;
		FVector Position = FVector(w,l,0.f);
		Position *= UnitFragment.BufferDistance;
		if (UnitFragment.Formation == EFormationType::Rectangle)
			Position -= CenterOffset;
		
		Position = Position.RotateAngleAxis(UnitFragment.InterpRotation.Yaw+180.f, FVector(0.f,0.f,1.f));
		
		OutNewPositions.Add(PosIndex, Position);
		//DrawDebugPoint(GetWorld(), Position+Unit.InterpolatedDestination, 20.f, FColor::Yellow, false, 10.f);
		PosIndex++;
	}
}

void URTSFormationSubsystem::UpdateUnitPosition(const FVector& NewPosition, const FMassEntityHandle& UnitHandle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UpdateUnitPosition"))
	if (!ensure(UnitHandle.IsValid())) { return; }

	// Convenience variables
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);
	TMap<int, FVector>& NewPositions = UnitFragment.NewPositions;

	// Calculate new positions for entities and output to NewPositions
	CalculateNewPositions(UnitHandle, NewPositions);

	// Calculate far corner by finding the new position that is furthest from the unit destination
	UnitFragment.FarCorner = NewPosition;
	NewPositions.ValueSort([&UnitFragment, &NewPosition](const FVector& A, const FVector& B)
	{
		return FVector::DistSquared2D(A+UnitFragment.InterpolatedDestination, NewPosition) > FVector::DistSquared2D(B+UnitFragment.InterpolatedDestination, NewPosition);
	});
	
	if (NewPositions.Num())
	{
		TArray<FVector> NewArray;
		NewArray.Reserve(NewPositions.Num());
		NewPositions.GenerateValueArray(NewArray);
		UnitFragment.FarCorner = NewArray[0];
	}
	//DrawDebugPoint(GetWorld(), Unit.FarCorner+Unit.InterpolatedDestination, 30.f, FColor::Green, false, 5.f);

	// Sort entities by distance to the far corner location
	UnitFragment.Entities.Sort([&EntitySubsystem, &UnitFragment](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		//@todo Find if theres a way to move this logic to a processor, most of the cost is coming from retrieving the location
		const FVector& LocA = EntitySubsystem->GetEntityManager().GetFragmentDataChecked<FTransformFragment>(A).GetTransform().GetLocation();
		const FVector& LocB = EntitySubsystem->GetEntityManager().GetFragmentDataChecked<FTransformFragment>(B).GetTransform().GetLocation();
		return FVector::DistSquared2D(LocA, UnitFragment.FarCorner+UnitFragment.InterpolatedDestination) > FVector::DistSquared2D(LocB, UnitFragment.FarCorner+UnitFragment.InterpolatedDestination);
	});

	// Sort new positions by distance to the far corner location
	NewPositions.ValueSort([&UnitFragment](const FVector& A, const FVector& B)
	{
		return FVector::DistSquared2D(A, UnitFragment.FarCorner) > FVector::DistSquared2D(B, UnitFragment.FarCorner);
	});

	// Signal entities to update their position index
	if (UnitFragment.Entities.Num())
	{
		TArray<FMassEntityHandle> Entities = UnitFragment.Entities.Array();
		GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(UpdateIndex, Entities);
	}
}

void URTSFormationSubsystem::SetUnitPositionByIndex(const FVector& NewPosition, int UnitIndex)
{
	SetUnitPosition(NewPosition, Units[UnitIndex]);
}

void URTSFormationSubsystem::MoveEntities(const FMassEntityHandle& UnitHandle)
{
	SCOPED_NAMED_EVENT(STAT_RTS_MoveEntities, FColor::Green);

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);
	
	// Final sort to ensure that entities are signaled from front to back
	UnitFragment.Entities.Sort([&EntitySubsystem, &UnitFragment](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		// Find if theres a way to move this logic to a processor, most of the cost is coming from retrieving the location
		const FVector& LocA = EntitySubsystem->GetEntityManager().GetFragmentDataChecked<FRTSFormationAgent>(A).Offset;
		const FVector& LocB = EntitySubsystem->GetEntityManager().GetFragmentDataChecked<FRTSFormationAgent>(B).Offset;
		return FVector::DistSquared2D(LocA, UnitFragment.FarCorner) > FVector::DistSquared2D(LocB, UnitFragment.FarCorner);
	});
	
	CurrentIndex = 0;

	// Signal entities to begin moving
	TArray<FMassEntityHandle> Entities = UnitFragment.Entities.Array();
	for(int i=0;i<UnitFragment.Entities.Num();++i)
	{
		GetWorld()->GetSubsystem<UMassSignalSubsystem>()->DelaySignalEntity(FormationUpdated, Entities[i], 0.1*(i/UnitFragment.FormationLength));
	} 
}

void URTSFormationSubsystem::SpawnEntitiesForUnitByIndex(int UnitIndex, const UMassEntityConfigAsset* EntityConfig,
	int Count)
{
	if (ensure(Units.IsValidIndex(UnitIndex)))
	{
		SpawnEntitiesForUnit(Units[UnitIndex], EntityConfig, Count);
	}
}

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, const FMassEntityHandle& UnitHandle)
{
	if (Units.IsEmpty()) { return; }
	
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);

	DrawDebugDirectionalArrow(GetWorld(), NewPosition, NewPosition+((NewPosition-UnitFragment.InterpolatedDestination).GetSafeNormal()*250.f), 150.f, FColor::Red, false, 5.f, 0, 25.f);

	FVector OldDir = UnitFragment.ForwardDir;
	UnitFragment.ForwardDir = (NewPosition-UnitFragment.InterpolatedDestination).GetSafeNormal();
	
	// Calculate turn direction and angle for entities in unit
	UnitFragment.TurnDirection = UnitFragment.ForwardDir.Y > 0 ? 1.f : -1.f;
	
	UnitFragment.OldRotation = UnitFragment.Rotation;
	UnitFragment.Rotation = UKismetMathLibrary::MakeRotFromX(UnitFragment.ForwardDir);
	
	UnitFragment.bBlendAngle = OldDir.Dot(UnitFragment.ForwardDir) > 0.4;
	UnitFragment.InterpRotation = UnitFragment.bBlendAngle ? UnitFragment.InterpRotation : UnitFragment.Rotation;

	// Jank solution to stop entities from moving
	for(const FMassEntityHandle& Entity : UnitFragment.Entities)
	{
		if (EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassMoveTargetFragment>(Entity))
		{
			EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassMoveTargetFragment>(Entity)->CreateNewAction(EMassMovementAction::Stand, *GetWorld());
			EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FMassVelocityFragment>(Entity)->Value = FVector::Zero();
		}
	}
	
	UnitFragment.UnitPosition = NewPosition;
	
	UpdateUnitPosition(NewPosition, UnitHandle);
}

void URTSFormationSubsystem::SpawnEntitiesForUnit(const FMassEntityHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count)
{
	if (!ensure(UnitHandle.IsValid() && EntityConfig)) { return; }

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);

	// Reserve space for the new units, the space will be filled in a processor
	UnitFragment.Entities.Reserve(UnitFragment.Entities.Num()+Count);
	
	TArray<FMassEntityHandle> Entities;
	auto& EntityTemplate = EntityConfig->GetConfig().GetOrCreateEntityTemplate(*UGameplayStatics::GetPlayerPawn(this, 0)->GetWorld());

	// We are doing a little bit of work here since we are setting the unit index manually
	// Otherwise, using SpawnEntities would be perfectly fine
	// @todo find if there is a better way to modify templates in code
	TArray<FMassEntityHandle> SpawnedEntities;
	auto CreationContext = EntitySubsystem->GetMutableEntityManager().BatchCreateEntities(EntityTemplate.GetArchetype(), EntityTemplate.GetSharedFragmentValues(), Count, SpawnedEntities);

	// Set the template default values for the entities
	TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate.GetInitialFragmentValues();
	EntityManager.BatchSetEntityFragmentValues(CreationContext->GetEntityCollections(EntityManager), FragmentInstances);

	// Set unit index for entities
	FRTSFormationAgent FormationAgent;
	FormationAgent.UnitHandle = UnitHandle;
	
	TArray<FInstancedStruct> Fragments;
	Fragments.Add(FInstancedStruct::Make(FormationAgent));
	EntityManager.BatchSetEntityFragmentValues(CreationContext->GetEntityCollections(EntityManager), Fragments);
}

int URTSFormationSubsystem::SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position)
{
	int UnitIndex = Units.Num();

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	FUnitFragment UnitFragment = FUnitFragment();
	UnitFragment.UnitPosition = Position;
	
	UE::Mass::FEntityBuilder UnitEntity(EntityManager);
	UnitEntity.Add<FUnitTag>().Add<FUnitFragment>();
	auto EntityHandle = UnitEntity.Commit();

	Units.Emplace(EntityHandle);
	
	SpawnEntitiesForUnit(EntityHandle, EntityConfig, Count);
	return UnitIndex;
}

void URTSFormationSubsystem::SetFormationPresetByIndex(int EntityIndex, UFormationPresets* FormationAsset)
{
	if (ensure(Units.IsValidIndex(EntityIndex)))
	{
		SetFormationPreset(Units[EntityIndex], FormationAsset);
	}
}

void URTSFormationSubsystem::SetFormationPreset(const FMassEntityHandle& UnitHandle, UFormationPresets* FormationAsset)
{
	if (!ensure(FormationAsset && UnitHandle.IsValid())) { return; }

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);
	
	UnitFragment.FormationLength = FormationAsset->FormationLength;
	UnitFragment.BufferDistance = FormationAsset->BufferDistance;
	UnitFragment.Formation = FormationAsset->Formation;
	UnitFragment.Rings = FormationAsset->Rings;
	UnitFragment.bHollow = FormationAsset->bHollow;

	SetUnitPosition(UnitFragment.UnitPosition, UnitHandle);
}

void URTSFormationSubsystem::Tick(float DeltaTime)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();
	
	for(int i=0;i<Units.Num();++i)
	{
		auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(Units[i]);
		if (UnitFragment.Formation != EFormationType::Circle)
		{
			UnitFragment.InterpRotation = UKismetMathLibrary::RInterpTo(UnitFragment.InterpRotation, UnitFragment.Rotation, DeltaTime, 0.5f);
		}
		
		UnitFragment.InterpolatedDestination = FMath::VInterpConstantTo(UnitFragment.InterpolatedDestination, UnitFragment.UnitPosition, DeltaTime, 150.f);
	}
}

bool URTSFormationSubsystem::IsTickable() const
{
	return true;
}

TStatId URTSFormationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URTSFormationSubsystem, STATGROUP_Tickables);
}

void URTSFormationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	
}
