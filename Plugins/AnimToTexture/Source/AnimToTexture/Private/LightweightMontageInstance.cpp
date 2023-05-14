// Copyright Epic Games, Inc. All Rights Reserved.

#include "LightweightMontageInstance.h"
#include "Animation/AnimMontage.h"

EMontageSubStepResult UE::VertexAnimation::FLightweightMontageSubstepper::Advance(float& InOut_P_Original, UAnimMontage* Montage)
{
	// Copied from MontageSubstepper::Advance
	// @todo : Clean up + optimize when we do profiling

	DeltaMove = 0.f;

	if (Montage == nullptr)
	{
		return EMontageSubStepResult::InvalidMontage;
	}

	bReachedEndOfSection = false;

	float PositionInSection;
	CurrentSectionIndex = Montage->GetAnimCompositeSectionIndexFromPos(InOut_P_Original, PositionInSection);
	if (!Montage->IsValidSectionIndex(CurrentSectionIndex))
	{
		return EMontageSubStepResult::InvalidSection;
	}

	const FCompositeSection& CurrentSection = Montage->GetAnimCompositeSection(CurrentSectionIndex);
	CurrentSectionStartTime = CurrentSection.GetTime();

	// Find end of current section. We only update one section at a time.
	CurrentSectionLength = Montage->GetSectionLength(CurrentSectionIndex);

	if (FMath::IsNearlyZero(TimeRemaining))
	{
		return EMontageSubStepResult::NotMoved;
	}

	const float PlayRate = Montage->RateScale;

	if (FMath::IsNearlyZero(PlayRate))
	{
		return EMontageSubStepResult::NotMoved;
	}

	DeltaMove = TimeRemaining * PlayRate;

	// Finally clamp DeltaMove by section markers.
	{
		const float OldDeltaMove = DeltaMove;

		// Clamp DeltaMove based on move allowed within current section
		// We stop at each section marker to evaluate whether we should jump to another section marker or not.
		// Test is inclusive, so we know if we've reached marker or not.
		const float MaxSectionMove = CurrentSectionLength - PositionInSection;
		if (DeltaMove >= MaxSectionMove)
		{
			DeltaMove = MaxSectionMove;
			bReachedEndOfSection = true;
		}
	}

	// DeltaMove is now final, see if it has any effect on our position.
	if (FMath::Abs(DeltaMove) > 0.f)
	{
		// Note that we don't worry about looping and wrapping around here.
		// We step per section to simplify code to extract notifies/root motion/etc.
		InOut_P_Original += DeltaMove;

		// Decrease RemainingTime with actual time elapsed 
		// So we can take more substeps as needed.
		const float TimeStep = DeltaMove / PlayRate;
		ensure(TimeStep >= 0.f);
		TimeRemaining = FMath::Max(TimeRemaining - TimeStep, 0.f);

		return EMontageSubStepResult::Moved;
	}
	else
	{
		return EMontageSubStepResult::NotMoved;
	}
}

void UE::VertexAnimation::FLightweightMontageInstance::Initialize(UAnimMontage* InMontage, float StartTime)
{
	Terminate();

	MontageWeak = InMontage;
	Montage = InMontage;
	if (Montage)
	{
		CachedLength = InMontage->GetPlayLength();
		bInitialized = true;
		Position = StartTime;
		RefreshNextPrevSections(Montage);
		MontageSubStepper.Initialize(this);
		MontageSubStepper.Advance(Position, Montage);
		SetSequence(FindSequenceAtCurrentTime(SectionStartTimeTransient));
	}

	Montage = nullptr;
}

void UE::VertexAnimation::FLightweightMontageInstance::RefreshNextPrevSections(UAnimMontage* InMontage)
{
	// initialize next section
	if (InMontage->CompositeSections.Num() > 0)
	{
		NextSections.Empty(InMontage->CompositeSections.Num());
		NextSections.AddUninitialized(InMontage->CompositeSections.Num());

		for (int32 I = 0; I < InMontage->CompositeSections.Num(); ++I)
		{
			FCompositeSection& Section = InMontage->CompositeSections[I];
			int32 NextSectionIdx = InMontage->GetSectionIndex(Section.NextSectionName);
			NextSections[I] = NextSectionIdx;
		}
	}
}

const UAnimSequence* UE::VertexAnimation::FLightweightMontageInstance::FindSequenceAtCurrentTime(float& SequenceStartTime)
{
	SequenceStartTime = 0.0f;
	const FAnimTrack& AnimTrack = Montage->SlotAnimTracks[0].AnimTrack;
	if (const FAnimSegment* Segment = AnimTrack.GetSegmentAtTime(Position))
	{
		SequenceStartTime = Segment->StartPos - Segment->AnimStartTime;
		return Cast<UAnimSequence>(Segment->AnimReference);
	}

	return nullptr;
}

