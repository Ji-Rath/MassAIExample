// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleRandomMovementTrait.h"

#include "MassEntityTemplateRegistry.h"

void USimpleRandomMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddFragment<FSimpleMovementFragment>();
}
