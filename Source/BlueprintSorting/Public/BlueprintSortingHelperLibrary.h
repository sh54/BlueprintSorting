#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Object.h"

#include "BlueprintSortingHelperLibrary.generated.h"

/**
 * Various helpers
 */
UCLASS()
class BLUEPRINTSORTING_API UBlueprintSortingHelperLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION()
    static int32 Increment(int32 ToIncrement) { return ToIncrement + 1; };

    UFUNCTION(CustomThunk)
    static void SwapItems(const TArray<int32>& TargetArray, int32& Index1, int32& Index2);
    static void SwapItems_Impl(void* TargetArray, const FArrayProperty* ArrayProp, int32 Index1, int32 Index2);

    UFUNCTION(CustomThunk)
    static bool GreaterThanOrEqualArrayLength(const TArray<int32>& TargetArray, int32& Index);
    static bool GreaterThanOrEqualArrayLength_Impl(void* TargetArray, const FArrayProperty* ArrayProp, int32 Index);

    UFUNCTION(CustomThunk)
    static bool LessThanOrEqualArrayLength(const TArray<int32>& TargetArray, int32& Index);
    static bool LessThanOrEqualArrayLength_Impl(void* TargetArray, const FArrayProperty* ArrayProp, int32 Index);

    UFUNCTION(CustomThunk)
    static void TestItem(int32& TargetItem);

    UFUNCTION(CustomThunk)
    static void TestTwoItems(int32& TargetItem1, int32& TargetItem2);

    UFUNCTION(CustomThunk)
    static void TestThreeItems(int32& TargetItem1, int32& TargetItem2, int32& TargetItem3);

    DECLARE_FUNCTION(execSwapItems)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FArrayProperty>(NULL);
        void* ArrayAddr = Stack.MostRecentPropertyAddress;
        FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
        if (!ArrayProperty)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_GET_PROPERTY(FIntProperty, Index1);
        P_GET_PROPERTY(FIntProperty, Index2);

        P_FINISH;
        P_NATIVE_BEGIN;
            SwapItems_Impl(ArrayAddr, ArrayProperty, Index1, Index2);
        P_NATIVE_END;
    }

    DECLARE_FUNCTION(execGreaterThanOrEqualArrayLength)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FArrayProperty>(NULL);
        void* ArrayAddr = Stack.MostRecentPropertyAddress;
        FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
        if (!ArrayProperty)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_GET_PROPERTY(FIntProperty, Index);

        P_FINISH;
        P_NATIVE_BEGIN;
            *(bool*)RESULT_PARAM = GreaterThanOrEqualArrayLength_Impl(ArrayAddr, ArrayProperty, Index);
        P_NATIVE_END;
    }

    DECLARE_FUNCTION(execLessThanOrEqualArrayLength)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FArrayProperty>(NULL);
        void* ArrayAddr = Stack.MostRecentPropertyAddress;
        FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
        if (!ArrayProperty)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_GET_PROPERTY(FIntProperty, Index);

        P_FINISH;
        P_NATIVE_BEGIN;
            *(bool*)RESULT_PARAM = LessThanOrEqualArrayLength_Impl(ArrayAddr, ArrayProperty, Index);
        P_NATIVE_END;
    }

    DECLARE_FUNCTION(execTestItem)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property1Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property1 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property1)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_FINISH;
        P_NATIVE_BEGIN;
            FString OutText;

            FObjectProperty* ObjProp = CastField<FObjectProperty>(Property1);
            if (ObjProp)
            {
                auto Obj = ObjProp->GetObjectPropertyValue(Property1Addr);
                int32 somethign = 932854;
            }

            FIntProperty* IntProp = CastField<FIntProperty>(Property1);
            if (IntProp)
            {
                auto Int = IntProp->GetSignedIntPropertyValue(Property1Addr);
                int32 somethign = 932854;
            }

            Property1->ExportTextItem(OutText, Property1Addr, nullptr, nullptr, 0);
            int32 testing = 1;
            // TODO
            //SortStuff_Redirect(ArrayAddr, ArrayProperty, DelegateAddr, DelegateProperty);
        P_NATIVE_END;
    }

    DECLARE_FUNCTION(execTestTwoItems)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property1Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property1 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property1)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property2Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property2 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property2)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_FINISH;
        P_NATIVE_BEGIN;
            FString OutText;

            {
                FObjectProperty* ObjProp = CastField<FObjectProperty>(Property1);
                if (ObjProp)
                {
                    auto Obj = ObjProp->GetObjectPropertyValue(Property1Addr);
                    int32 somethign = 932854;
                }

                FIntProperty* IntProp = CastField<FIntProperty>(Property1);
                if (IntProp)
                {
                    auto Int = IntProp->GetSignedIntPropertyValue(Property1Addr);
                    int32 somethign = 932854;
                }
            }

            {
                FObjectProperty* ObjProp = CastField<FObjectProperty>(Property2);
                if (ObjProp)
                {
                    auto Obj = ObjProp->GetObjectPropertyValue(Property2Addr);
                    int32 somethign = 932854;
                }

                FIntProperty* IntProp = CastField<FIntProperty>(Property2);
                if (IntProp)
                {
                    auto Int = IntProp->GetSignedIntPropertyValue(Property2Addr);
                    int32 somethign = 932854;
                }
            }

            int32 testing = 1;
            // TODO
            //SortStuff_Redirect(ArrayAddr, ArrayProperty, DelegateAddr, DelegateProperty);
        P_NATIVE_END;
    }

    DECLARE_FUNCTION(execTestThreeItems)
    {
        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property1Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property1 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property1)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property2Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property2 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property2)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        Stack.MostRecentProperty = nullptr;
        Stack.StepCompiledIn<FProperty>(NULL);
        void* Property3Addr = Stack.MostRecentPropertyAddress;
        FProperty* Property3 = CastField<FProperty>(Stack.MostRecentProperty);
        if (!Property3)
        {
            Stack.bArrayContextFailed = true;
            return;
        }

        P_FINISH;
        P_NATIVE_BEGIN;
            FString OutText;

            {
                FObjectProperty* ObjProp = CastField<FObjectProperty>(Property1);
                if (ObjProp)
                {
                    auto Obj = ObjProp->GetObjectPropertyValue(Property1Addr);
                    int32 somethign = 932854;
                }

                FIntProperty* IntProp = CastField<FIntProperty>(Property1);
                if (IntProp)
                {
                    auto Int = IntProp->GetSignedIntPropertyValue(Property1Addr);
                    int32 somethign = 932854;
                }
            }

            {
                FObjectProperty* ObjProp = CastField<FObjectProperty>(Property2);
                if (ObjProp)
                {
                    auto Obj = ObjProp->GetObjectPropertyValue(Property2Addr);
                    int32 somethign = 932854;
                }

                FIntProperty* IntProp = CastField<FIntProperty>(Property2);
                if (IntProp)
                {
                    auto Int = IntProp->GetSignedIntPropertyValue(Property2Addr);
                    int32 somethign = 932854;
                }
            }

            {
                FObjectProperty* ObjProp = CastField<FObjectProperty>(Property3);
                if (ObjProp)
                {
                    auto Obj = ObjProp->GetObjectPropertyValue(Property3Addr);
                    int32 somethign = 932854;
                }

                FIntProperty* IntProp = CastField<FIntProperty>(Property3);
                if (IntProp)
                {
                    auto Int = IntProp->GetSignedIntPropertyValue(Property3Addr);
                    int32 somethign = 932854;
                }
            }

            int32 testing = 1;
            // TODO
            //SortStuff_Redirect(ArrayAddr, ArrayProperty, DelegateAddr, DelegateProperty);
        P_NATIVE_END;
    }
};
