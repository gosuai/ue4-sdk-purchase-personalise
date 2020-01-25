// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GosuAnticheatDataModel.h"

#include "GosuPurchasesDataModel.h"

#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "Interfaces/OnlineStoreInterface.h"
#include "Tickable.h"

#include "GosuAnticheatController.generated.h"

class APlayerController;
class FJsonObject;
struct FGuid;

UCLASS()
class GOSUANTICHEAT_API UGosuAnticheatController : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

protected:
	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual TStatId GetStatId() const override { return TStatId(); }
	// FTickableGameObject end

public:
	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void Initialize(const FString& InAppId, const FString& InSecretKey);

	/** Register game session with provided player id */
	void RegisterSession(APlayerController* PlayerController, const FString& PlayerId);

	/** Register game session while using SteamOnlineSubsystem */
	void RegisterSteamSession(APlayerController* PlayerController);

protected:
	/** Sessions is started with PlayerId  */
	void CallRegisterSession(const FString& PlayerId);

public:
	/** Return true if error is happened */
	bool HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);

protected:
	/** Load save game and extract data */
	void LoadData();

	/** Save cached data or reset one if necessary */
	void SaveData();

	/** Check is we're in development mode for now or not */
	bool IsDevelopmentModeEnabled() const;

	/** Create http request and add API meta */
	TSharedRef<IHttpRequest> CreateHttpRequest(const FString& Url, const FString& BodyContent = TEXT(""), ERequestVerb Verb = ERequestVerb::POST);

	/** Serialize json object into string */
	FString SerializeJson(const TSharedPtr<FJsonObject> DataJson) const;

	/** Helper functions to check filled stuff */
	bool CheckUserId() const;

	/** Flush showcase events */
	void FlushEvents();

public:
	/** Cached user id */
	FString UserID;

protected:
	static const FString GosuApiEndpoint;

private:
	/** Cached AppId */
	FString AppId;

	/** Cached secret key */
	FString SecretKey;

	/** Cached showcase events to be sent in bundle */
	TArray<FGosuCustomEvent> ShowcaseEvents;

	/** Events flush time handler */
	float FlushTimeAccumulator;
};
