#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <map>
#include <string>

#include "VirtualRoot.generated.h"

UCLASS()
class UNREALCECLIENT_API AVirtualRoot : public AActor
{
	GENERATED_BODY()

public:
	AVirtualRoot();

	static std::map<std::string, FVector> proxyPositions;
	static std::map<std::string, FQuat> proxyRotations;

	static FVector TransformPosition(std::string home, FVector position);
	static FQuat TransformRotation(std::string home, FQuat rotation);

	static void UpdateProxy(std::string home, FVector position, FQuat rotation );

private:
	static TObjectPtr<AVirtualRoot> instance;
};