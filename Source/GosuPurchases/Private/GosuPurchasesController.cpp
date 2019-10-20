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
#include "Kismet/GameplayStatics.h"
#include "Misc/Base64.h"
#include "Misc/Guid.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FGosuPurchasesModule"

const FString UGosuPurchasesController::GosuApiEndpoint(TEXT("https://localhost:3000/api/"));

UGosuPurchasesController::UGosuPurchasesController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGosuPurchasesController::Initialize(UWorld* World)
{
	LoadData();

	// Pre-cache secret key
	const UGosuPurchasesSettings* Settings = FGosuPurchasesModule::Get().GetSettings();
	if (IsDevelopmentModeEnabled())
	{
		SecretKey = Settings->SecretKeyDevelopment;
	}
	else
	{
		SecretKey = Settings->SecretKeyProduction;
	}

	// Cache app id
	AppId = Settings->AppId;

	// Set flushing timer
	World->GetTimerManager().SetTimer(FlushTimerHandle, FTimerDelegate::CreateUObject(this, &UGosuPurchasesController::FlushEvents), 1.f, true);
}

void UGosuPurchasesController::CollectSession(const FString& PlayerId)
{
	UserID = PlayerId;

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	FString PlatformName = UGameplayStatics::GetPlatformName();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("platform"), PlatformName);

	const FString Url = FString::Printf(TEXT("%s/collect/%s/session"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CollectStoreOpened()
{
	// No requests, just impressionId generation
	ImpressionId = FGuid::NewGuid();
}

void UGosuPurchasesController::CollectShowcaseItemShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	FGosuItemInfo ItemInfo;
	ItemInfo.name = ItemName;
	ItemInfo.price = Price;
	ItemInfo.currency = Currency;
	ItemInfo.description = Description;

	FGosuShowcaseEvent Event;
	Event.EventType = EGosuShowcaseEventType::Show;
	Event.impressionId = ImpressionId.ToString();
	Event.scenario = Scenario;
	Event.sku = ItemSKU;
	Event.category = Category;
	Event.timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	Event.eventUUID = FGuid::NewGuid().ToString();
	Event.item = ItemInfo;

	ShowcaseEvents.Add(Event);
}

void UGosuPurchasesController::CollectShowcaseItemHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	FGosuShowcaseEvent Event;
	Event.EventType = EGosuShowcaseEventType::Hide;
	Event.impressionId = ImpressionId.ToString();
	Event.scenario = Scenario;
	Event.sku = ItemSKU;
	Event.category = Category;
	Event.timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	Event.eventUUID = FGuid::NewGuid().ToString();

	ShowcaseEvents.Add(Event);
}

void UGosuPurchasesController::CollectItemDetailsShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("impressionId"), ImpressionId.ToString());
	RequestDataJson->SetStringField(TEXT("eventUUID"), FGuid::NewGuid().ToString());
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("scenario"), GetScenarioAsString(Scenario));
	RequestDataJson->SetStringField(TEXT("category"), Category);
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);

	TSharedPtr<FJsonObject> ItemDataJson = MakeShareable(new FJsonObject);
	ItemDataJson->SetStringField(TEXT("name"), ItemName);
	ItemDataJson->SetNumberField(TEXT("price"), Price);
	ItemDataJson->SetStringField(TEXT("currency"), Currency);
	ItemDataJson->SetStringField(TEXT("description"), Description);
	RequestDataJson->SetObjectField(TEXT("item"), ItemDataJson);

	const FString Url = FString::Printf(TEXT("%s/collect/%s/showcase/preview_open"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CollectItemDetailsHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("impressionId"), ImpressionId.ToString());
	RequestDataJson->SetStringField(TEXT("eventUUID"), FGuid::NewGuid().ToString());
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("scenario"), GetScenarioAsString(Scenario));
	RequestDataJson->SetStringField(TEXT("category"), Category);
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);

	const FString Url = FString::Printf(TEXT("%s/collect/%s/showcase/preview_close"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CollectPurchaseStarted(const FString& ItemSKU)
{
	if (!CheckUserId())
	{
		return;
	}

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	const FGuid EventUUID = FGuid::NewGuid();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("eventUUID"), EventUUID.ToString());
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);

	const FString Url = FString::Printf(TEXT("%s/collect/%s/purchase_started"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CollectPurchaseCompleted(const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID)
{
	if (!CheckUserId())
	{
		return;
	}

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	const FGuid EventUUID = FGuid::NewGuid();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("eventUUID"), EventUUID.ToString());
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);
	RequestDataJson->SetStringField(TEXT("transaction"), TransactionID);
	RequestDataJson->SetStringField(TEXT("status"), GetInAppPurchaseStateAsString(PurchaseState));

	const FString Url = FString::Printf(TEXT("%s/collect/%s/purchase_completed"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::GetRecommendations(ERecommendationScenario Scenario, const FString& Category, const FOnReceiveRecommendation& SuccessCallback)
{
	const FString Url = FString::Printf(TEXT("%s/recommend/%s/store/%s?category=%s"), *GosuApiEndpoint, *AppId, *UserID, *Category);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, FString(), ERequestVerb::GET);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::GetRecommendations_HttpRequestComplete, SuccessCallback);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::GetRecommendations_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceiveRecommendation SuccessCallback)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, FOnRequestError()))
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*HttpResponse->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't deserialize server response"), *VA_FUNC_LINE);
		return;
	}

	FGosuRecommendation Recommendation;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FGosuRecommendation::StaticStruct(), &Recommendation))
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't convert server response to struct"), *VA_FUNC_LINE);
		return;
	}

	Recommendations.Add(Recommendation.scenario, Recommendation);

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogGosuPurchases, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);

	SuccessCallback.ExecuteIfBound(Recommendation);
}

