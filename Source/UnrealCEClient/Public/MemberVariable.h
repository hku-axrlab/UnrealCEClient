#pragma once

#include "nlohmann/json.hpp"
#include "MemberVariable.generated.h"

USTRUCT(BlueprintType)
struct FMemberVariable
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    FString name;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metadata")
    FString type;

    nlohmann::json value;

    // Constructor
    FMemberVariable()
        : name("defaultName"), type("float")
    {
    }
};