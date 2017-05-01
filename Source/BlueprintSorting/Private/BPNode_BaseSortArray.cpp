#include "CoreMinimal.h"
#include "BPNode_BaseSortArray.h"
#include "BPCHandler_BubbleSortArray.h"
#include "Engine/Blueprint.h"
#include "Engine/MemberReference.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraphUtilities.h"
#include "EditorCategoryUtils.h"
#include "BlueprintNodeSpawner.h"
#include "OrderingResult.h"

#define LOCTEXT_NAMESPACE "BlueprintSorting"

struct FBPNode_BaseSortArrayHelper
{
    static FString DelegatePinName;
    static FString ArrayPinName;
    static FString SortDirectionPinName;
};
FString FBPNode_BaseSortArrayHelper::DelegatePinName(TEXT("Delegate"));
FString FBPNode_BaseSortArrayHelper::ArrayPinName(TEXT("Array"));
FString FBPNode_BaseSortArrayHelper::SortDirectionPinName(TEXT("Sort Direction"));

UBPNode_BaseSortArray::UBPNode_BaseSortArray(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    int32 Test = 1;
}



void UBPNode_BaseSortArray::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    auto DelegatePin = this->GetDelegatePin();
    auto BlueprintClass = this->GetBlueprintClassFromNode();

    if(DelegatePin)
    {
        auto Ref = FMemberReference::ResolveSimpleMemberReference<UFunction>(DelegatePin->PinType.PinSubCategoryMemberReference);
        // TODO: determine if this is needed or not...
        //DelegatePin->PinType.PinSubCategoryMemberReference.MemberParent = BlueprintClass;
    }

    auto ArrayPin = this->GetTargetArrayPin();

    if(ArrayPin->PinType.PinCategory == K2Schema->PC_Wildcard)
    {
        MessageLog.Error(*FString::Printf(*NSLOCTEXT("K2Node", "BaseSortArrayNotAssignable", "Array input must have a type. Connect it to something. @@").ToString()), this);
    }
    else
    {
        auto Signature = GetSignatureFunction(ArrayPin->PinType);
        if(!Signature)
        {
            FString TypeToSort;
            
            if(ArrayPin->PinType.PinCategory == K2Schema->PC_Object || ArrayPin->PinType.PinCategory == K2Schema->PC_Interface || ArrayPin->PinType.PinCategory == K2Schema->PC_Byte || ArrayPin->PinType.PinCategory == K2Schema->PC_Struct)
            {
                auto PinSubCategoryObject = ArrayPin->PinType.PinSubCategoryObject.Get();
                if(PinSubCategoryObject)
                {
                    TypeToSort = PinSubCategoryObject->GetName();
                }
                else
                {
                    TypeToSort = ArrayPin->PinType.PinCategory;
                }
            }
            else
            {
                TypeToSort = ArrayPin->PinType.PinCategory;
            }
            MessageLog.Error(*FString::Printf(*NSLOCTEXT("K2Node", "BaseSortArray_MissingSignature", "Signature is missing. Create a (dummy) event dispatcher with a signature of %s, %s, OrderingResult. @@").ToString(), *TypeToSort, *TypeToSort), this);
        }
    }

    Super::ValidateNodeDuringCompilation(MessageLog);
}

bool UBPNode_BaseSortArray::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
    UEdGraphSchema const* Schema = TargetGraph->GetSchema();
    EGraphType GraphType = Schema->GetGraphType(TargetGraph);

    bool const bIsCompatible = (GraphType == GT_Ubergraph) || (GraphType == GT_Function);
    return bIsCompatible&& Super::IsCompatibleWithGraph(TargetGraph);
}

UK2Node::ERedirectType UBPNode_BaseSortArray::DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const
{
    ERedirectType OrginalResult = Super::DoPinsMatchForReconstruction(NewPin, NewPinIndex, OldPin, OldPinIndex);
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
    if ((ERedirectType::ERedirectType_None == OrginalResult) && K2Schema && NewPin && OldPin)
    {
        if ((OldPin->PinType.PinCategory == K2Schema->PC_Delegate) &&
            (NewPin->PinType.PinCategory == K2Schema->PC_Delegate) &&
            (FCString::Stricmp(*(NewPin->PinName), *(OldPin->PinName)) == 0))
        {
            return ERedirectType_Name;
        }
    }
    return OrginalResult;
}