bool UGosuPurchasesController::HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
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
		UE_LOG(LogGosuPurchases, Warning, TEXT("%s: request failed (%s): %s"), *VA_FUNC_LINE, *ErrorStr, *ResponseStr);
		ErrorCallback.ExecuteIfBound(StatusCode, ErrorStr);
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

FString UGosuPurchasesController::SerializeJson(const TSharedPtr<FJsonObject> DataJson) const
{
	FString JsonContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonContent);
	FJsonSerializer::Serialize(DataJson.ToSharedRef(), Writer);
	return JsonContent;
}

FString UGosuPurchasesController::GetInAppPurchaseStateAsString(EInAppPurchaseState::Type EnumValue) const
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EInAppPurchaseState"), true))
	{
		return EnumPtr->GetNameByValue((int64)EnumValue).ToString();
	}

	return FString("Invalid");
}

FString UGosuPurchasesController::GetScenarioAsString(ERecommendationScenario EnumValue) const
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERecommendationScenario"), true))
	{
		return EnumPtr->GetNameByValue((int64)EnumValue).ToString();
	}

	return FString("Invalid");
}

bool UGosuPurchasesController::CheckUserId() const
{
	if (UserID.IsEmpty())
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't process request: userId is invalid"), *VA_FUNC_LINE);
		return false;
	}

	return true;
}

bool UGosuPurchasesController::CheckImpressionId() const
{
	if (ImpressionId.IsValid())
	{
		return true;
	}

	UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't process request: impressionId is invalid"), *VA_FUNC_LINE);
	return false;
}

void UGosuPurchasesController::FlushEvents()
{
	if (ShowcaseEvents.Num() == 0)
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> ShowEvents;
	TArray<TSharedPtr<FJsonValue>> HideEvents;

	for (auto& Event : ShowcaseEvents)
	{
		TSharedPtr<FJsonObject> EventJson = MakeShareable(new FJsonObject);
		EventJson->SetStringField(TEXT("impressionId"), Event.impressionId);
		EventJson->SetStringField(TEXT("eventUUID"), Event.eventUUID);
		EventJson->SetNumberField(TEXT("timestamp"), Event.timestamp);
		EventJson->SetStringField(TEXT("scenario"), GetScenarioAsString(Event.scenario));
		EventJson->SetStringField(TEXT("category"), Event.category);
		EventJson->SetStringField(TEXT("sku"), Event.sku);

		if (!Event.item.name.IsEmpty())
		{
			TSharedPtr<FJsonObject> ItemJson = MakeShareable(new FJsonObject);
			ItemJson->SetStringField(TEXT("name"), Event.item.name);
			ItemJson->SetNumberField(TEXT("price"), Event.item.price);
			ItemJson->SetStringField(TEXT("currency"), Event.item.currency);
			ItemJson->SetStringField(TEXT("description"), Event.item.description);
			EventJson->SetObjectField(TEXT("item"), ItemJson);
		}

		TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueObject(EventJson));

		switch (Event.EventType)
		{
		case EGosuShowcaseEventType::Show:
			ShowEvents.Add(NewVal);
			break;
		case EGosuShowcaseEventType::Hide:
			HideEvents.Add(NewVal);
			break;
		default:
			unimplemented();
		}
	}

	ShowcaseEvents.Empty();

	if (ShowEvents.Num() > 0)
	{
		TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
		RequestDataJson->SetStringField(TEXT("uid"), UserID);
		RequestDataJson->SetArrayField(TEXT("events"), ShowEvents);

		const FString Url = FString::Printf(TEXT("%s/collect/%s/showcase/show"), *GosuApiEndpoint, *AppId);

		TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
		HttpRequest->ProcessRequest();
	}

	if (HideEvents.Num() > 0)
	{
		TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
		RequestDataJson->SetStringField(TEXT("uid"), UserID);
		RequestDataJson->SetArrayField(TEXT("events"), HideEvents);

		const FString Url = FString::Printf(TEXT("%s/collect/%s/showcase/hide"), *GosuApiEndpoint, *AppId);

		TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
		HttpRequest->ProcessRequest();
	}
}

TArray<FGosuRecommendedItem> UGosuPurchasesController::GetRecommendedItems(ERecommendationScenario Scenario) const
{
	return TArray<FGosuRecommendedItem>();
}

#undef LOCTEXT_NAMESPACE
