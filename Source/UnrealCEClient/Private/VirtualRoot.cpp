#include "VirtualRoot.h"
#include "Engine/Engine.h"

TObjectPtr<AVirtualRoot> AVirtualRoot::instance = nullptr;
std::map<std::string, FVector> AVirtualRoot::proxyPositions;
std::map<std::string, FQuat> AVirtualRoot::proxyRotations;

// Sets default values
AVirtualRoot::AVirtualRoot()
{
	PrimaryActorTick.bCanEverTick = false;
	instance = this;
}

FVector AVirtualRoot::TransformPosition(std::string home, FVector position)
{
	if (instance == nullptr ) return position;
	else if ( !proxyPositions.contains(home) )
		return instance->GetActorLocation() + 
           instance->GetActorQuat() * position;

	FQuat inverseProxy = proxyRotations[home].Inverse();
    return instance->GetActorLocation() + 
           instance->GetActorQuat() * inverseProxy * (position - proxyPositions[home]);
}

FQuat AVirtualRoot::TransformRotation(std::string home, FQuat rotation)
{
	if (instance == nullptr) return rotation;
	else if (!proxyRotations.contains(home))
		instance->GetActorQuat() * rotation;

	return instance->GetActorQuat() * proxyRotations[home].Inverse() * rotation;
}

void AVirtualRoot::UpdateProxy(std::string home, FVector position, FQuat rotation)
{
	if (instance == nullptr) return;

	if (proxyPositions.contains(home)) {
		proxyPositions[home] = position;
		proxyRotations[home] = rotation;
	}
	else {
		proxyPositions.emplace(home, position);
		proxyRotations.emplace(home, rotation);
	}
}