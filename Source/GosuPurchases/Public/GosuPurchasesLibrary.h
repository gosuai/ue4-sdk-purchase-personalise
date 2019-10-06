// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GosuPurchasesDataModel.h"

#include "GosuPurchasesLibrary.generated.h"

class UGosuPurchasesController;

UCLASS()
class GOSUPURCHASES_API UGosuPurchasesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Direct access to purchases controller */
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases", meta = (WorldContext = "WorldContextObject"))
	static UGosuPurchasesController* GetPurchasesController(UObject* WorldContextObject);

	/** Direct access to purchases settings */
	UFUNCTION(BlueprintPure, Category = "GOSU|Purchases")
	static UGosuPurchasesSettings* GetPurchasesSettings();
};
