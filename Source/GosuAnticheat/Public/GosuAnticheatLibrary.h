// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GosuAnticheatController.h"
#include "GosuAnticheatDataModel.h"

#include "Interfaces/OnlineStoreInterface.h"

#include "GosuAnticheatLibrary.generated.h"

class UGosuPurchasesSettings;

UCLASS()
class GOSUANTICHEAT_API UGosuAnticheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Direct access to Anticheat controller */
	UFUNCTION(BlueprintPure, Category = "GOSU|Common", meta = (WorldContext = "WorldContextObject"))
	static UGosuAnticheatController* GetAnticheatController(const UObject* WorldContextObject);
};
