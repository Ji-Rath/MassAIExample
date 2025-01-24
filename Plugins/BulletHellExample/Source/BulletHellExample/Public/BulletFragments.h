// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "BulletFragments.generated.h"


USTRUCT()
struct FBulletFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector SpawnLocation;
	
	FVector Direction;

	UPROPERTY(EditAnywhere)
	float Speed = 500.f;

	UPROPERTY(EditAnywhere)
	float Lifetime = 5.f;
};

USTRUCT()
struct FBulletTag : public FMassTag
{
	GENERATED_BODY()
	
};

UCLASS()
class UBulletTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	FBulletFragment BulletFragment;
};
