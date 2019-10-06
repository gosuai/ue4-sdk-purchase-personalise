// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuPurchasesDataModel.h"

#include "Delegates/DelegateCombinations.h"
#include "Http.h"

#include "GosuPurchasesController.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnRequestError, int32, StatusCode, int32, ErrorCode, const FString&, ErrorMessage);

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesController : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Purchases")
	void Initialize();

protected:
	/** Return true if error is happened */
	bool HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);

protected:
	/** Load save game and extract data */
	void LoadData();

	/** Save cached data or reset one if necessary */
	void SaveData();

	/** Check is we're in development mode for now or not */
	bool IsDevelopmentModeEnabled() const;

private:
	/** Create http request and add API meta */
	TSharedRef<IHttpRequest> CreateHttpRequest(const FString& Url);
};
