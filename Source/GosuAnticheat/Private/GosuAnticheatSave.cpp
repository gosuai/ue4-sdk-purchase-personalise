// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuAnticheatSave.h"

#include "GosuAnticheatDefines.h"

#include "Kismet/GameplayStatics.h"

const FString UGosuAnticheatSave::SaveSlotName = "GosuAnticheatSaveSlot";
const int32 UGosuAnticheatSave::UserIndex = 0;

FGosuAnticheatSaveData UGosuAnticheatSave::Load()
{
	auto SaveInstance = Cast<UGosuAnticheatSave>(UGameplayStatics::LoadGameFromSlot(UGosuAnticheatSave::SaveSlotName, UGosuAnticheatSave::UserIndex));
	if (!SaveInstance)
	{
		return FGosuAnticheatSaveData();
	}

	return SaveInstance->SaveData;
}

void UGosuAnticheatSave::Save(const FGosuAnticheatSaveData& InSaveData)
{
	auto SaveInstance = Cast<UGosuAnticheatSave>(UGameplayStatics::LoadGameFromSlot(UGosuAnticheatSave::SaveSlotName, UGosuAnticheatSave::UserIndex));
	if (!SaveInstance)
	{
		SaveInstance = Cast<UGosuAnticheatSave>(UGameplayStatics::CreateSaveGameObject(UGosuAnticheatSave::StaticClass()));
	}

	SaveInstance->SaveData = InSaveData;

	UGameplayStatics::SaveGameToSlot(SaveInstance, UGosuAnticheatSave::SaveSlotName, 0);
}
