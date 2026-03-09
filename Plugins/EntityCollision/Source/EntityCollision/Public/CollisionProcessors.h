// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LineTypes.h"
#include "MassExecutionContext.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"

#include "CollisionProcessors.generated.h"


/**
 * When an entity is given the specified fragment, we do initialization so that the entity and position are added to the hash grid
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionInitializerProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UCollisionInitializerProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};

/**
 * When the fragment is removed from the entity, we assume that we no longer want the entity to be tracked by the hash grid. Do appropriate cleanup here
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionDestroyProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	UCollisionDestroyProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};

/**
 * This processor will mainly update the entities that are within the hash grid and update their position. This will allow for accurate queries when the entities move
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()
	UCollisionProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
	FMassEntityQuery AvoidanceQuery;

	FVector2f ResolveCollisions(const TConstArrayView<UE::Geometry::FLine2f>& OrcaLines, FMassExecutionContext& Context,
	                            const FVector
	                            & EntityLocation, const FVector& EntityVelocity,
	                            const FMassExecutionContext::FEntityIterator& EntityIt);

	/**
	* This function is adapted from the RVO2 Library.
	* Original Copyright 2008 University of North Carolina at Chapel Hill.
	* Licensed under the Apache License, Version 2.0.
	* Modified by Gareth Aguilar for Unreal Engine Mass integration.
	* @param Lines - The half-plane constraints
	* @param Radius - Max speed of the agent
	* @param OptVelocity - The preferred velocity we want to match
	* @param DirectionOpt - True if we only care about direction (usually false)
	* @param OutResult - The resulting collision-free velocity
	* @return True if a solution was found within the radius
	*/
	bool LinearProgram2(const TConstArrayView<UE::Geometry::FLine2f>& Lines, float Radius, FVector2f OptVelocity,
	                    bool DirectionOpt, FVector2f& OutResult);

	/**
	* This function is adapted from the RVO2 Library.
	* Original Copyright 2008 University of North Carolina at Chapel Hill.
	* Licensed under the Apache License, Version 2.0.
	* Modified by Gareth Aguilar for Unreal Engine Mass integration.
	*/
	bool LinearProgram1(const TConstArrayView<UE::Geometry::FLine2f>& Lines, int32 LineNo, float Radius, FVector2f OptVelocity,
	                    bool DirectionOpt, FVector2f& OutResult);
	
	// Adapted from MassAvoidanceProcessor - templated to support other grids
	template<typename Grid, typename GridCell, typename GridItem, typename ArrayItem, int32 MaxResults>
	static void FindCloseObstacles(const FVector& Center, const FVector::FReal SearchRadius, const Grid& AvoidanceObstacleGrid,
									TArray<ArrayItem, TInlineAllocator<MaxResults>>& OutCloseEntities)
	{
		OutCloseEntities.Reset();
		const FVector Extent(SearchRadius, SearchRadius, 0.);
		const FBox QueryBox = FBox(Center - Extent, Center + Extent);

		struct FSortingCell
		{
			int32 X;
			int32 Y;
			int32 Level;
			FVector::FReal SqDist;
		};
		TArray<FSortingCell, TInlineAllocator<64>> Cells;
		const FVector QueryCenter = QueryBox.GetCenter();
		
		for (int32 Level = 0; Level < AvoidanceObstacleGrid.NumLevels; Level++)
		{
			const FVector::FReal CellSize = AvoidanceObstacleGrid.GetCellSize(Level);
			const Grid::FCellRect Rect = AvoidanceObstacleGrid.CalcQueryBounds(QueryBox, Level);

			// Use int64 to prevent overflow when MaxX or MaxY are equal to the maximum value of int32.
			for (int64 Y = Rect.MinY; Y <= Rect.MaxY; Y++)
			{
				for (int64 X = Rect.MinX; X <= Rect.MaxX; X++)
				{
					const FVector::FReal CenterX = (X + 0.5) * CellSize;
					const FVector::FReal CenterY = (Y + 0.5) * CellSize;
					const FVector::FReal DX = CenterX - QueryCenter.X;
					const FVector::FReal DY = CenterY - QueryCenter.Y;
					const FVector::FReal SqDist = DX * DX + DY * DY;
					FSortingCell SortCell;
					SortCell.X = X;
					SortCell.Y = Y;
					SortCell.Level = Level;
					SortCell.SqDist = SqDist;
					Cells.Add(SortCell);
				}
			}
		}

		Cells.Sort([](const FSortingCell& A, const FSortingCell& B) { return A.SqDist < B.SqDist; });

		for (const FSortingCell& SortedCell : Cells)
		{
			if (const GridCell* Cell = AvoidanceObstacleGrid.FindCell(SortedCell.X, SortedCell.Y, SortedCell.Level))
			{
				const TSparseArray<GridItem>&  Items = AvoidanceObstacleGrid.GetItems();
				for (int32 Idx = Cell->First; Idx != INDEX_NONE; Idx = Items[Idx].Next)
				{
					OutCloseEntities.Add(Items[Idx].ID);
					if (OutCloseEntities.Num() >= MaxResults)
					{
						return;
					}
				}
			}
		}
	}
};
