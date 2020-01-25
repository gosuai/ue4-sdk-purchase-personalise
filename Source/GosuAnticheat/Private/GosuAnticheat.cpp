// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuAnticheat.h"

#include "GosuAnticheatController.h"
#include "GosuAnticheatDefines.h"

#include "GosuPurchases.h"
#include "GosuPurchasesSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Engine/World.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FGosuAnticheatModule"

void FGosuAnticheatModule::StartupModule()
{
	FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld* World, bool bSessionEnded, bool bCleanupResources) {
		GosuAnticheatControllers.Remove(World);

		UE_LOG(LogGosuAnticheat, Log, TEXT("%s: GosuAnticheat Controller is removed for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

	FWorldDelegates::OnPostWorldInitialization.AddLambda([this](UWorld* World, const UWorld::InitializationValues IVS) {
		auto AnticheatContoller = NewObject<UGosuAnticheatController>(GetTransientPackage());
		AnticheatContoller->SetFlags(RF_Standalone);
		AnticheatContoller->AddToRoot();

		// Initialize controller with default settings
		const UGosuPurchasesSettings* Settings = FGosuPurchasesModule::Get().GetSettings();
		AnticheatContoller->Initialize(Settings->AppId, Settings->bDevelopmentMode ? Settings->SecretKeyDevelopment : Settings->SecretKeyProduction);

		GosuAnticheatControllers.Add(World, AnticheatContoller);

		UE_LOG(LogGosuAnticheat, Log, TEXT("%s: GosuAnticheat Controller is created for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

	UE_LOG(LogGosuAnticheat, Log, TEXT("%s: GosuAnticheat module started"), *VA_FUNC_LINE);
}

void FGosuAnticheatModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "GosuAnticheat");
	}

	if (!GExitPurge)
	{
		for (auto AnticheatContoller : GosuAnticheatControllers)
		{
			AnticheatContoller.Value->RemoveFromRoot();
		}
	}

	GosuAnticheatControllers.Empty();
}

UGosuAnticheatController* FGosuAnticheatModule::GetAnticheatController(UWorld* World) const
{
	return GosuAnticheatControllers.FindChecked(World);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGosuAnticheatModule, GosuAnticheat)

DEFINE_LOG_CATEGORY(LogGosuAnticheat);
