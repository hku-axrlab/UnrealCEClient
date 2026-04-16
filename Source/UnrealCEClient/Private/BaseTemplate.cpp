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
	else if ( isLive && IsValid(this)) // forward to Blueprints
	{
        FScopedScriptExceptionHandler ExceptionHandler([](ELogVerbosity::Type Verbosity, const TCHAR* Message, const TCHAR* StackTrace)
        {
            UE_LOG(LogTemp, Error, TEXT("Blueprint ProcessVariable exception: %s"), Message);
        });

        ABaseTemplate::Execute_ProcessVariable(this, variable);
	}
}