// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesLibrary.h"

#include "GosuPurchases.h"
#include "GosuPurchasesDataModel.h"
#include "GosuPurchasesSettings.h"

#include "Engine/Engine.h"
#include "Kismet/KismetTextLibrary.h"

UGosuPurchasesLibrary::UGosuPurchasesLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGosuPurchasesController* UGosuPurchasesLibrary::GetPurchasesController(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FGosuPurchasesModule::Get().GetPurchasesController(World);
	}

	return nullptr;
}

UGosuPurchasesSettings* UGosuPurchasesLibrary::GetPurchasesSettings(UObject* WorldContextObject)
{
	return FGosuPurchasesModule::Get().GetSettings();
}

void UGosuPurchasesLibrary::CollectSession(UObject* WorldContextObject, const FString& PlayerId)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectSession(PlayerId);
	}
}

void UGosuPurchasesLibrary::CollectStoreOpened(UObject* WorldContextObject)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectStoreOpened();
	}
}

void UGosuPurchasesLibrary::CollectShowcaseItemShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectShowcaseItemShow(Scenario, StoreCategory, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CollectShowcaseItemHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectShowcaseItemHide(Scenario, StoreCategory, ItemSKU);
	}
}

void UGosuPurchasesLibrary::CollectItemDetailsShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectItemDetailsShow(Scenario, StoreCategory, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CollectItemDetailsHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectItemDetailsHide(Scenario, StoreCategory, ItemSKU);
	}
}

void UGosuPurchasesLibrary::CollectPurchaseStarted(UObject* WorldContextObject, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectPurchaseStarted(ItemSKU);
	}
}

void UGosuPurchasesLibrary::CollectPurchaseCompleted(UObject* WorldContextObject, const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectPurchaseCompleted(ItemSKU, PurchaseState, TransactionID);
	}
}

void UGosuPurchasesLibrary::ReceiveRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FOnReceiveRecommendation& SuccessCallback)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->ReceiveRecommendations(Scenario, StoreCategory, SuccessCallback);
	}
}

TArray<FGosuRecommendedItem> UGosuPurchasesLibrary::GetRecommendedItems(UObject* WorldContextObject, ERecommendationScenario Scenario)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		return PurchasesController->GetRecommendedItems(Scenario);
	}

	return TArray<FGosuRecommendedItem>();
}
