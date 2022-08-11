using UnrealBuildTool;
using System.IO;

public class BlueprintSorting : ModuleRules
{
    public BlueprintSorting(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "CoreUObject",
                "Engine",
                "KismetCompiler",
                "BlueprintGraph",
                "GraphEditor",
                "UnrealEd",
                "Kismet",
            }
        );
    }
}