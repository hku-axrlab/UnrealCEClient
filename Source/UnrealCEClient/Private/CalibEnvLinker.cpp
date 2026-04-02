// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibEnvLinker.h"
#include "WebSocketsModule.h"

#include "VirtualRoot.h"
#include <stdio.h>
#include "IJsonData.h"
#include "BaseTemplate.h"

using json = nlohmann::json;

SlotData ACalibEnvLinker::ParseSlot(const json& slotResponse)
{
    SlotData slot;
    const auto& data = slotResponse["data"];

    // ID comes from the slot-level id field
    slot.id = data["id"].get<std::string>();

    // Name and tag are nested under data, each with a "value" field
    slot.name = data["name"]["value"].get<std::string>();
    slot.tag = data["tag"]["value"].is_null() ? "" : data["tag"]["value"].get<std::string>();

    // Position
    const auto& pos = data["position"]["value"];
    slot.transform.x = pos["x"].get<float>();
    slot.transform.y = pos["y"].get<float>();
    slot.transform.z = pos["z"].get<float>();

    // Rotation (quaternion)
    const auto& rot = data["rotation"]["value"];
    slot.transform.qx = rot["x"].get<float>();
    slot.transform.qy = rot["y"].get<float>();
    slot.transform.qz = rot["z"].get<float>();
    slot.transform.qw = rot["w"].get<float>();

    // Scale
    const auto& scl = data["scale"]["value"];
    slot.transform.sx = scl["x"].get<float>();
    slot.transform.sy = scl["y"].get<float>();
    slot.transform.sz = scl["z"].get<float>();

    for (const auto& component : data["components"]) {
        if (component["componentType"].get<std::string>().find("DynamicValueVariable\u003C") != std::string::npos) {
            std::string type = component["componentType"].get<std::string>();
            FMemberVariable v;
            for (const auto& [mKey, mVal] : component["members"].items()) {
                if (mKey == "VariableName") {
                    v.name = mVal["value"].get<std::string>().c_str();
                }
                if (mKey == "Value") {
                    v.type = mVal["$type"].get<std::string>().c_str();
                    v.value = mVal["value"];
                }
            }
            slot.variables.push_back(v);
        }
    }

    return slot;
}

std::vector<SlotData> ACalibEnvLinker::ParseBatchResponse(const std::string& jsonString)
{
    std::vector<SlotData> slots;

    json root = json::parse(jsonString);
    if (root["responses"].is_null()) return slots;

    for (const auto& response : root["responses"])
    {
        // Skip any responses that aren't slotData
        if (response["$type"] == "slotData")
            slots.push_back(ParseSlot(response));
    }

    return slots;
}

void ACalibEnvLinker::HandleSlotSpawning(const std::vector<SlotData>& slots)
{
    for (auto slot : slots)
    {
        if (slot.tag == "vRoot")
        {
            AVirtualRoot::proxyPosition.X = slot.transform.x;
            AVirtualRoot::proxyPosition.Y = slot.transform.y;
            AVirtualRoot::proxyPosition.Z = slot.transform.z;

            AVirtualRoot::proxyRotation.X = slot.transform.qx;
            AVirtualRoot::proxyRotation.Y = slot.transform.qy;
            AVirtualRoot::proxyRotation.Z = slot.transform.qz;
            AVirtualRoot::proxyRotation.W = slot.transform.qw;
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
            FQuat rotation(slot.transform.qx, slot.transform.qz, slot.transform.qy, -slot.transform.qw);
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
                    actor->SetActorLocationAndRotation(AVirtualRoot::TransformPosition(position), AVirtualRoot::TransformRotation(rotation));
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
        std::vector<SlotData> slotsFound = ParseBatchResponse(TCHAR_TO_UTF8(*Message));
        HandleSlotSpawning(slotsFound);
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