#include "Data/FTFragments.h"

#include "MassEntityTemplateRegistry.h"

void UFTMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FFTTransformFragment>();
	BuildContext.AddFragment<FFTDesiredMovementFragment>();
	BuildContext.AddFragment<FFTVelocityFragment>();
	BuildContext.AddFragment<FFTMoveTargetFragment>();
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct& MovementSettingsShared = EntityManager.GetOrCreateConstSharedFragment(MovementSettings);
	
	BuildContext.AddConstSharedFragment(MovementSettingsShared);
}
