// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GosuPurchasesController.h"
#include "GosuPurchasesDataModel.h"

#include "Interfaces/OnlineStoreInterface.h"

#include "GosuPurchasesLibrary.generated.h"

class UGosuPurchasesSettings;

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Direct access to purchases controller */
	static UGosuPurchasesController* GetPurchasesController(UObject* WorldContextObject);

	/** Direct access to purchases settings */
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases", meta = (WorldContext = "WorldContextObject"))
	static UGosuPurchasesSettings* GetPurchasesSettings(UObject* WorldContextObject);

	/** Register game session with provided player id */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectSession(UObject* WorldContextObject, const FString& PlayerId);

	/** Generates unique impression id for futher events */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectStoreOpened(UObject* WorldContextObject);

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
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectShowcaseItemShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item was hidden at store page */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectShowcaseItemHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU);

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
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectItemDetailsShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item details window was closed */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectItemDetailsHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU);

	/** Player triggered purchase process for item */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectPurchaseStarted(UObject* WorldContextObject, const FString& ItemSKU);

	/**
	 * Purchase state was updated
	 *
	 * @param ItemSKU 
	 * @param PurchaseState 
	 * @param TransactionID  
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Collect", meta = (WorldContext = "WorldContextObject"))
	static void CollectPurchaseCompleted(UObject* WorldContextObject, const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID = TEXT(""));

	/**
	 * Fetch recommended items from GOSU server for desired scenario and store category
	 *
	 * @param Scenario 
	 * @param Category (optional) 
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Receive", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "SuccessCallback"))
	static void FetchRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FOnReceiveRecommendation& SuccessCallback);

	/** Get cached recommended items for desired category (call ReceiveRecommendations to have fresh data) */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Data", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGosuRecommendedItem> GetRecommendedItems(UObject* WorldContextObject, ERecommendationScenario Scenario);
};