void UBPNode_BaseSortArray::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();

    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    CreatePin(EGPD_Input, K2Schema->PC_Exec, TEXT(""), NULL, false, false, K2Schema->PN_Execute);
    CreatePin(EGPD_Output, K2Schema->PC_Exec, TEXT(""), NULL, false, false, K2Schema->PN_Then);

    UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ESortDirection"), true);
    CreatePin(EGPD_Input, K2Schema->PC_Byte, TEXT(""), EnumPtr, false, false, FBPNode_BaseSortArrayHelper::SortDirectionPinName);
    GetSortDirectionPin()->DefaultValue = "Ascending";

    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, TEXT(""), NULL, true, false, FBPNode_BaseSortArrayHelper::ArrayPinName);
}

void UBPNode_BaseSortArray::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
    AllocateDefaultPins();

    UEdGraphPin* ArrayPin = nullptr;
    for (int32 i = 0; i < OldPins.Num(); i++)
    {
        UEdGraphPin* Pin = OldPins[i];
        if (Pin->PinName == FBPNode_BaseSortArrayHelper::ArrayPinName)
        {
            ArrayPin = Pin;
            break;
        }
    }

    if(ArrayPin)
    {
        if(ArrayPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
        {
            const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

            auto DelegatePin = CreatePin(EGPD_Input, K2Schema->PC_Delegate, TEXT(""), NULL, false, true, FBPNode_BaseSortArrayHelper::DelegatePinName, true);
            if (DelegatePin)
            {
                UFunction* SignatureFunction = GetSignatureFunction(ArrayPin->PinType);

                FMemberReference::FillSimpleMemberReference<UFunction>(SignatureFunction, DelegatePin->PinType.PinSubCategoryMemberReference);
                DelegatePin->PinFriendlyName = NSLOCTEXT("K2Node", "PinFriendlyDelegatetName", "Ordering Event");
            }
        }
    }

    RestoreSplitPins(OldPins);
}

void UBPNode_BaseSortArray::PostReconstructNode()
{
    if (GetTargetArrayPin()->LinkedTo.Num() > 0)
    {
        OnArrayPinChanged();
    }
}

// TODO: use NotifyPinConnectionListChanged instead?
void UBPNode_BaseSortArray::PinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::PinConnectionListChanged(Pin);
    if (Pin && (Pin->PinName == FBPNode_BaseSortArrayHelper::ArrayPinName))
    {
        OnArrayPinChanged();
    } 
}

UEdGraphPin* UBPNode_BaseSortArray::GetThenPin() const
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
    return FindPin(K2Schema->PN_Then, EGPD_Output);
}

UEdGraphPin* UBPNode_BaseSortArray::GetExecutePin() const
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
    return FindPin(K2Schema->PN_Execute, EGPD_Input);
}

UEdGraphPin* UBPNode_BaseSortArray::GetTargetArrayPin() const
{
    return FindPin(FBPNode_BaseSortArrayHelper::ArrayPinName, EGPD_Input);
}

UEdGraphPin* UBPNode_BaseSortArray::GetSortDirectionPin() const
{
    return FindPin(FBPNode_BaseSortArrayHelper::SortDirectionPinName, EGPD_Input);
}

UFunction* UBPNode_BaseSortArray::GetSignatureFunction(FEdGraphPinType& PinType, TSubclassOf<class UObject> ClassToSearch) const
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    if (PinType.PinCategory == K2Schema->PC_Wildcard)
    {
        return nullptr;
    }

    for (TFieldIterator<UMulticastDelegateProperty> It(ClassToSearch); It; ++It)
    {
        UMulticastDelegateProperty* DelProp = *It;
        if(!DelProp)
        {
            continue;
        }

        UFunction* Func = It->SignatureFunction;
        if (!Func)
        {
            continue;
        }

        if (Func->NumParms != 3)
        {
            continue;
        }

        bool Relevant = true;

        int32 PropertyIndex = 0;
        for (TFieldIterator<UProperty> PIt(Func); PIt; ++PIt, ++PropertyIndex)
        {
            if (PIt->PropertyFlags & CPF_OutParm)
            {
                Relevant = false;
                break;
            }

            if (PIt->PropertyFlags & CPF_ReturnParm)
            {
                Relevant = false;
                break;
            }

            FEdGraphPinType PropType;
            K2Schema->ConvertPropertyToPinType(*PIt, PropType);

            if (PropertyIndex < 2)
            {
                if (PropType.bIsArray || PropType.bIsMap || PropType.bIsSet)
                {
                    Relevant = false;
                    break;
                }

                if (PinType.PinCategory != PropType.PinCategory)
                {
                    Relevant = false;
                    break;
                }

                if (PinType.PinSubCategory != PropType.PinSubCategory)
                {
                    Relevant = false;
                    break;
                }

                if (PinType.PinSubCategoryObject != PropType.PinSubCategoryObject)
                {
                    Relevant = false;
                    break;
                }
            }

            if (PropertyIndex == 2)
            {
                if (PropType.bIsArray || PropType.bIsMap || PropType.bIsSet)
                {
                    Relevant = false;
                    break;
                }

                if (PropType.PinCategory != K2Schema->PC_Object)
                {
                    Relevant = false;
                    break;
                }

                auto SubCategoryClass = Cast<UClass>(PropType.PinSubCategoryObject.Get());
                if (!SubCategoryClass)
                {
                    Relevant = false;
                    break;
                }

                auto ClassName = SubCategoryClass->GetName();
                auto OrderingResultClass = UOrderingResult::StaticClass();
                auto OrderingResultClassName = OrderingResultClass->GetName();

                if (ClassName != OrderingResultClassName)
                {
                    Relevant = false;
                    break;
                }

                // TODO: check that it is return?
            }

            if (PropertyIndex > 2)
            {
                Relevant = false;
                break;
            }
        }

        if (Relevant)
        {
            return DelProp->SignatureFunction;
        }
    }

    return nullptr;
}

