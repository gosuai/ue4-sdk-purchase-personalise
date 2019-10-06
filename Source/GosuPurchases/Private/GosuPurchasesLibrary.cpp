// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchasesLibrary.h"

#include "GosuPurchases.h"
#include "GosuPurchasesController.h"
#include "GosuPurchasesDataModel.h"

#include "Engine/Engine.h"
#include "Kismet/KismetTextLibrary.h"

UGosuPurchasesLibrary::UGosuPurchasesLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGosuPurchasesController* UGosuPurchasesLibrary::GetPurchasesController(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FGosuPurchasesModule::Get().GetPurchasesController(World);
	}

	return nullptr;
}

UGosuPurchasesSettings* UGosuPurchasesLibrary::GetPurchasesSettings()
{
	return FGosuPurchasesModule::Get().GetSettings();
}
