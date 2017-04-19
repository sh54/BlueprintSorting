#include "CoreMinimal.h"

#include "BlueprintSortingHelperLibrary.h"

void UBlueprintSortingHelperLibrary::SwapItems_Impl(void* TargetArray, const UArrayProperty* ArrayProp, int32 Index1, int32 Index2)
{
    if (TargetArray)
    {
        FScriptArrayHelper ArrayHelper(ArrayProp, TargetArray);

        ArrayHelper.SwapValues(Index1, Index2);
    }
}

bool UBlueprintSortingHelperLibrary::GreaterThanOrEqualArrayLength_Impl(void* TargetArray, const UArrayProperty* ArrayProp, int32 Index)
{
    if (TargetArray)
    {
        FScriptArrayHelper ArrayHelper(ArrayProp, TargetArray);

        auto Count = ArrayHelper.Num();
        return Index >= Count;
    }

    return false;
}

bool UBlueprintSortingHelperLibrary::LessThanOrEqualArrayLength_Impl(void* TargetArray, const UArrayProperty* ArrayProp, int32 Index)
{
    if (TargetArray)
    {
        FScriptArrayHelper ArrayHelper(ArrayProp, TargetArray);

        auto Count = ArrayHelper.Num();
        return Index <= Count;
    }

    return false;
}
