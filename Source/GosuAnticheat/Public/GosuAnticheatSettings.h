// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "CoreMinimal.h"

#include "GosuAnticheatSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class GOSUANTICHEAT_API UGosuAnticheatSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Application ID */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "GOSU Anticheat Settings")
	FString AppId;

	/** Credentials for development mode */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "GOSU Anticheat Settings")
	FString SecretKeyDevelopment;

	/** Credentials for public release */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "GOSU Anticheat Settings")
	FString SecretKeyProduction;

	/** Attn.! Check that it's disabled in release */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "GOSU Anticheat Settings")
	bool bDevelopmentMode;
};
