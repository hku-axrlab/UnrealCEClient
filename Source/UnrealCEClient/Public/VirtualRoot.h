#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VirtualRoot.generated.h"

UCLASS()
class UNREALCECLIENT_API AVirtualRoot : public AActor
{
	GENERATED_BODY()

public:
	AVirtualRoot();

	static FVector proxyPosition;
	static FQuat proxyRotation;

	static FVector TransformPosition(FVector position);

	static FQuat TransformRotation(FQuat rotation);

private:
	static TObjectPtr<AVirtualRoot> instance;
};