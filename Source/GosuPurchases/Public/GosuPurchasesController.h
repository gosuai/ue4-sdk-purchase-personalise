// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuPurchasesDataModel.h"

#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "Interfaces/OnlineStoreInterface.h"

#include "GosuPurchasesController.generated.h"

class FJsonObject;
struct FGuid;

/** Verb used by the request */
UENUM(BlueprintType)
enum class ERequestVerb : uint8
{
	GET,
	POST
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnReceiveRecommendation, const FGosuRecommendation&, Recommendation);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestError, int32, StatusCode, const FString&, ErrorMessage);

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesController : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void Initialize();

	/** Register game session with provided player id */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectSession(const FString& PlayerId);

	/** Generates unique impression id for futher events */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectStoreOpened();

	/**
	 * Item is shown at store page right now 
	 *
	 * @param Scenario 
	 * @param Category 
	 * @param ItemSKU 
	 * @param ItemName 
	 * @param Price 
	 * @param Currency
	 * @param Description (optional) 
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectShowcaseItemShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item was hidden at store page */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectShowcaseItemHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU);

	/**
	 * Item details window was shown
	 *
	 * @param Scenario 
	 * @param Category 
	 * @param ItemSKU 
	 * @param ItemName 
	 * @param Price 
	 * @param Currency
	 * @param Description (optional) 
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectItemDetailsShow(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item details window was closed */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectItemDetailsHide(ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU);

	/** Player triggered purchase process for item */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectPurchaseStarted(const FString& ItemSKU);

	/**
	 * Purchase state was updated
	 *
	 * @param ItemSKU 
	 * @param PurchaseState 
	 * @param TransactionID  
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect")
	void CollectPurchaseCompleted(const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID = TEXT(""));

	/**
	 * Receive recommended items for desired scenario and store category
	 *
	 * @param Scenario 
	 * @param Category (optional) 
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Receive", meta = (AutoCreateRefTerm = "SuccessCallback"))
	void GetRecommendations(ERecommendationScenario Scenario, const FString& Category, const FOnReceiveRecommendation& SuccessCallback);

protected:
	void GetRecommendations_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceiveRecommendation SuccessCallback);

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

public:
	/** Get recommendated items for desired category */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Data")
	TArray<FGosuRecommendedItem> GetRecommendedItems(ERecommendationScenario Scenario) const;

protected:
	/** Cached recommendations storage */
	TMap<ERecommendationScenario, FGosuRecommendation> Recommendations;

	/** Cached user id */
	FString UserID;

	/** Cached impression id (store opened event) */
	FGuid ImpressionId;

protected:
	static const FString GosuApiEndpoint;

private:
	/** Cached secret key */
	FString SecretKey;

	/** Cached AppId */
	FString AppId;
};
