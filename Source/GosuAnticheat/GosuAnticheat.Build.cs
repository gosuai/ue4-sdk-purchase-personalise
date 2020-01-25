// Copyright 2020 GOSU.AI. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

using UnrealBuildTool;

public class GosuAnticheat : ModuleRules
{
    public GosuAnticheat(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HTTP",
                "Json",
                "JsonUtilities",
                "OnlineSubsystem",
                "GosuPurchases"
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Sockets",
            }
            );

        PublicDefinitions.Add("WITH_GOSU_Anticheat=1");
    }
}
