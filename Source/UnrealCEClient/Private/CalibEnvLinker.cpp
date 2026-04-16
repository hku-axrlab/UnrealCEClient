// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibEnvLinker.h"
#include "WebSocketsModule.h"

#include "VirtualRoot.h"
#include <stdio.h>
#include "IJsonData.h"
#include "BaseTemplate.h"

using json = nlohmann::json;

ObjectData ACalibEnvLinker::ParseObject(const json& objectJson)
{
    ObjectData slot;

    // ID comes from the slot-level id field
    slot.id = objectJson["id"].get<std::string>();

    // Name and tag are nested under data, each with a "value" field
    slot.name = objectJson["name"].get<std::string>();
    slot.tag = objectJson["tag"].is_null() ? "" : objectJson["tag"].get<std::string>();
    slot.home = objectJson["home"].get<std::string>();
    
    const auto& transform = objectJson["transform"];

    // Position
    const auto& pos = transform["position"];
    slot.transform.x = pos["X"].get<float>();
    slot.transform.y = pos["Y"].get<float>();
    slot.transform.z = pos["Z"].get<float>();

    // Rotation (quaternion)
    const auto& rot = transform["rotation"];
    slot.transform.qx = rot["X"].get<float>();
    slot.transform.qy = rot["Y"].get<float>();
    slot.transform.qz = rot["Z"].get<float>();
    slot.transform.qw = rot["W"].get<float>();

    // Scale
    const auto& scl = transform["scale"];
    slot.transform.sx = scl["X"].get<float>();
    slot.transform.sy = scl["Y"].get<float>();
    slot.transform.sz = scl["Z"].get<float>();

    for (const auto& variable : objectJson["data"]) {
        std::string type = variable["type"].get<std::string>();
        FMemberVariable v;
        v.name = variable["name"].get<std::string>().c_str();
        v.type = variable["type"].get<std::string>().c_str();
        v.value = variable["value"];
        slot.variables.push_back(v);
    }

    return slot;
}

std::vector<ObjectData> ACalibEnvLinker::ParseBatchResponse(const std::string& jsonString)
{
    std::vector<ObjectData> objects;

    json root = json::parse(jsonString);
    if (root["objects"].is_null()) return objects;

    for (const auto& objJson : root["objects"])
    {
       objects.push_back(ParseObject(objJson));
    }

    return objects;
}

void ACalibEnvLinker::HandleObjectSpawning(const std::vector<ObjectData>& slots)
{
    for (auto slot : slots)
    {
        if (slot.tag == "vRoot")
        {
            AVirtualRoot::UpdateProxy(slot.home, 
                FVector(-slot.transform.x * 100.0f, slot.transform.z * 100.0f, slot.transform.y * 100.0f), 
                FQuat(-slot.transform.qx, slot.transform.qz, slot.transform.qy, slot.transform.qw));
        }
        else
        {
            const char * id = slot.id.c_str();
            const char * tag = slot.tag.c_str();
            // Check if the ID is present in the Spawned Object Vector
            if (!spawnedObjects.Contains(id)) {
                // Try to spawn the actor
                if (!objectMap->ClassMap.Contains(tag)) {
                    continue;   // not found, skip this one
                }
                
                // Spawn the actor & insert into map
                AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(objectMap->ClassMap[tag]);
                spawnedObjects.Add(id, SpawnedActor);
            }

            TObjectPtr<AActor> actor = spawnedObjects[id];

            FVector position(-slot.transform.x * 100.0f, slot.transform.z * 100.0f, slot.transform.y * 100.0f);
            FQuat rotation(-slot.transform.qx, slot.transform.qz, slot.transform.qy, slot.transform.qw);
            FVector scale(slot.transform.sx, slot.transform.sz, slot.transform.sy);
            
            if (actor) {
                ABaseTemplate* baseActor = nullptr;
                if (actor->IsA(ABaseTemplate::StaticClass())) {
                    baseActor = Cast<ABaseTemplate>(actor);

                    // Send variables for processing
                    for (const FMemberVariable& variable : slot.variables) {
                        // TODO: test this
                        baseActor->ProcessVariable(variable);
                    }
                }

                if (baseActor == nullptr || baseActor->isLive) {
                    actor->SetActorLocationAndRotation(AVirtualRoot::TransformPosition(slot.home, position), AVirtualRoot::TransformRotation(slot.home, rotation));
                    actor->SetActorScale3D(scale);
                }
            }
        }
    }
}

