// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesLibrary.h"

#include "GosuPurchases.h"
#include "GosuPurchasesDataModel.h"
#include "GosuPurchasesSettings.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetTextLibrary.h"
#include "Online.h"
#include "SocketSubsystem.h"

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

void UGosuPurchasesLibrary::RegisterSession(UObject* WorldContextObject, APlayerController* PlayerController, const FString& PlayerId)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->RegisterSession(PlayerController, PlayerId);
	}
}

void UGosuPurchasesLibrary::RegisterSteamSession(UObject* WorldContextObject, APlayerController* PlayerController)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->RegisterSteamSession(PlayerController);
	}
}

void UGosuPurchasesLibrary::RegisterStoreOpened(UObject* WorldContextObject)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->RegisterStoreOpened();
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

void UGosuPurchasesLibrary::FetchRecommendations(UObject* WorldContextObject, ERecommendationScenario Scenario, const FString& StoreCategory, const FOnReceiveRecommendation& SuccessCallback, int32 MaxItems)
{
	if (auto PurchasesController = UGosuPurchasesLibrary::GetPurchasesController(WorldContextObject))
	{
		PurchasesController->FetchRecommendations(Scenario, StoreCategory, SuccessCallback, MaxItems);
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

bool UGosuPurchasesLibrary::GetControllerNetworkID(APlayerController* PlayerController, FString& NetworkID, bool bKeepPort)
{
	if (!PlayerController || !PlayerController->PlayerState)
	{
		return false;
	}

	NetworkID = PlayerController->PlayerState->UniqueId->ToString();

	if (!bKeepPort)
	{
		FString EmptyString;
		NetworkID.Split(FString(":"), &NetworkID, &EmptyString, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	}

	return true;
}

bool UGosuPurchasesLibrary::GetUniquePlayerId(APlayerController* PlayerController, FString& PlayerId)
{
	if (PlayerController)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		TSharedPtr<const FUniqueNetId> UniquePlayerId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(PlayerController->GetLocalPlayer()->GetControllerId());

		if (UniquePlayerId->IsValid())
		{
			PlayerId = UniquePlayerId->ToString();
			return true;
		}
		else
		{
			UE_LOG(LogGosuPurchases, Error, TEXT("%s: Invalid UniquePlayerId"), *VA_FUNC_LINE);
		}
	}
	else
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Invalid PlayerController"), *VA_FUNC_LINE);
	}

	PlayerId = FString();
	return false;
}

FString UGosuPurchasesLibrary::GetUniquePlayerId(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		TSharedPtr<const FUniqueNetId> UniquePlayerId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(PlayerController->GetLocalPlayer()->GetControllerId());

		if (UniquePlayerId->IsValid())
		{
			return UniquePlayerId->ToString();
		}
		else
		{
			UE_LOG(LogGosuPurchases, Error, TEXT("%s: Invalid UniquePlayerId"), *VA_FUNC_LINE);
		}
	}
	else
	{
		UE_LOG(LogGosuPurchases, Error, TEXT("%s: Invalid PlayerController"), *VA_FUNC_LINE);
	}

	return FString();
}

FString UGosuPurchasesLibrary::GetUniqueNetId()
{
	FString HostName;
	if (!ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostName(HostName))
	{
		bool bCanBindAll;
		TSharedPtr<class FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
		HostName = Addr->ToString(false);
	}

	return FString::Printf(TEXT("%s-%s"), *HostName, *FPlatformMisc::GetLoginId().ToUpper());
}

bool UGosuPurchasesLibrary::IsSteamEnabled()
{
	return IOnlineSubsystem::IsEnabled(STEAM_SUBSYSTEM);
}
