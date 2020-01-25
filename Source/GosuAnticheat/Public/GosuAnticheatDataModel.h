// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuAnticheatDefines.h"

#include "GosuAnticheatDataModel.generated.h"

USTRUCT(BlueprintType)
struct GOSUANTICHEAT_API FGosuCustomEvent
{
public:
	GENERATED_BODY()

	/** */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	FString data;

public:
	FGosuCustomEvent(){};
};
