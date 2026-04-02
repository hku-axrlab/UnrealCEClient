#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IJsonData.h"
#include "MemberVariable.h"

#include "BaseTemplate.generated.h"

UCLASS()
class UNREALCECLIENT_API ABaseTemplate : public AActor, public IJsonData
{
	GENERATED_BODY()

public:
	ABaseTemplate();

	UPROPERTY(VisibleAnywhere, Category = "External Variables")
	bool isLive = true;

	UPROPERTY(VisibleAnywhere, Category = "External Variables")
	bool isActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variable Map")
	TArray<FString> variableMap = TArray<FString>{ "live", "visible" };

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    FString ToString( const FMemberVariable& variable ) const
    {
        return variable.value.get<std::string>().c_str();
    }

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    float ToFloat(const FMemberVariable& variable) const
    {
        return variable.value.get<float>();
    }

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    int ToInt(const FMemberVariable& variable) const
    {
        return variable.value.get<int>();
    }

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    bool ToBool(const FMemberVariable& variable) const
    {
        return variable.value.get<bool>();
    }

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    FLinearColor ToColor(const FMemberVariable& variable) const
    {
        FLinearColor c;
        c.R = variable.value["r"].get<float>();
        c.G = variable.value["g"].get<float>();
        c.B = variable.value["b"].get<float>();
        c.A = variable.value["a"].get<float>();
        return c;
    }

    UFUNCTION(BlueprintCallable, Category = "DataFunctions")
    FVector ToVector(const FMemberVariable& variable) const
    {
        FVector v;
        v.X = variable.value["x"].get<float>();
        v.Y = variable.value["y"].get<float>();
        v.Z = variable.value["z"].get<float>();
        return v;
    }

    virtual void ProcessVariable(const FMemberVariable& variable);
};