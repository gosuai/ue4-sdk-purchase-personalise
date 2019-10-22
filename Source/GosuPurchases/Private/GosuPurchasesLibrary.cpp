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

void UGosuPurchasesLibrary::RegisterSession(UObject* WorldContextObject, const FString& PlayerId)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->RegisterSession(PlayerId);
	}
}

void UGosuPurchasesLibrary::CallStoreOpened(UObject* WorldContextObject)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallStoreOpened();
	}
}

void UGosuPurchasesLibrary::CallShowcaseItemShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallShowcaseItemShow(Scenario, StoreCategory, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CallShowcaseItemHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallShowcaseItemHide(Scenario, StoreCategory, ItemSKU);
	}
}

void UGosuPurchasesLibrary::CallItemDetailsShow(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU, const FString& ItemName, float Price, const FString& Currency, const FString& Description)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallItemDetailsShow(Scenario, StoreCategory, ItemSKU, ItemName, Price, Currency, Description);
	}
}

void UGosuPurchasesLibrary::CallItemDetailsHide(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallItemDetailsHide(Scenario, StoreCategory, ItemSKU);
	}
}

void UGosuPurchasesLibrary::CallPurchaseStarted(UObject* WorldContextObject, const FString& ItemSKU)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallPurchaseStarted(ItemSKU);
	}
}

void UGosuPurchasesLibrary::CallPurchaseCompleted(UObject* WorldContextObject, const FString& ItemSKU, EInAppPurchaseState::Type PurchaseState, const FString& TransactionID)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->CallPurchaseCompleted(ItemSKU, PurchaseState, TransactionID);
	}
}

void UGosuPurchasesLibrary::FetchRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FOnReceiveRecommendation& SuccessCallback)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->FetchRecommendations(Scenario, StoreCategory, SuccessCallback);
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
