// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/WeakObjectPtrTemplates.h"

class UAnimMontage;
class UAnimSequence;
struct FLightweightMontageInstance;

enum class EMontageSubStepResult : uint8;

namespace UE
{
	namespace VertexAnimation
	{
		struct FLightWeightMontageExtractionSettings
		{
			bool bExtractRootMotion = false;
		};

		struct FLightweightMontageInstance;

		struct FLightweightMontageSubstepper
		{
			void AddEvaluationTime(float InDeltaTime) { TimeRemaining += InDeltaTime; }
			bool HasTimeRemaining() const { return (TimeRemaining > SMALL_NUMBER); }
			bool HasReachedEndOfSection() const { return bReachedEndOfSection; }
			float GetRemainingTime() const { return TimeRemaining; }
			EMontageSubStepResult Advance(float& InOutTime, UAnimMontage* Montage);
			int32 GetCurrentSectionIndex() const { return CurrentSectionIndex; }
			float GetCurrentSectionLength() const { return CurrentSectionLength; }
			float GetCurrentSectionStartTime() const { return CurrentSectionStartTime; }
			float GetCurrentDeltaMove() const { return DeltaMove; }

			void Initialize(UE::VertexAnimation::FLightweightMontageInstance* InMontageInstance)
			{
			}

			float TimeRemaining = 0.0f;
			float DeltaMove = 0.0f;
			float CurrentSectionLength = 0.0f;
			float CurrentSectionStartTime = 0.0f;
			int32 CurrentSectionIndex = 0;
			bool bReachedEndOfSection = false;
		};

		struct ANIMTOTEXTURE_API FLightweightMontageInstance
		{
			bool SequenceChangedThisFrame() const { return bSequenceChangedThisFrame; }
			bool WasInitialized() const { return bInitialized; }
			bool IsValid() const;
			void Advance(float DeltaTime, float InGlobalTime, struct FRootMotionMovementParams& OutRootMotionParams, const FLightWeightMontageExtractionSettings& ExtractionSettings);
			void Initialize(UAnimMontage* InMontage, float StartTime = 0.0f);
			UAnimMontage* GetMontage() const;
			const UAnimSequence* GetSequence() const;
			float GetPositionInSection() const;
			float GetPosition() const { return Position; }
			float GetLength() const { return CachedLength; }
			void Terminate();

		private:
			bool Advance_Internal(float DeltaTime, struct FRootMotionMovementParams& OutRootMotionParams, const FLightWeightMontageExtractionSettings& ExtractionSettings);
			void RefreshNextPrevSections(UAnimMontage* InMontage);
			void SetSequence(const UAnimSequence* InSequence);
			const UAnimSequence* FindSequenceAtCurrentTime(float& SequenceStartTime);

		public:
			
			FRootMotionMovementParams RootMotionDelta;

		private:

			UE::VertexAnimation::FLightweightMontageSubstepper MontageSubStepper;

			// @todo : Handle asset references externally to avoid paying the cost of the weak ptr
			TWeakObjectPtr<UAnimMontage> MontageWeak = nullptr;
			TWeakObjectPtr<const UAnimSequence> SequenceWeak = nullptr;

			// Transient. Gets cleared at the end of update
			UAnimMontage* Montage = nullptr;

			TArray<int32, TInlineAllocator<4>> NextSections;
			float Position = 0.0f;
			float CachedLength = 0.0f;

			// We need this since GetSegmentIndexAtTime and GetAnimCompositeSectionIndexFromPos can return different sections at the same frame
			float SectionStartTimeTransient = 0.0f;

			bool bSequenceChangedThisFrame = false;
			bool bInitialized = false;
		};
	}
}