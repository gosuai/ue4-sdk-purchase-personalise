// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GameFramework/SaveGame.h"

#include "GosuPurchasesDefines.h"

#include "GosuPurchasesSave.generated.h"

USTRUCT(Blueprintable)
struct GOSUPURCHASES_API FGosuPurchasesSaveData
{
	GENERATED_USTRUCT_BODY()

	FGosuPurchasesSaveData(){};
};

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesSave : public USaveGame
{
	GENERATED_BODY()

public:
	static FGosuPurchasesSaveData Load();
	static void Save(const FGosuPurchasesSaveData& InSaveData);

public:
	static const FString SaveSlotName;

	/** User index (always 0) */
	static const int32 UserIndex;

protected:
	UPROPERTY()
	FGosuPurchasesSaveData SaveData;
};
