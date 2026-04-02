// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibEnvLinker.h"
#include "WebSocketsModule.h"

// Drop nlohmann/json.hpp into Source/ThirdParty/nlohmann/
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Sets default values
ACalibEnvLinker::ACalibEnvLinker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
        const json Parsed = json::parse(TCHAR_TO_UTF8(*Message));

        // TODO: Parse the objects for all the stuff we need:

        // Loop through all children:
        
            //  Get: ID, Tag, Transform

            // Spawn or Get object from spawnedObjects
            
            // Apply transform
            // Check if obj implements UJsonObject
                // if so: parse & send variable data (maybe a separate function? or do we implement this in Blueprints?)

    }
    catch (const json::parse_error& e)
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