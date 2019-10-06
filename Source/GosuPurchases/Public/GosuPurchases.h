// Copyright 2019 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UGosuPurchasesSettings;
class UGosuPurchasesController;

class FGosuPurchasesModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FGosuPurchasesModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FGosuPurchasesModule>("GosuPurchases");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("GosuPurchases");
	}

	/** Getter for internal settings object to support runtime configuration changes */
	UGosuPurchasesSettings* GetSettings() const;

	/** Get global data controller */
	UGosuPurchasesController* GetPurchasesController(UWorld* World) const;

private:
	/** Module settings */
	UGosuPurchasesSettings* GosuPurchasesSettings;

	/** Data controllers (one for each World we have) */
	TMap<UWorld*, UGosuPurchasesController*> GosuPurchasesControllers;
};
