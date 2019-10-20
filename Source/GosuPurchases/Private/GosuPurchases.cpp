// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "GosuPurchases.h"

#include "GosuPurchasesController.h"
#include "GosuPurchasesDefines.h"
#include "GosuPurchasesSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Engine/World.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FGosuPurchasesModule"

void FGosuPurchasesModule::StartupModule()
{
	GosuPurchasesSettings = NewObject<UGosuPurchasesSettings>(GetTransientPackage(), "GosuPurchasesSettings", RF_Standalone);
	GosuPurchasesSettings->AddToRoot();

	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "GosuPurchases",
			LOCTEXT("RuntimeSettingsName", "GOSU Purchases"),
			LOCTEXT("RuntimeSettingsDescription", "Configure GOSU Purchases"),
			GosuPurchasesSettings);
	}

	FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld* World, bool bSessionEnded, bool bCleanupResources) {
		GosuPurchasesControllers.Remove(World);

		UE_LOG(LogGosuPurchases, Log, TEXT("%s: GosuPurchases Controller is removed for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

	FWorldDelegates::OnPostWorldInitialization.AddLambda([this](UWorld* World, const UWorld::InitializationValues IVS) {
		auto PurchasesContoller = NewObject<UGosuPurchasesController>(GetTransientPackage());
		PurchasesContoller->SetFlags(RF_Standalone);
		PurchasesContoller->AddToRoot();

		// Initialize controller with default settings
		PurchasesContoller->Initialize(World);

		GosuPurchasesControllers.Add(World, PurchasesContoller);

		UE_LOG(LogGosuPurchases, Log, TEXT("%s: GosuPurchases Controller is created for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

	UE_LOG(LogGosuPurchases, Log, TEXT("%s: GosuPurchases module started"), *VA_FUNC_LINE);
}

void FGosuPurchasesModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "GosuPurchases");
	}

	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		GosuPurchasesSettings->RemoveFromRoot();

		for (auto PurchasesContoller : GosuPurchasesControllers)
		{
			PurchasesContoller.Value->RemoveFromRoot();
		}
	}
	else
	{
		GosuPurchasesSettings = nullptr;
	}

	GosuPurchasesControllers.Empty();
}

UGosuPurchasesSettings* FGosuPurchasesModule::GetSettings() const
{
	check(GosuPurchasesSettings);
	return GosuPurchasesSettings;
}

UGosuPurchasesController* FGosuPurchasesModule::GetPurchasesController(UWorld* World) const
{
	return GosuPurchasesControllers.FindChecked(World);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGosuPurchasesModule, GosuPurchases)

DEFINE_LOG_CATEGORY(LogGosuPurchases);
