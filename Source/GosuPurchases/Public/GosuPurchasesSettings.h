// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Blueprint/UserWidget.h"

#include "GosuPurchasesSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class GOSUPURCHASES_API UGosuPurchasesSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Credentials for development mode */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "GOSU Purchases Settings")
	FString SecretKeyDevelopment;

	/** Credentials for public release */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "GOSU Purchases Settings")
	FString SecretKeyRelease;

	/** Attn.! Check that it's disabled in release */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "GOSU Purchases Settings")
	bool bDevelopmentMode;
};
