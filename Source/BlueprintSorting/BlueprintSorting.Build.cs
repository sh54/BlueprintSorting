using UnrealBuildTool;
using System.IO;

public class BlueprintSorting : ModuleRules
{
    public BlueprintSorting(TargetInfo Target)
    {
        PublicIncludePaths.AddRange(
            new string[] {
                "BlueprintSorting/Public",
                // ... add public include paths required here ...
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                "BlueprintSorting/Private",
                // ... add other private include paths required here ...
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                // ... add other public dependencies that you statically link with here ...
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
                // ... add private dependencies that you statically link with here ...
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        bEnforceIWYU = false;
    }
}