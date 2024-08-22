// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VertexAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class VERTEXANIMCHARACTER_API UVertexAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Velocity;

	void SetVelocity(const FVector& NewVelocity);
};
