#include "BaseTemplate.h"

ABaseTemplate::ABaseTemplate() 
{
	PrimaryActorTick.bCanEverTick = true;

	variableMap.Add("live");
	variableMap.Add("visible");
}

void ABaseTemplate::ProcessVariable(const FMemberVariable& variable)
{
	if (variableMap.Contains(variable.name)) {
		if (variable.name == "live") {
			isLive = ToBool(variable);
		}
		else if (variable.name == "visible") {
			SetActorHiddenInGame(!ToBool(variable));
		}
	}
	else if ( isLive ) // forward to Blueprints
		ABaseTemplate::Execute_ProcessVariable(this, variable);
}