// Sets default values
ACalibEnvLinker::ACalibEnvLinker()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACalibEnvLinker::BeginPlay()
{
    Super::BeginPlay();

    // Auto-connect on start
    Connect(serverIP, serverPort);
}

// Called when the game starts or when spawned
void ACalibEnvLinker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Auto-connect on start
    Disconnect();
}

// Called every frame
void ACalibEnvLinker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ── Connect ────────────────────────────────────────────────────────────────

void ACalibEnvLinker::Connect(const FString& Host, int32 Port)
{
    if (Socket && Socket->IsConnected())
    {
        UE_LOG(LogTemp, Warning, TEXT("WebSocketClient: already connected."));
        return;
    }

    const FString Url = FString::Printf(TEXT("ws://%s:%d"), *Host, Port);
    UE_LOG(LogTemp, Log, TEXT("WebSocketClient: connecting to %s"), *Url);

    Socket = FWebSocketsModule::Get().CreateWebSocket(Url, TEXT("ws"));
    BindSocketEvents();
    Socket->Connect();
}

// ── Disconnect ─────────────────────────────────────────────────────────────

void ACalibEnvLinker::Disconnect()
{
    if (Socket)
    {
        Socket->Close();
    }
}

// ── Send ───────────────────────────────────────────────────────────────────

void ACalibEnvLinker::SendRawJson(const FString& JsonString)
{
    if (Socket && Socket->IsConnected())
    {
        Socket->Send(JsonString);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WebSocketClient: cannot send — not connected."));
    }
}

bool ACalibEnvLinker::IsConnected() const
{
    return Socket && Socket->IsConnected();
}

// ── Event binding ──────────────────────────────────────────────────────────

void ACalibEnvLinker::BindSocketEvents()
{
    Socket->OnConnected().AddUObject(this, &ACalibEnvLinker::OnConnectedHandler);
    Socket->OnMessage().AddUObject(this, &ACalibEnvLinker::OnMessageHandler);
    Socket->OnConnectionError().AddUObject(this, &ACalibEnvLinker::OnConnectionErrorHandler);
    Socket->OnClosed().AddUObject(this, &ACalibEnvLinker::OnClosedHandler);
}

// ── Handlers ───────────────────────────────────────────────────────────────

void ACalibEnvLinker::OnConnectedHandler()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocketClient: connected. Sending handshake."));

    // Send the client-type identification message immediately on connect
    const json handshake = { 
        {"clientType", "unreal"}, 
        {"msgType", 0}, 
        {"updateRate", 30 }
    };
    const FString HandshakeStr = UTF8_TO_TCHAR(handshake.dump().c_str());
    Socket->Send(HandshakeStr);
}

void ACalibEnvLinker::OnMessageHandler(const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocketClient: message received: %s"), *Message);

    TMap<FString, FString> OutMap;

    try
    {
        std::vector<ObjectData> slotsFound = ParseBatchResponse(TCHAR_TO_UTF8(*Message));
        HandleObjectSpawning(slotsFound);
    }
    catch (const json::parse_error& e)
    {
        const FString ErrMsg = FString::Printf(
            TEXT("JSON parse error: %s"), UTF8_TO_TCHAR(e.what()));
        UE_LOG(LogTemp, Error, TEXT("WebSocketClient: %s"), *ErrMsg);
        return;
    }
    catch (const std::exception& e)
    {
        const FString ErrMsg = FString::Printf(
            TEXT("JSON parse error: %s"), UTF8_TO_TCHAR(e.what()));
        UE_LOG(LogTemp, Error, TEXT("WebSocketClient: %s"), *ErrMsg);
        return;
    }

    outMap = OutMap;
}

void ACalibEnvLinker::OnConnectionErrorHandler(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocketClient: connection error: %s"), *Error);
    // OnError.Broadcast(Error);
}

void ACalibEnvLinker::OnClosedHandler(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocketClient: closed (code %d, reason: %s, clean: %s)"),
        StatusCode, *Reason, bWasClean ? TEXT("yes") : TEXT("no"));
    // OnClosed.Broadcast();
}

TMap<FString, FString> ACalibEnvLinker::GetJsonMap() const {
    return outMap;
}