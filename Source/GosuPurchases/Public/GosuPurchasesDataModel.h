// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuPurchasesDefines.h"

#include "GosuPurchasesDataModel.generated.h"

UENUM(BlueprintType)
enum class ERecommendationScenario : uint8
{
	/** No scenatio used */
	Default,
	Recommended,
	Highlight,
	Popup
};

USTRUCT(BlueprintType)
struct GOSUPURCHASES_API FGosuItemInfo
{
public:
	GENERATED_BODY()

	/** Item human-readable name */
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	FString name;

	/** Price amount */
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	float price;

	/** Price currency */
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	FString currency;

	/** */
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	FString description;

public:
	FGosuItemInfo()
		: price(0.f){};
};

USTRUCT(BlueprintType)
struct GOSUPURCHASES_API FGosuShowcaseEvent
{
public:
	GENERATED_BODY()

	/** Unique event id */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	FString eventUUID;

	/** Event unix time */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	int64 timestamp;

	/** Scenario that caused event */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	ERecommendationScenario scenario;

	/** Showcase category where event occured */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	FString category;

	/** Item id */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	FString sku;

	/** (optional) Full item info */
	UPROPERTY(BlueprintReadOnly, Category = "Event Data")
	FGosuItemInfo item;

public:
	FGosuShowcaseEvent()
		: timestamp(0)
		, scenario(ERecommendationScenario::Default){};
};

USTRUCT(BlueprintType)
struct GOSUPURCHASES_API FGosuRecommendedItem
{
public:
	GENERATED_BODY()

	/** Item id */
	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	FString sku;

public:
	FGosuRecommendedItem(){};
};

USTRUCT(BlueprintType)
struct GOSUPURCHASES_API FGosuRecommendation
{
public:
	GENERATED_BODY()

	/** Scenario used for recommendation */
	UPROPERTY(BlueprintReadOnly, Category = "Recommendation Data")
	ERecommendationScenario scenario;

	/** Recommented items */
	UPROPERTY(BlueprintReadOnly, Category = "Recommendation Data")
	TArray<FGosuRecommendedItem> items;

public:
	FGosuRecommendation(){};
};
