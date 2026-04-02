// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WebSocketsModule.h" // Module definition
#include "IWebSocket.h"       // Socket definition
#include "ObjectMap.h"

#include <string>
#include <vector>

#include "Structs.h"
// Third party located directly in Source because sub-includes do not parse correctly otherwise  
#include "nlohmann/json.hpp"  

#include "Containers/UnrealString.h"
#include "CalibEnvLinker.generated.h"

UCLASS()
class UNREALCECLIENT_API ACalibEnvLinker : public AActor
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // Called when the game ends
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Sets default values for this actor's properties
    ACalibEnvLinker();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerInfo")
    FString serverIP = "localhost";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerInfo")
    int serverPort = 4196;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectMap")
    TObjectPtr<UObjectMap> objectMap;

    UPROPERTY(VisibleAnywhere, Category = "ObjectMap")
    TMap<FString, TObjectPtr<AActor>> spawnedObjects;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // ── Blueprint callable API ─────────────────────────────────────────────

    // Open the connection to ws://Host:Port
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Connect(const FString& Host, int32 Port);

    // Gracefully close the socket
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Disconnect();

    // Send an arbitrary JSON string (convenience for Blueprint)
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendRawJson(const FString& JsonString);

    UFUNCTION(BlueprintPure, Category = "WebSocket")
    bool IsConnected() const;

    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    TMap<FString, FString> GetJsonMap() const;
    
private:
    TSharedPtr<IWebSocket> Socket;

    TMap<FString, FString> outMap;

    void BindSocketEvents();
    void OnConnectedHandler();
    void OnMessageHandler(const FString& Message);
    void OnConnectionErrorHandler(const FString& Error);
    void OnClosedHandler(int32 StatusCode, const FString& Reason, bool bWasClean);

    SlotData ParseSlot(const nlohmann::json& slotResponse);
    std::vector<SlotData> ParseBatchResponse(const std::string& jsonString);
    void HandleSlotSpawning(const std::vector<SlotData>& slots);
};