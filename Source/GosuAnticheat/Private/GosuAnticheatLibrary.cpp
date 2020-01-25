// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuAnticheatLibrary.h"

#include "GosuAnticheat.h"
#include "GosuAnticheatDataModel.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Online.h"
#include "SocketSubsystem.h"

UGosuAnticheatLibrary::UGosuAnticheatLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGosuAnticheatController* UGosuAnticheatLibrary::GetAnticheatController(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FGosuAnticheatModule::Get().GetAnticheatController(World);
	}

	return nullptr;
}
