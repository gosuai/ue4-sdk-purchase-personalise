// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesController.h"

#include "GosuPurchases.h"
#include "GosuPurchasesDataModel.h"
#include "GosuPurchasesDefines.h"
#include "GosuPurchasesSave.h"
#include "GosuPurchasesSettings.h"

#include "Engine.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FGosuPurchasesModule"

UGosuPurchasesController::UGosuPurchasesController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGosuPurchasesController::Initialize()
{
	LoadData();

	const UGosuPurchasesSettings* Settings = FGosuPurchasesModule::Get().GetSettings();
	if (Settings->bDevelopmentMode)
	{
#if UE_BUILD_SHIPPING
		UE_LOG(LogGosuPurchases, Warning, TEXT("%s: Developmend mode should be disabled in Shipping build"), *VA_FUNC_LINE);
#endif
		SecretKey = Settings->SecretKeyDevelopment;
	}
	else
	{
		SecretKey = Settings->SecretKeyProduction;
	}
}

bool UGosuPurchasesController::HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
{
	FString ErrorStr;
	int32 ErrorCode = 0;
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
		UE_LOG(LogGosuPurchases, Warning, TEXT("%s: request failed (%s): %s"), *VA_FUNC_LINE, *ErrorStr, *ResponseStr);
		ErrorCallback.ExecuteIfBound(StatusCode, ErrorCode, ErrorStr);
		return true;
	}

	return false;
}

void UGosuPurchasesController::LoadData()
{
	// @TODO UGosuPurchasesSave::Load()
}

void UGosuPurchasesController::SaveData()
{
	// @TODO UGosuPurchasesSave::Save(...);
}

bool UGosuPurchasesController::IsDevelopmentModeEnabled() const
{
	const UGosuPurchasesSettings* Settings = FGosuPurchasesModule::Get().GetSettings();
	bool bDevelopmentModeEnabled = Settings->bDevelopmentMode;

#if UE_BUILD_SHIPPING
	if (bDevelopmentModeEnabled)
	{
		UE_LOG(LogGosuPurchases, Warning, TEXT("%s: Development mode should be disabled in Shipping build"), *VA_FUNC_LINE);
	}
#endif // UE_BUILD_SHIPPING

	return bDevelopmentModeEnabled;
}

TSharedRef<IHttpRequest> UGosuPurchasesController::CreateHttpRequest(const FString& Url, const FString& BodyContent, ERequestVerb Verb)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);

	HttpRequest->SetHeader(TEXT("X-ENGINE"), TEXT("UE4"));
	HttpRequest->SetHeader(TEXT("X-ENGINE-V"), ENGINE_VERSION_STRING);
	HttpRequest->SetHeader(TEXT("X-SDK-V"), GOSU_PURCHASES_VERSION);

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
		uint8 HMACHash[20];
		FSHA1::HMACBuffer(*SecretKey, SecretKey.Len(), *BodyContent, BodyContent.Len(), HMACHash);
		FString RequestHash = FBase64::Encode(HMACHash, ARRAY_COUNT(HMACHash));
		HttpRequest->SetHeader(TEXT("Authorization"), RequestHash);
	}
	else
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't setup request auth: SecretKey is empty"), *VA_FUNC_LINE);
	}

	return HttpRequest;
}

#undef LOCTEXT_NAMESPACE
