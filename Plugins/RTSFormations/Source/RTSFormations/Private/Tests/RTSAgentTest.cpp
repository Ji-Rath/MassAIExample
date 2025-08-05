#include "MassEntityConfigAsset.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"
#include "Tests/AutomationCommon.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRTSSpawnUnitsTest, "RTS.SpawnUnits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FRTSSpawnUnitsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper WorldWrapper;
	WorldWrapper.CreateTestWorld(EWorldType::Game);
	UWorld* World = WorldWrapper.GetTestWorld();

	if (!World)
	{
		return false;
	}

	WorldWrapper.BeginPlayInTestWorld();

	auto FormationSubsystem = World->GetSubsystem<URTSFormationSubsystem>();

	FMassEntityConfig EntityConfig;
	auto TraitInstance = NewObject<URTSFormationAgentTrait>(FormationSubsystem, FName(), RF_Transactional);
	check(TraitInstance);
	EntityConfig.AddTrait(*TraitInstance);
	
	auto UnitHandle = FormationSubsystem->SpawnUnit(EntityConfig, 10000, FVector::Zero());
	WorldWrapper.TickTestWorld(); // Finish defer spawn entities

	int Iterations = 100;
	float AvgTime = 0.0f;
	for (int i=0;i<Iterations;i++)
	{
		FormationSubsystem->UpdateUnitPosition(UnitHandle);
		AvgTime += RTS::Stats::UpdateEntityIndexTimeSec;
	}
	AvgTime/=Iterations;
	
	ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(FString::Printf(TEXT("Average Update Entity Index: %fms"), AvgTime*1000)));
	ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(FString::Printf(TEXT("Update Unit Position: %fms"), RTS::Stats::UpdateUnitPositionTimeSec*1000)));
	
	WorldWrapper.ForwardErrorMessages(this);
	return !HasAnyErrors();
};

#endif