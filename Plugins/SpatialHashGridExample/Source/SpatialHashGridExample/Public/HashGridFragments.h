// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HashGridSubsystem.h"
#include "HashGridFragments.generated.h"

namespace HashGridExample::Signals
{
	const FName EntityQueried = FName(TEXT("EntityQueried"));
}

/**
 * 
 */
USTRUCT()
struct SPATIALHASHGRIDEXAMPLE_API FHashGridFragment : public FMassFragment
{
	GENERATED_BODY()
public:
	FHashGridExample::FCellLocation CellLocation;
};
