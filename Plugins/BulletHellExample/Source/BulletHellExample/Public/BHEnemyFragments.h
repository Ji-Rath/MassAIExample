// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletHellSubsystem.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "BHEnemyFragments.generated.h"

// Basic fragment about a bullet hell enemy
USTRUCT()
struct BULLETHELLEXAMPLE_API FBHEnemyFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Health;

	UPROPERTY(EditAnywhere)
	FVector CollisionExtent = FVector(100.f);

	FBHEntityHashGrid::FCellLocation CellLocation;
};

// Defines an entity as a bullet hell enemy
USTRUCT()
struct BULLETHELLEXAMPLE_API FBHEnemyTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS(meta=(DisplayName="Bullet Hell Enemy"))
class BULLETHELLEXAMPLE_API UBHEnemyTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(Category="Bullet Hell", EditAnywhere)
	FBHEnemyFragment BHEnemyFragment;
};