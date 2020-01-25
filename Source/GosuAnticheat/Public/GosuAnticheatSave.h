// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GameFramework/SaveGame.h"

#include "GosuAnticheatDataModel.h"
#include "GosuAnticheatDefines.h"

#include "GosuAnticheatSave.generated.h"

USTRUCT(Blueprintable)
struct GOSUANTICHEAT_API FGosuAnticheatSaveData
{
	GENERATED_USTRUCT_BODY()

	FGosuAnticheatSaveData(){};
};

UCLASS()
class GOSUANTICHEAT_API UGosuAnticheatSave : public USaveGame
{
	GENERATED_BODY()

public:
	static FGosuAnticheatSaveData Load();
	static void Save(const FGosuAnticheatSaveData& InSaveData);

public:
	static const FString SaveSlotName;

	/** User index (always 0) */
	static const int32 UserIndex;

protected:
	UPROPERTY()
	FGosuAnticheatSaveData SaveData;
};
