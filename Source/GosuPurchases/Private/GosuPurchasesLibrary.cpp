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

void UGosuPurchasesLibrary::CollectShowcaseItemShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectShowcaseItemShow(Scenario, Category, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CollectShowcaseItemHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectShowcaseItemHide(Scenario, Category, ItemSKU);
	}
}

void UGosuPurchasesLibrary::CollectItemDetailsShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectItemDetailsShow(Scenario, Category, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CollectItemDetailsHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& Category, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CollectItemDetailsHide(Scenario, Category, ItemSKU);
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

void UGosuPurchasesLibrary::GetRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& Category, const FOnReceiveRecommendation& SuccessCallback)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->GetRecommendations(Scenario, Category, SuccessCallback);
	}
}
