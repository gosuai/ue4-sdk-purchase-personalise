// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GosuAnticheatController.h"
#include "GosuAnticheatDataModel.h"

#include "Interfaces/OnlineStoreInterface.h"

#include "GosuAnticheatLibrary.generated.h"

class UGosuAnticheatSettings;

UCLASS()
class GOSUANTICHEAT_API UGosuAnticheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Direct access to Anticheat controller */
	UFUNCTION(BlueprintPure, Category = "GOSU|Anticheat", meta = (WorldContext = "WorldContextObject"))
	static UGosuAnticheatController* GetAnticheatController(UObject* WorldContextObject);

	/** Direct access to Anticheat settings */
	UFUNCTION(BlueprintPure, Category = "GOSU|Anticheat", meta = (WorldContext = "WorldContextObject"))
	static UGosuAnticheatSettings* GetAnticheatSettings(UObject* WorldContextObject);

	/** Register game session with provided player id */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Anticheat|Events", meta = (WorldContext = "WorldContextObject"))
	static void RegisterSession(UObject* WorldContextObject, APlayerController* PlayerController, const FString& PlayerId);

	/** Register game session while using SteamOnlineSubsystem */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Anticheat|Events", meta = (WorldContext = "WorldContextObject"))
	static void RegisterSteamSession(UObject* WorldContextObject, APlayerController* PlayerController);
};