UFunction* UBPNode_BaseSortArray::GetSignatureFunction(FEdGraphPinType& PinType) const  // , bool bForceNotFromSkelClass
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    if (PinType.PinCategory == K2Schema->PC_Wildcard)
    {
        return nullptr;
    }

    UBlueprint* Blueprint = GetBlueprint();

    // Try to find the signature function in the current blueprint
    UFunction* Sig = GetSignatureFunction(PinType, Blueprint->SkeletonGeneratedClass);

    if(Sig)
    {
        return Sig;
    }

    // Otherwise try to find the signature function in any class! It does not matter where the signature is.
    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (It->IsChildOf(UObject::StaticClass()))
        {
            Sig = GetSignatureFunction(PinType, *It);
            if(Sig)
            {
                return Sig;
            }
        }
    }

    return nullptr;
}

UFunction* UBPNode_BaseSortArray::GetFunction(FEdGraphPinType& PinType) const
// , bool bForceNotFromSkelClass
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    UBlueprint* Blueprint = GetBlueprint();

    //Blueprint->SkeletonGeneratedClass->GetAuthoritativeClass()

    // Search by comparing FNames (INTs), not strings
    for (TFieldIterator<UFunction> It(Blueprint->SkeletonGeneratedClass); It; ++It)
    {
        if (It->NumParms != 3)
        {
            continue;
        }
        bool Relevant = true;

        int32 PropertyIndex = 0;
        for (TFieldIterator<UProperty> PIt(*It); PIt; ++PIt, ++PropertyIndex)
        {
            FEdGraphPinType PropType;
            K2Schema->ConvertPropertyToPinType(*PIt, PropType);

            if (PropertyIndex < 2)
            {
                if (PropType.bIsArray || PropType.bIsMap || PropType.bIsSet)
                {
                    Relevant = false;
                    break;
                }
                if (PinType.PinCategory != PropType.PinCategory)
                {
                    Relevant = false;
                    break;
                }
                if (PinType.PinSubCategory != PropType.PinSubCategory)
                {
                    Relevant = false;
                    break;
                }
                if (PinType.PinSubCategoryObject != PropType.PinSubCategoryObject)
                {
                    Relevant = false;
                    break;
                }
            }

            if (PropertyIndex == 2)
            {
                if (PropType.bIsArray || PropType.bIsMap || PropType.bIsSet)
                {
                    Relevant = false;
                    break;
                }
                if (PropType.PinCategory != "int")
                {
                    Relevant = false;
                    break;
                }
            }

            if (PropertyIndex > 2)
            {
                Relevant = false;
                break;
            }
        }

        if (Relevant)
        { 
            return *It;
        }
    }

    return nullptr;
}

bool UBPNode_BaseSortArray::AreSameSignatures(const UEdGraphSchema_K2* K2Schema, UFunction* SigA, UFunction* SigB)
{
    if(SigA->NumParms != SigB->NumParms)
    {
        return false;
    }

    for (TFieldIterator<UProperty> ItA(SigA), ItB(SigB); ItA, ItB; ++ItA, ++ItB)
    {
        FEdGraphPinType PropTypeA;
        FEdGraphPinType PropTypeB;
        K2Schema->ConvertPropertyToPinType(*ItA, PropTypeA);
        K2Schema->ConvertPropertyToPinType(*ItB, PropTypeB);

        if(PropTypeA != PropTypeB)
        {
            return false;
        }
    }

    return true;
}

