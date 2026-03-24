#include "Data/FTFragments.h"

#include "MassEntityTemplateRegistry.h"

void UFTMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FFTTransformFragment>();
	BuildContext.AddFragment<FFTDesiredMovementFragment>();
	FFTVelocityFragment& Velocity = BuildContext.AddFragment_GetRef<FFTVelocityFragment>();
	
	Velocity.Value = FVector(50.f, 0, 0);
}
