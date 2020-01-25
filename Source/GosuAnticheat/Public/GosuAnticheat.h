// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UGosuAnticheatSettings;
class UGosuAnticheatController;

class FGosuAnticheatModule : public IModuleInterface
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
	static inline FGosuAnticheatModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FGosuAnticheatModule>("GosuAnticheat");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("GosuAnticheat");
	}

	/** Getter for internal settings object to support runtime configuration changes */
	UGosuAnticheatSettings* GetSettings() const;

	/** Get global data controller */
	UGosuAnticheatController* GetAnticheatController(UWorld* World) const;

private:
	/** Module settings */
	UGosuAnticheatSettings* GosuAnticheatSettings;

	/** Data controllers (one for each World we have) */
	TMap<UWorld*, UGosuAnticheatController*> GosuAnticheatControllers;
};
