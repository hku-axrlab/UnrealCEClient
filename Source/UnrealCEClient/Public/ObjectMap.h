#pragma once

#include "CoreMinimal.h"
#include "ObjectMap.generated.h"

UCLASS(BlueprintType)
class UNREALCECLIENT_API UObjectMap : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Map")
    TMap<FString, TSubclassOf<AActor>> ClassMap;
};