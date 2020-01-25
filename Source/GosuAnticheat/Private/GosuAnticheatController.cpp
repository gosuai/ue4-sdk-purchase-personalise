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

const FString UGosuAnticheatController::GosuApiEndpoint(TEXT("https://gosutag.com/api/v1"));

UGosuAnticheatController::UGosuAnticheatController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FlushTimeAccumulator = 0.f;
}

void UGosuAnticheatController::Tick(float DeltaTime)
{
	FlushTimeAccumulator += DeltaTime;
	if (FlushTimeAccumulator >= 1.f)
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

void UGosuAnticheatController::FlushEvents()
{
}

#undef LOCTEXT_NAMESPACE
