#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IJsonData.generated.h"

UINTERFACE(MinimalAPI)
class UJsonData : public UInterface
{
    GENERATED_BODY()
};

class UNREALCECLIENT_API IJsonData
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ProcessVariable(const FMemberVariable& variable);
};