void UBPNode_BaseSortArray::OnArrayPinChanged()
{
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    // Remove all pins related to archetype variables
    TArray<UEdGraphPin*> OldPins = Pins;
    TArray<UEdGraphPin*> OldClassPins;
    auto DelegatePin = GetDelegatePin();

    TArray<UEdGraphPin*> NewClassPins;

    auto ArrayPin = GetTargetArrayPin();
    if (ArrayPin->LinkedTo.Num() > 0)
    {
        FEdGraphPinType PinType = ArrayPin->LinkedTo[0]->PinType;

        ArrayPin->PinType = PinType;
        ArrayPin->PinType.bIsArray = true;
        ArrayPin->PinType.bIsReference = false;

        UBlueprint* Blueprint = GetBlueprint();

        UFunction* SignatureFunction = GetSignatureFunction(PinType);

        if (SignatureFunction)
        {
            bool ReplaceDelegatePin = false;

            if(DelegatePin)
            {
                auto DelType = DelegatePin->PinType;

                auto Sig = FMemberReference::ResolveSimpleMemberReference<UFunction>(DelegatePin->PinType.PinSubCategoryMemberReference);
                if(!AreSameSignatures(K2Schema, SignatureFunction, Sig))
                {
                    ReplaceDelegatePin = true;
                }
            }
            else
            {
                ReplaceDelegatePin = true;
            }

            if (ReplaceDelegatePin)
            {
                if (DelegatePin)
                {
                    DelegatePin->MarkPendingKill();
                    Pins.Remove(DelegatePin);
                    OldClassPins.Add(DelegatePin);
                }

                DelegatePin = CreatePin(EGPD_Input, K2Schema->PC_Delegate, TEXT(""), NULL, false, true, FBPNode_BaseSortArrayHelper::DelegatePinName, true);
                if (DelegatePin)
                {
                    FMemberReference::FillSimpleMemberReference<UFunction>(SignatureFunction, DelegatePin->PinType.PinSubCategoryMemberReference);
                    DelegatePin->PinFriendlyName = NSLOCTEXT("K2Node", "PinFriendlyDelegatetName", "Ordering Event");
                }
            }
        }
        else
        {
            // Unable to find a signature. Another part of this class with return the appropriate error to the user.

            if (DelegatePin)
            {
                DelegatePin->MarkPendingKill();
                Pins.Remove(DelegatePin);
                OldClassPins.Add(DelegatePin);
            }
        }
    }
    else
    {
        if (DelegatePin)
        {
            DelegatePin->MarkPendingKill();
            Pins.Remove(DelegatePin);
            OldClassPins.Add(DelegatePin);
        }
    }

    RewireOldPinsToNewPins(OldClassPins, NewClassPins);

    // Destroy the old pins
    DestroyPinList(OldClassPins);

    // Refresh the UI for the graph so the pin changes show up
    UEdGraph* Graph = GetGraph();
    Graph->NotifyGraphChanged();

    // Mark dirty
    FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

UFunction* UBPNode_BaseSortArray::GetDelegateSignature(bool bForceNotFromSkelClass) const
{
    auto ArrayPin = GetTargetArrayPin();
    return GetSignatureFunction(ArrayPin->PinType);
}

UEdGraphPin* UBPNode_BaseSortArray::GetDelegatePin() const
{
    return FindPin(FBPNode_BaseSortArrayHelper::DelegatePinName);
}

// TODO: are there any more dependencies?
bool UBPNode_BaseSortArray::HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const
{
    const UBlueprint* SourceBlueprint = GetBlueprint();

    auto Signature = GetDelegateSignature(true);
    UClass* SignatureSourceClass = Signature ? Signature->GetOwnerClass() : nullptr;
    const bool bSignatureResult = (SignatureSourceClass != NULL) && (SignatureSourceClass->ClassGeneratedBy != SourceBlueprint);
    if (bSignatureResult && OptionalOutput)
    {
        OptionalOutput->AddUnique(Signature);
    }

    const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
    return bSignatureResult || bSuperResult;
}

FText UBPNode_BaseSortArray::GetMenuCategory() const
{
    static FNodeTextCache CachedCategory;

    if (CachedCategory.IsOutOfDate(this))
    {
        // FText::Format() is slow, so we cache this to save on performance
        CachedCategory.SetCachedText(FEditorCategoryUtils::BuildCategoryString(FCommonEditorCategory::Utilities, LOCTEXT("ActionMenuCategory", "Array")), this);
    }
    return CachedCategory;
}

void UBPNode_BaseSortArray::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    // actions get registered under specific object-keys; the idea is that 
    // actions might have to be updated (or deleted) if their object-key is  
    // mutated (or removed)... here we use the node's class (so if the node 
    // type disappears, then the action should go with it)
    UClass* ActionKey = GetClass();

    // to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
    // check to make sure that the registrar is looking for actions of this type
    // (could be regenerating actions for a specific asset, and therefore the 
    // registrar would only accept actions corresponding to that asset)
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        check(NodeSpawner != nullptr);

        ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
    }
}

#undef LOCTEXT_NAMESPACE
