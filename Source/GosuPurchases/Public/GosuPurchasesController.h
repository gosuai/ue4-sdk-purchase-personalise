// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuPurchasesDataModel.h"

#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "Interfaces/OnlineStoreInterface.h"
#include "Tickable.h"

#include "GosuPurchasesController.generated.h"

class APlayerController;
class FJsonObject;
struct FGuid;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFetchRecommendation, const FGosuRecommendation&, Recommendation);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnReceiveRecommendation, const FGosuRecommendation&, Recommendation);

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesController : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

protected:
	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual TStatId GetStatId() const override { return TStatId(); }
	// FTickableGameObject end

public:
	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void Initialize(const FString& InAppId, const FString& InSecretKey);

	/** Register game session with provided player id */
	void RegisterSession(APlayerController* PlayerController, const FString& PlayerId);

	/** Register game session while using SteamOnlineSubsystem */
	void RegisterSteamSession(APlayerController* PlayerController);

protected:
	/** Sessions is started with PlayerId  */
	void CallRegisterSession(const FString& PlayerId);

public:
	/** Generates unique impression id for futher events */
	void RegisterStoreOpened();

	/** Item is shown at store page right now  */
	void CallShowcaseItemShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item was hidden at store page */
	void CallShowcaseItemHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU);

	/** Item details window was shown */
	void CallItemDetailsShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item details window was closed */
	void CallItemDetailsHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU);

	/** Player triggered purchase process for item */
	void CallPurchaseStarted(const FString& ItemSKU);

	/** Purchase state was updated */
	void CallPurchaseCompleted(const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID = TEXT(""));

	/** Receive recommended items for desired scenario and store category, limited with MaxItems cap */
	void FetchRecommendations(ERecommendationScenario Scenario, const FString& Category, const FOnReceiveRecommendation& SuccessCallback, int32 MaxItems = 20);

protected:
	void CallEvent_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void FetchRecommendations_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceiveRecommendation SuccessCallback);

	/** Return true if error is happened */
	bool HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);

protected:
	/** Load save game and extract data */
	void LoadData();

	/** Save cached data or reset one if necessary */
	void SaveData();

	/** Check is we're in development mode for now or not */
	bool IsDevelopmentModeEnabled() const;

	/** Create http request and add API meta */
	TSharedRef<IHttpRequest> CreateHttpRequest(const FString& Url, const FString& BodyContent = TEXT(""), ERequestVerb Verb = ERequestVerb::POST);

	/** Serialize json object into string */
	FString SerializeJson(const TSharedPtr<FJsonObject> DataJson) const;

	/** Convert enum to string */
	FString GetInAppPurchaseStateAsString(EInAppPurchaseState::Type EnumValue) const;
	FString GetScenarioAsString(ERecommendationScenario EnumValue) const;

public:
	/** Helper functions to check filled stuff */
	bool CheckUserId() const;
	bool CheckImpressionId() const;

protected:
	/** Flush showcase events */
	void FlushEvents();

public:
	/** Get recommendated items for desired category */
	TArray<FGosuRecommendedItem> GetRecommendedItems(ERecommendationScenario Scenario, const FString& StoreCategory) const;

	/** Event occured when recommended items were fetched */
	UPROPERTY(BlueprintAssignable, Category = "GOSU|Purchases|Callback")
	FOnFetchRecommendation OnFetchRecommendation;

	/** Get cached user id */
	FString GetUserID() const;

protected:
	/** Cached recommendations storage */
	TArray<FGosuRecommendation> Recommendations;

	/** Cached user id */
	FString UserID;

	/** Cached impression id (store opened event) */
	FGuid ImpressionId;

protected:
	static const FString GosuApiEndpoint;

private:
	/** Cached AppId */
	FString AppId;

	/** Cached secret key */
	FString SecretKey;

	/** Cached showcase events to be sent in bundle */
	TArray<FGosuShowcaseEvent> ShowcaseEvents;

	/** Events flush time handler */
	float FlushTimeAccumulator;
};
