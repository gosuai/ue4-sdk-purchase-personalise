// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesSave.h"

#include "GosuPurchasesDefines.h"

#include "Kismet/GameplayStatics.h"

const FString UGosuPurchasesSave::SaveSlotName = "GosuPurchasesSaveSlot";
const int32 UGosuPurchasesSave::UserIndex = 0;

FGosuPurchasesSaveData UGosuPurchasesSave::Load()
{
	auto SaveInstance = Cast<UGosuPurchasesSave>(UGameplayStatics::LoadGameFromSlot(UGosuPurchasesSave::SaveSlotName, UGosuPurchasesSave::UserIndex));
	if (!SaveInstance)
	{
		return FGosuPurchasesSaveData();
	}

	return SaveInstance->SaveData;
}

void UGosuPurchasesSave::Save(const FGosuPurchasesSaveData& InSaveData)
{
	auto SaveInstance = Cast<UGosuPurchasesSave>(UGameplayStatics::LoadGameFromSlot(UGosuPurchasesSave::SaveSlotName, UGosuPurchasesSave::UserIndex));
	if (!SaveInstance)
	{
		SaveInstance = Cast<UGosuPurchasesSave>(UGameplayStatics::CreateSaveGameObject(UGosuPurchasesSave::StaticClass()));
	}

	SaveInstance->SaveData = InSaveData;

	UGameplayStatics::SaveGameToSlot(SaveInstance, UGosuPurchasesSave::SaveSlotName, 0);
}