UAnimMontage* UE::VertexAnimation::FLightweightMontageInstance::GetMontage() const
{
	return bInitialized ? MontageWeak.Get() : nullptr;
}

const UAnimSequence* UE::VertexAnimation::FLightweightMontageInstance::GetSequence() const
{
	return bInitialized ? SequenceWeak.Get() : nullptr;
}

void UE::VertexAnimation::FLightweightMontageInstance::SetSequence(const UAnimSequence* InSequence)
{
	bSequenceChangedThisFrame = false;
	const UAnimSequence* PrevSequence = SequenceWeak.Get();
	if (PrevSequence != InSequence)
	{
		bSequenceChangedThisFrame = true;
		SequenceWeak = InSequence;
	}
}

float UE::VertexAnimation::FLightweightMontageInstance::GetPositionInSection() const
{
	return Position - SectionStartTimeTransient;
}

void UE::VertexAnimation::FLightweightMontageInstance::Terminate()
{
	*this = FLightweightMontageInstance();
}

bool UE::VertexAnimation::FLightweightMontageInstance::Advance_Internal(float DeltaTime, FRootMotionMovementParams& OutRootMotionParams, const FLightWeightMontageExtractionSettings& ExtractionSettings)
{
	// Copied from FAnimMontageInstance::Advance
	// @todo : Clean up + optimize when we do profiling

	const bool bExtractRootMotion = Montage->HasRootMotion() && ExtractionSettings.bExtractRootMotion;
	
	/**
		Limit number of iterations for performance.
		This can get out of control if PlayRate is set really high, or there is a hitch, and Montage is looping for example.
	*/
	const int32 MaxIterations = 10;
	int32 NumIterations = 0;

	/**
		If we're hitting our max number of iterations for whatever reason,
		make sure we're not accumulating too much time, and go out of range.
	*/
	if (MontageSubStepper.GetRemainingTime() < 10.f)
	{
		MontageSubStepper.AddEvaluationTime(DeltaTime);
	}

	while (MontageSubStepper.HasTimeRemaining() && (++NumIterations < MaxIterations))
	{
		const float PreviousSubStepPosition = Position;
		EMontageSubStepResult SubStepResult = MontageSubStepper.Advance(Position, Montage);

		if (SubStepResult != EMontageSubStepResult::Moved)
		{
			// stop and leave this loop
			break;
		}

		// Extract Root Motion for this time slice, and accumulate it.
		if (bExtractRootMotion)
		{
			OutRootMotionParams.Accumulate(Montage->ExtractRootMotionFromTrackRange(PreviousSubStepPosition, Position));
		}

		// if we reached end of section, and we were not processing a branching point, and no events has messed with out current position..
		// .. Move to next section.
		// (this also handles looping, the same as jumping to a different section).
		if (MontageSubStepper.HasReachedEndOfSection())
		{
			const int32 CurrentSectionIndex = MontageSubStepper.GetCurrentSectionIndex();

			// Get recent NextSectionIndex in case it's been changed by previous events.
			const int32 RecentNextSectionIndex = NextSections[CurrentSectionIndex];
			if (RecentNextSectionIndex != INDEX_NONE)
			{
				float LatestNextSectionStartTime;
				float LatestNextSectionEndTime;
				Montage->GetSectionStartAndEndTime(RecentNextSectionIndex, LatestNextSectionStartTime, LatestNextSectionEndTime);

				// Jump to next section's appropriate starting point (start or end).
				Position = LatestNextSectionStartTime;
			}
			else
			{
				// Reached end of last section. Exit.
				return false;
				break;
			}
		}
	}

	return true;
}

bool UE::VertexAnimation::FLightweightMontageInstance::IsValid() const
{
	return bInitialized && MontageWeak.IsValid() && SequenceWeak.IsValid();
}

void UE::VertexAnimation::FLightweightMontageInstance::Advance(float DeltaTime, float InGlobalTime, FRootMotionMovementParams& OutRootMotionParams, const FLightWeightMontageExtractionSettings& ExtractionSettings)
{
	Montage = GetMontage();
	if (Montage == nullptr)
	{
		return;
	}

	const bool bIsPlaying = Advance_Internal(DeltaTime, OutRootMotionParams, ExtractionSettings);
	const UAnimSequence* CurrentSequence = FindSequenceAtCurrentTime(SectionStartTimeTransient);

	SetSequence(CurrentSequence);

	Montage = nullptr;

	if (!bIsPlaying)
	{
		Terminate();
	}
}