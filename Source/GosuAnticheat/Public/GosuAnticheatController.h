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
class FJsonValue;
struct FGuid;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnReceivePlayerStatus, const FString&, PlayerId, EGosuPlayerStatus, PlayerStatus);

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

	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void ServerMatchStateChanged(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& Map, const FString& GameMode, bool IsRanked, const TArray<FGosuPlayerState>& PlayerStates, const TArray<FGosuTeamState>& TeamStates);

	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void ServerPlayerJoin(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating);

	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void ServerPlayerLeave(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating);

	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void SendCustomEvent(const FString& MatchId, EGosuMatchStatus MatchStatus, float MatchTime, const FString& PlayerId, const FString& PlayerNetId, const FString& PlayerNickname, float PlayerRating, const FString& JsonFormattedData);

	UFUNCTION(BlueprintCallable, Category = "GOSU|Controller")
	void CheckUserStatus(const FString& PlayerId, const FString& PlayerNetId, const FOnReceivePlayerStatus& SuccessCallback, const FOnRequestError& ErrorCallback);

	/** Flush custom events */
	void FlushEvents();

public:
	void Generic_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void CheckUserStatus_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnReceivePlayerStatus SuccessCallback, FOnRequestError ErrorCallback);

	/** Return true if error is happened */
	bool HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);

	/** Convert enum to string */
	FString GetMatchStatusAsString(EGosuMatchStatus EnumValue) const;

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

protected:
	static const FString GosuApiEndpoint;

private:
	/** Cached AppId */
	FString AppId;

	/** Cached secret key */
	FString SecretKey;

	/** Cached showcase events to be sent in bundle */
	TArray<TSharedPtr<FJsonValue>> CustomEvents;

	/** Events flush time handler */
	float FlushTimeAccumulator;
};
