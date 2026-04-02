#include "VirtualRoot.h"
#include "Engine/Engine.h"

TObjectPtr<AVirtualRoot> AVirtualRoot::instance = nullptr;
FVector AVirtualRoot::proxyPosition;
FQuat AVirtualRoot::proxyRotation;

// Sets default values
AVirtualRoot::AVirtualRoot()
{
	PrimaryActorTick.bCanEverTick = false;
	instance = this;
}

FVector AVirtualRoot::TransformPosition(FVector position)
{
	if (instance == nullptr) return position;

	// todo implement
	return position;
}

FQuat AVirtualRoot::TransformRotation(FQuat rotation)
{
	if (instance == nullptr) return rotation;

	// todo implement
	return rotation;
}