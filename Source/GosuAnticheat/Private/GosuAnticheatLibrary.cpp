// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuAnticheatLibrary.h"

#include "GosuAnticheat.h"
#include "GosuAnticheatDataModel.h"
#include "GosuAnticheatSettings.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Online.h"
#include "SocketSubsystem.h"

UGosuAnticheatLibrary::UGosuAnticheatLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGosuAnticheatController* UGosuAnticheatLibrary::GetAnticheatController(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FGosuAnticheatModule::Get().GetAnticheatController(World);
	}

	return nullptr;
}

UGosuAnticheatSettings* UGosuAnticheatLibrary::GetAnticheatSettings(UObject* WorldContextObject)
{
	return FGosuAnticheatModule::Get().GetSettings();
}

void UGosuAnticheatLibrary::RegisterSession(UObject* WorldContextObject, APlayerController* PlayerController, const FString& PlayerId)
{
	if (auto AnticheatController = UGosuAnticheatLibrary::GetAnticheatController(WorldContextObject))
	{
		AnticheatController->RegisterSession(PlayerController, PlayerId);
	}
}

void UGosuAnticheatLibrary::RegisterSteamSession(UObject* WorldContextObject, APlayerController* PlayerController)
{
	if (auto AnticheatController = UGosuAnticheatLibrary::GetAnticheatController(WorldContextObject))
	{
		AnticheatController->RegisterSteamSession(PlayerController);
	}
}
