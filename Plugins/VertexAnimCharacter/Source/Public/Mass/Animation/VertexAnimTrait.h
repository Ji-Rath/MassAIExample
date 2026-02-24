// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityElementTypes.h"
#include "MassEntityTraitBase.h"
#include "MassExternalSubsystemTraits.h"
#include "VertexAnimTrait.generated.h"

class UAnimToTextureDataAsset;

// Holds simple animation data
USTRUCT()
struct VERTEXANIMCHARACTER_API FVertexAnimInfoFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FVertexAnimInfoFragment() = default;
	
	float GlobalStartTime = 0.0f;
	float PlayRate = 1.0f;
	int8 AnimationStateIndex = 0;
	bool bSwappedThisFrame = false;
	bool bCustomAnimation = false;
};

USTRUCT()
struct VERTEXANIMCHARACTER_API FVertexAnimSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;
};

template<>
struct TMassFragmentTraits<FVertexAnimSharedFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
};

// Animation data for playing montages
USTRUCT()
struct FMassMontageFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UAnimMontage> Montage;
	
	float Position = 0.f;

	FMassMontageFragment() = default;

	FMassMontageFragment(const TSoftObjectPtr<UAnimMontage>& InMontage): Montage(InMontage) {};
};

template<>
struct TMassFragmentTraits<FMassMontageFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
};

USTRUCT()
struct FVertexAnimLocomotionFragment : public FMassFragment
{
	GENERATED_BODY()
	
	// Speed threshhold for swapping between idle/run
	UPROPERTY(EditAnywhere)
	int16 SpeedThreshhold = 50;
	
	UPROPERTY(EditAnywhere)
	int8 IdleAnimIndex = 0;

	UPROPERTY(EditAnywhere)
	int8 RunAnimIndex = 0;
};

/**
 * @class UVertexAnimTrait
 * @brief A mass entity trait that supports vertex animation functionality in a character system.
 *
 * This class integrates vertex animation details into entities by utilizing the FMassEntityTraitBase
 * framework. It allows customization and configuration for vertex animations using properties
 * defined in the associated fragment, FVertexAnimInfoFragment.
 */
UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	
	UPROPERTY(EditAnywhere, Category = "Vertex Animation")
	FVertexAnimSharedFragment VertexAnimData;
	
	UPROPERTY(EditAnywhere, Category = "Vertex Animation")
	FVertexAnimLocomotionFragment VertexAnimLocomotion;
};
