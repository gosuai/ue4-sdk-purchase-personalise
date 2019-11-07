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
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases", meta = (WorldContext = "WorldContextObject"))
	static UGosuPurchasesController* GetPurchasesController(UObject* WorldContextObject);

	/** Direct access to purchases settings */
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases", meta = (WorldContext = "WorldContextObject"))
	static UGosuPurchasesSettings* GetPurchasesSettings(UObject* WorldContextObject);

	/** Register game session with provided player id */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void RegisterSession(UObject* WorldContextObject, APlayerController* PlayerController, const FString& PlayerId);

	/** Register game session while using SteamOnlineSubsystem */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void RegisterSteamSession(UObject* WorldContextObject, APlayerController* PlayerController);

	/** Generates unique impression id for futher events */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void RegisterStoreOpened(UObject* WorldContextObject);

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
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallShowcaseItemShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item was hidden at store page */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallShowcaseItemHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU);

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
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallItemDetailsShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description = TEXT("(optional)"));

	/** (optional) Item details window was closed */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallItemDetailsHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU);

	/** Player triggered purchase process for item */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallPurchaseStarted(UObject* WorldContextObject, const FString& ItemSKU);

	/**
	 * Purchase state was updated
	 *
	 * @param ItemSKU 
	 * @param PurchaseState 
	 * @param TransactionID  
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Events", meta = (WorldContext = "WorldContextObject"))
	static void CallPurchaseCompleted(UObject* WorldContextObject, const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID = TEXT(""));

	/**
	 * Fetch recommended items from GOSU server for desired scenario and store category
	 *
	 * @param Scenario 
	 * @param StoreCategory (optional) Desired category for Highlight scenario
	 * @param MaxItems Limit recommendation size with provided number of items
	 */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases|Receive", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "SuccessCallback"))
	static void FetchRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FOnReceiveRecommendation& SuccessCallback, int32 MaxItems = 20);

	/** Get cached recommended items for desired category (call ReceiveRecommendations to have fresh data)
	 * 
	 * @param Scenario 
	 * @param StoreCategory (optional) Desired category for Highlight scenario
	 */
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases|Data", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGosuRecommendedItem> GetRecommendedItems(UObject* WorldContextObject, ERecommendationScenario Scenario = ERecommendationScenario::Recommended, FString StoreCategory = TEXT(""));

	//////////////////////////////////////////////////////////////////////////
	// Helper functions

	/** Helper function to get unique network id */
	UFUNCTION(BlueprintPure, Category = "GOSU|Tools")
	static bool GetControllerNetworkID(APlayerController* PlayerController, FString& NetworkID, bool bKeepPort = false);

	/** Helper function to get unique player id from Online Subsystem */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Tools")
	static bool GetUniquePlayerId(APlayerController* PlayerController, FString& PlayerId);

	/** Helper function for easy C++ usage */
	static FString GetUniquePlayerId(APlayerController* PlayerController);

	/** Helper function to get unique local net id */
	static FString GetUniqueNetId();

	/** Check is SteamOnlineSubsystem enabled */
	UFUNCTION(BlueprintPure, Category = "GOSU|Tools")
	static bool IsSteamEnabled();
};
