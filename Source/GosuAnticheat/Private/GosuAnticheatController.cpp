// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuAnticheatController.h"

#include "GosuAnticheat.h"
#include "GosuAnticheatDataModel.h"
#include "GosuAnticheatDefines.h"
#include "GosuAnticheatLibrary.h"
#include "GosuAnticheatSave.h"

#include "GosuPurchases.h"
#include "GosuPurchasesController.h"
#include "GosuPurchasesLibrary.h"
#include "GosuPurchasesSettings.h"

#include "Engine.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Base64.h"
#include "Misc/Guid.h"
#include "Misc/SecureHash.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FGosuAnticheatModule"

const FString UGosuAnticheatController::GosuApiEndpoint(TEXT("https://stormy-depths-03783.herokuapp.com/api/v1/anticheat"));

UGosuAnticheatController::UGosuAnticheatController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FlushTimeAccumulator = 0.f;
}

void UGosuAnticheatController::Tick(float DeltaTime)
{
	FlushTimeAccumulator += DeltaTime;
	if (FlushTimeAccumulator >= 5.f)
	{
		FlushEvents();
		FlushTimeAccumulator = 0.f;
	}
}

void UGosuAnticheatController::Initialize(const FString& InAppId, const FString& InSecretKey)
{
	// Pre-cache initialization data
	SecretKey = InSecretKey;
	AppId = InAppId;

	// Load cached recommendations
	LoadData();

	UE_LOG(LogGosuAnticheat, Log, TEXT("%s: Controller initialized: %s"), *VA_FUNC_LINE, *AppId);
}

void UGosuAnticheatController::ServerMatchStateChanged(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& Map, const FString& GameMode, bool IsRanked, const TArray<FGosuPlayerState>& PlayerStates, const TArray<FGosuTeamState>& TeamStates)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("matchId"), MatchId);
	RequestDataJson->SetStringField(TEXT("matchState"), GetMatchStatusAsString(MatchStatus));
	RequestDataJson->SetStringField(TEXT("map"), Map);
	RequestDataJson->SetStringField(TEXT("gameMode"), GameMode);
	RequestDataJson->SetBoolField(TEXT("ranked"), IsRanked);
	RequestDataJson->SetNumberField(TEXT("matchTime"), MatchTime);

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	for (auto& PlayerData : PlayerStates)
	{
		if (FJsonObjectConverter::UStructToJsonObject(FGosuPlayerState::StaticStruct(), &PlayerData, JsonObject, 0, 0))
		{
			JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
		}
	}
	RequestDataJson->SetArrayField(TEXT("players"), JsonArray);

	JsonArray.Empty();
	for (auto& TeamData : TeamStates)
	{
		if (FJsonObjectConverter::UStructToJsonObject(FGosuTeamState::StaticStruct(), &TeamData, JsonObject, 0, 0))
		{
			JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
		}
	}
	RequestDataJson->SetArrayField(TEXT("teams"), JsonArray);

	const FString Url = FString::Printf(TEXT("%s/match/%s/state"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuAnticheatController::Generic_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuAnticheatController::ServerPlayerJoin(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("matchId"), MatchId);
	RequestDataJson->SetStringField(TEXT("uid"), PlayerId);
	RequestDataJson->SetStringField(TEXT("netId"), PlayerNetId);
	RequestDataJson->SetStringField(TEXT("matchState"), GetMatchStatusAsString(MatchStatus));
	RequestDataJson->SetNumberField(TEXT("matchTime"), MatchTime);
	RequestDataJson->SetStringField(TEXT("nickname"), PlayerNickname);
	RequestDataJson->SetNumberField(TEXT("rating"), PlayerRating);

	const FString Url = FString::Printf(TEXT("%s/match/%s/playerJoin"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuAnticheatController::Generic_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuAnticheatController::ServerPlayerLeave(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("matchId"), MatchId);
	RequestDataJson->SetStringField(TEXT("uid"), PlayerId);
	RequestDataJson->SetStringField(TEXT("netId"), PlayerNetId);
	RequestDataJson->SetStringField(TEXT("matchState"), GetMatchStatusAsString(MatchStatus));
	RequestDataJson->SetNumberField(TEXT("matchTime"), MatchTime);
	RequestDataJson->SetStringField(TEXT("nickname"), PlayerNickname);
	RequestDataJson->SetNumberField(TEXT("rating"), PlayerRating);

	const FString Url = FString::Printf(TEXT("%s/match/%s/playerLeave"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuAnticheatController::Generic_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuAnticheatController::SendCustomEvent(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating, const TArray<FGosuCustomEvent>& CustomData)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("matchId"), MatchId);
	RequestDataJson->SetStringField(TEXT("matchState"), GetMatchStatusAsString(MatchStatus));
	RequestDataJson->SetNumberField(TEXT("matchTime"), MatchTime);
	RequestDataJson->SetStringField(TEXT("eventUUID"), FGuid::NewGuid().ToString());
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("uid"), PlayerId);
	RequestDataJson->SetStringField(TEXT("netId"), PlayerNetId);
	RequestDataJson->SetStringField(TEXT("nickname"), PlayerNickname);
	RequestDataJson->SetNumberField(TEXT("rating"), PlayerRating);

	TSharedPtr<FJsonObject> CustomDataJson = MakeShareable(new FJsonObject);
	for (auto& EventData : CustomData)
	{
		CustomDataJson->SetStringField(EventData.ParamName, EventData.ParamValue);
	}
	RequestDataJson->SetObjectField(TEXT("data"), CustomDataJson);

	CustomEvents.Add(MakeShareable(new FJsonValueObject(RequestDataJson)));
}

void UGosuAnticheatController::CheckUserStatus(const FString& PlayerId, const FString& PlayerNetId, const FOnReceivePlayerStatus& SuccessCallback, const FOnRequestError& ErrorCallback)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), PlayerId);
	RequestDataJson->SetStringField(TEXT("netId"), PlayerNetId);

	const FString Url = FString::Printf(TEXT("%s/user/%s/check"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuAnticheatController::CheckUserStatus_HttpRequestComplete, SuccessCallback, ErrorCallback);
	HttpRequest->ProcessRequest();
}

void UGosuAnticheatController::FlushEvents()
{
	if (CustomEvents.Num() == 0)
	{
		// Do nothing if we haven't any events
		return;
	}

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetArrayField(TEXT("events"), CustomEvents);
	CustomEvents.Empty();

	const FString Url = FString::Printf(TEXT("%s/event/%s/send"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuAnticheatController::Generic_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuAnticheatController::Generic_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, FOnRequestError()))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogGosuAnticheat, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);
}

