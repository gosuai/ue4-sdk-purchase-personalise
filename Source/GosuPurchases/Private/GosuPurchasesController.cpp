// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesController.h"

#include "GosuPurchases.h"
#include "GosuPurchasesDataModel.h"
#include "GosuPurchasesDefines.h"
#include "GosuPurchasesLibrary.h"
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
#include "Online.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FGosuPurchasesModule"

const FString UGosuPurchasesController::GosuApiEndpoint(TEXT("https://stormy-depths-03783.herokuapp.com/api/v1"));

UGosuPurchasesController::UGosuPurchasesController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FlushTimeAccumulator = 0.f;
}

void UGosuPurchasesController::Tick(float DeltaTime)
{
	FlushTimeAccumulator += DeltaTime;
	if (FlushTimeAccumulator >= 1.f)
	{
		FlushEvents();
		FlushTimeAccumulator = 0.f;
	}
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

	UE_LOG(LogGosuPurchases, Log, TEXT("%s: Controller initialized: %s"), *VA_FUNC_LINE, *AppId);
}

void UGosuPurchasesController::RegisterSession(APlayerController* PlayerController, const FString& PlayerId)
{
	if (IOnlineSubsystem::IsEnabled(STEAM_SUBSYSTEM))
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: SteamOnlineSubsystem is enabled: forcing to register Steam session"), *VA_FUNC_LINE);
		RegisterSteamSession(PlayerController);
		return;
	}

	CallRegisterSession(PlayerId);
}

void UGosuPurchasesController::RegisterSteamSession(APlayerController* PlayerController)
{
	CallRegisterSession(UGosuPurchasesLibrary::GetUniquePlayerId(PlayerController));
}

void UGosuPurchasesController::CallRegisterSession(const FString& PlayerId)
{
	if (PlayerId.IsEmpty())
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Can't register session with empty PlayerId"), *VA_FUNC_LINE);
		return;
	}

	UserID = PlayerId;

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	FString PlatformName = IOnlineSubsystem::GetLocalPlatformName();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("platform"), PlatformName);

	const FString Url = FString::Printf(TEXT("%s/event/%s/session"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::RegisterStoreOpened()
{
	// No requests, just impressionId generation
	ImpressionId = FGuid::NewGuid();

	UE_LOG(LogGosuPurchases, Log, TEXT("%s: Impression registered: %s"), *VA_FUNC_LINE, *ImpressionId.ToString());
}

void UGosuPurchasesController::CallShowcaseItemShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
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

void UGosuPurchasesController::CallShowcaseItemHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
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

void UGosuPurchasesController::CallItemDetailsShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
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

	const FString Url = FString::Printf(TEXT("%s/event/%s/showcase/preview_open"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CallItemDetailsHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
{
	if (!CheckUserId() || !CheckImpressionId())
	{
		return;
	}

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
	RequestDataJson->SetStringField(TEXT("impressionId"), ImpressionId.ToString());
	RequestDataJson->SetStringField(TEXT("eventUUID"), FGuid::NewGuid().ToString());
	RequestDataJson->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	RequestDataJson->SetStringField(TEXT("scenario"), GetScenarioAsString(Scenario));
	RequestDataJson->SetStringField(TEXT("category"), Category);
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);

	const FString Url = FString::Printf(TEXT("%s/event/%s/showcase/preview_close"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CallPurchaseStarted(const FString& ItemSKU)
{
	if (!CheckUserId())
	{
		return;
	}

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	const FGuid EventUUID = FGuid::NewGuid();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("eventUUID"), EventUUID.ToString());
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);

	const FString Url = FString::Printf(TEXT("%s/event/%s/purchase_started"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CallPurchaseCompleted(const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID)
{
	if (!CheckUserId())
	{
		return;
	}

	const int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	const FGuid EventUUID = FGuid::NewGuid();

	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
	RequestDataJson->SetNumberField(TEXT("timestamp"), Timestamp);
	RequestDataJson->SetStringField(TEXT("eventUUID"), EventUUID.ToString());
	RequestDataJson->SetStringField(TEXT("sku"), ItemSKU);
	RequestDataJson->SetStringField(TEXT("transaction"), TransactionID);
	RequestDataJson->SetStringField(TEXT("status"), GetInAppPurchaseStateAsString(PurchaseState));

	const FString Url = FString::Printf(TEXT("%s/event/%s/purchase_completed"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::FetchRecommendations(ERecommendationScenario Scenario, const FString& Category, const FOnReceiveRecommendation& SuccessCallback, int32 MaxItems)
{
	TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
	RequestDataJson->SetStringField(TEXT("uid"), UserID);
	RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
	RequestDataJson->SetStringField(TEXT("scenario"), GetScenarioAsString(Scenario));
	RequestDataJson->SetStringField(TEXT("category"), Category);
	RequestDataJson->SetNumberField(TEXT("maxItems"), MaxItems);

	const FString Url = FString::Printf(TEXT("%s/recommend/%s/purchases"), *GosuApiEndpoint, *AppId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::FetchRecommendations_HttpRequestComplete, SuccessCallback);
	HttpRequest->ProcessRequest();
}

void UGosuPurchasesController::CallEvent_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, FOnRequestError()))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogGosuPurchases, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);
}

void UGosuPurchasesController::FetchRecommendations_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceiveRecommendation SuccessCallback)
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
	SaveData();
	OnFetchRecommendation.Broadcast(Recommendation);

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
	auto SavedData = UGosuPurchasesSave::Load();
	Recommendations = SavedData.Recommendations;
}

void UGosuPurchasesController::SaveData()
{
	UGosuPurchasesSave::Save(FGosuPurchasesSaveData(Recommendations));
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
		FString RequestHash = FMD5::HashAnsiString(*(BodyContent + SecretKey));
		HttpRequest->SetHeader(TEXT("Authorization"), FBase64::Encode(RequestHash));
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
		RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
		RequestDataJson->SetArrayField(TEXT("events"), ShowEvents);

		const FString Url = FString::Printf(TEXT("%s/event/%s/showcase/show"), *GosuApiEndpoint, *AppId);

		TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
		HttpRequest->ProcessRequest();
	}

	if (HideEvents.Num() > 0)
	{
		TSharedPtr<FJsonObject> RequestDataJson = MakeShareable(new FJsonObject);
		RequestDataJson->SetStringField(TEXT("uid"), UserID);
		RequestDataJson->SetStringField(TEXT("netId"), UGosuPurchasesLibrary::GetUniqueNetId());
		RequestDataJson->SetArrayField(TEXT("events"), HideEvents);

		const FString Url = FString::Printf(TEXT("%s/event/%s/showcase/hide"), *GosuApiEndpoint, *AppId);

		TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, SerializeJson(RequestDataJson));
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGosuPurchasesController::CallEvent_HttpRequestComplete);
		HttpRequest->ProcessRequest();
	}
}

TArray<FGosuRecommendedItem> UGosuPurchasesController::GetRecommendedItems(ERecommendationScenario Scenario) const
{
	if (Recommendations.Contains(Scenario))
	{
		return Recommendations.FindChecked(Scenario).items;
	}

	return TArray<FGosuRecommendedItem>();
}

#undef LOCTEXT_NAMESPACE
