#include "MassEntityConfigAsset.h"
#include "RTSAgentTraits.h"
#include "RTSFormationSubsystem.h"
#include "Tests/AutomationCommon.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRTSSpawnUnitsTest, "RTS.SpawnUnits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FRTSSpawnUnitsTest::RunTest(const FString& Parameters)
{
	FRandomStream RandomStream { 12345 };
	
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

	auto EntityCount = 10000;
	auto UnitHandle = FormationSubsystem->SpawnUnit(EntityConfig, EntityCount, FVector::Zero());
	WorldWrapper.TickTestWorld(); // Finish defer spawn entities

	int Iterations = 100;
	float AvgTime = 0.0f;
	for (int i=0;i<Iterations;i++)
	{
		FormationSubsystem->SetUnitPosition(FVector(RandomStream.FRand() * 5000.f, RandomStream.FRand() * 5000.f, 0.f),UnitHandle);
		AvgTime += RTS::Stats::UpdateEntityIndexTimeSec;
	}
	AvgTime/=Iterations;

	ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(FString::Printf(TEXT("Test Info: %d Entities | %d Iterations | %d Avg Iterations In Loop"), EntityCount, Iterations, RTS::Stats::IterationsInLoop++ / Iterations)));
	ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(FString::Printf(TEXT("Avg Update Entity Index: %fms"), AvgTime*1000)));
	ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(FString::Printf(TEXT("Update Unit Position: %fms"), RTS::Stats::UpdateUnitPositionTimeSec*1000)));
	
	WorldWrapper.ForwardErrorMessages(this);
	return !HasAnyErrors();
};

#endif