void UGosuAnticheatController::CheckUserStatus_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceivePlayerStatus SuccessCallback, FOnRequestError ErrorCallback)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, ErrorCallback))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogGosuAnticheat, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*HttpResponse->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogGosuAnticheat, Error, TEXT("%s: Can't deserialize server response"), *VA_FUNC_LINE);
		return;
	}

	FString uid = JsonObject->GetStringField("uid");
	FString status = JsonObject->GetStringField("status");

	SuccessCallback.ExecuteIfBound(uid, status.Equals(TEXT("EGosuPlayerStatus::Banned")) ? EGosuPlayerStatus::Banned : EGosuPlayerStatus::Default);
}

bool UGosuAnticheatController::HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
{
	FString ErrorStr;
	int32 StatusCode = 204;
	FString ResponseStr = TEXT("invalid");

	if (bSucceeded && HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();

		if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			StatusCode = HttpResponse->GetResponseCode();
			ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"), HttpResponse->GetResponseCode(), *ResponseStr);

			// Example: {"errorMessage":"Bad authorization key"}
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseStr);
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				static const FString ErrorFieldName = TEXT("errorMessage");
				if (JsonObject->HasTypedField<EJson::String>(ErrorFieldName))
				{
					ErrorStr = JsonObject->GetStringField(ErrorFieldName);
				}
				else
				{
					ErrorStr = FString::Printf(TEXT("Can't deserialize error json: no field '%s' found"), *ErrorFieldName);
				}
			}
			else
			{
				ErrorStr = TEXT("Can't deserialize error json");
			}
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogGosuAnticheat, Warning, TEXT("%s: request failed (%s): %s"), *VA_FUNC_LINE, *ErrorStr, *ResponseStr);
		ErrorCallback.ExecuteIfBound(StatusCode, ErrorStr);
		return true;
	}

	return false;
}

FString UGosuAnticheatController::GetMatchStatusAsString(EGosuMatchStatus EnumValue) const
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EGosuMatchStatus"), true))
	{
		return EnumPtr->GetNameByValue((int64)EnumValue).ToString();
	}

	return FString("Invalid");
}

void UGosuAnticheatController::LoadData()
{
	auto SavedData = UGosuAnticheatSave::Load();
}

void UGosuAnticheatController::SaveData()
{
	UGosuAnticheatSave::Save(FGosuAnticheatSaveData());
}

bool UGosuAnticheatController::IsDevelopmentModeEnabled() const
{
	const UGosuPurchasesSettings* Settings = FGosuPurchasesModule::Get().GetSettings();
	bool bDevelopmentModeEnabled = Settings->bDevelopmentMode;

#if UE_BUILD_SHIPPING
	if (bDevelopmentModeEnabled)
	{
		UE_LOG(LogGosuAnticheat, Warning, TEXT("%s: Development mode should be disabled in Shipping build"), *VA_FUNC_LINE);
	}
#endif // UE_BUILD_SHIPPING

	return bDevelopmentModeEnabled;
}

TSharedRef<IHttpRequest> UGosuAnticheatController::CreateHttpRequest(const FString& Url, const FString& BodyContent, ERequestVerb Verb)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);

	HttpRequest->SetHeader(TEXT("X-ENGINE"), TEXT("UE4"));
	HttpRequest->SetHeader(TEXT("X-ENGINE-V"), ENGINE_VERSION_STRING);
	HttpRequest->SetHeader(TEXT("X-SDK-V"), GOSU_ANTICHEAT_VERSION);

	switch (Verb)
	{
	case ERequestVerb::GET:
		HttpRequest->SetVerb(TEXT("GET"));
		break;

	case ERequestVerb::POST:
		HttpRequest->SetVerb(TEXT("POST"));
		break;

	default:
		unimplemented();
	}

	if (!BodyContent.IsEmpty())
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		HttpRequest->SetContentAsString(BodyContent);
	}

	// Authorization
	if (!SecretKey.IsEmpty())
	{
		FString RequestHash = FMD5::HashAnsiString(*(BodyContent + SecretKey));
		HttpRequest->SetHeader(TEXT("Authorization"), FBase64::Encode(RequestHash));
	}
	else
	{
		UE_LOG(LogGosuAnticheat, Error, TEXT("%s: Can't setup request auth: SecretKey is empty"), *VA_FUNC_LINE);
	}

	return HttpRequest;
}

FString UGosuAnticheatController::SerializeJson(const TSharedPtr<FJsonObject> DataJson) const
{
	FString JsonContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonContent);
	FJsonSerializer::Serialize(DataJson.ToSharedRef(), Writer);
	return JsonContent;
}

bool UGosuAnticheatController::CheckUserId() const
{
	return UGosuPurchasesLibrary::GetPurchasesController(this)->CheckUserId();
}

#undef LOCTEXT_NAMESPACE
