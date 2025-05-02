#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MSRepresentationFragments.generated.h"

class ANiagaraEntityVizActor;

/**	Please keep in mind that we key NiagaraSystemFragments off of the pointer
*	to the niagara system selected in the trait.
*	Don't use the regular struct CRC32 hash like you would for other shared fragments.
**/
USTRUCT()
struct NIAGARAVISUALIZATION_API FSharedNiagaraSystemFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<ANiagaraEntityVizActor> NiagaraManagerActor;

	//This is used to make sure we insert to the right space in the niagara array after iterating a chunk and so on
	int32 IteratorOffset = 0;


	inline static FName ParticlePositionsName = "MassParticlePositions";
	
	UPROPERTY()
	TArray<FVector> ParticlePositions;
	
	inline static FName ParticleOrientationsParameterName = "MassParticleOrientations";

	UPROPERTY()
	TArray<FQuat4f> ParticleOrientations;
	
	inline static FName AnimationIndexesParameterName = "MassAnimationIndexes";
	
	UPROPERTY()
	TArray<uint8> AnimationIndexes;
};



