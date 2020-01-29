// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuAnticheatDefines.h"

#include "GosuAnticheatDataModel.generated.h"

UENUM(BlueprintType)
enum class EGosuPlayerStatus : uint8
{
	Default, // Not banned
	Banned   // Banned
};

UENUM(BlueprintType)
enum class EGosuMatchStatus : uint8
{
	Warmup,
	Playing,
	Ended
};

USTRUCT(BlueprintType)
struct GOSUANTICHEAT_API FGosuCustomEvent
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Custom Event")
	FString ParamName;

	UPROPERTY(BlueprintReadWrite, Category = "Custom Event")
	FString ParamValue;

public:
	FGosuCustomEvent(){};
};

USTRUCT(BlueprintType)
struct GOSUANTICHEAT_API FGosuPlayerState
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Player Data")
	FString uid;

	UPROPERTY(BlueprintReadWrite, Category = "Player Data")
	FString team;

	UPROPERTY(BlueprintReadWrite, Category = "Player Data")
	float points;

public:
	FGosuPlayerState()
		: points(0.f){};
};

USTRUCT(BlueprintType)
struct GOSUANTICHEAT_API FGosuTeamState
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Team Data")
	FString team;

	UPROPERTY(BlueprintReadWrite, Category = "Team Data")
	float points;

public:
	FGosuTeamState()
		: points(0.f){};
};
