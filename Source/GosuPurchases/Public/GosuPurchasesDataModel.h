// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuPurchasesDefines.h"

#include "GosuPurchasesDataModel.generated.h"

USTRUCT(BlueprintType)
struct GOSUPURCHASES_API FPurchaseEvent
{
public:
	GENERATED_BODY()

	/** @FIXME Just a test field */
	UPROPERTY(BlueprintReadOnly, Category = "Purchase Data")
	FString UUID;

public:
	FPurchaseEvent(){};
};
