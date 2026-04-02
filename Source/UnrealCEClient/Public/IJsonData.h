#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IJsonData.generated.h"

UINTERFACE(MinimalAPI)
class UJsonData : public UInterface
{
    GENERATED_BODY()
};

class MYPROJECT_API IJsonData
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void ProcessVariable(const FString& name, const FString& value);  // TODO: figure out if we can make this a little smarter (maybe implement specific float/string/int/bool variants?
};