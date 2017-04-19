#pragma once

#include "CoreMinimal.h"
#include "KismetCompilerMisc.h"

struct FBPTerminal;

struct FMyDelegateOwnerId
{
    typedef TMap<FMyDelegateOwnerId, FBPTerminal*> FInnerTermMap;

    const class UEdGraphPin* OutputPin;
    const class UBPNode_BaseSortArray* DelegateNode;

    FMyDelegateOwnerId(const class UEdGraphPin* InOutputPin, const class UBPNode_BaseSortArray* InDelegateNode)
        : OutputPin(InOutputPin), DelegateNode(InDelegateNode)
    {
        ensure(OutputPin && DelegateNode);
    }

    bool operator==(const FMyDelegateOwnerId& Other) const
    {
        return (Other.OutputPin == OutputPin) && (Other.DelegateNode == DelegateNode);
    }

    friend uint32 GetTypeHash(const FMyDelegateOwnerId& DelegateOwnerId)
    {
        return FCrc::MemCrc_DEPRECATED(&DelegateOwnerId, sizeof(FMyDelegateOwnerId));
    }
};

class BPCHandler_BaseSortArray : public FNodeHandlingFunctor
{
protected:
    EKismetCompiledStatementType Command;
    FMyDelegateOwnerId::FInnerTermMap InnerTermMap;

    TMap<UEdGraphNode*, FBPTerminal*> BoolTermMap;
    TMap<UEdGraphNode*, FBPTerminal*> BoolSomeAlterationTermMap; // RENAME
    TMap<UEdGraphNode*, FBPTerminal*> OrderingResultTermMap;
    TMap<UEdGraphNode*, FBPTerminal*> ArrayItem1TermMap;
    TMap<UEdGraphNode*, FBPTerminal*> ArrayItem2TermMap;
    TMap<UEdGraphNode*, FBPTerminal*> ArrayIndex1TermMap;
    TMap<UEdGraphNode*, FBPTerminal*> ArrayIndex2TermMap;
    TMap<UEdGraphNode*, FBPTerminal*> SortDelegateTermMap;

    virtual void CompileSort(FKismetFunctionContext& Context, UEdGraphNode* Node) = 0;

    UFunction* GetMakeOrderingResultFunction();
    UFunction* GetIsFirstAfterSecondFunction();
    UFunction* GetSwapItemsFunction();
    UFunction* GetIncrementFunction();
    UFunction* GetGreaterThanOrEqualArrayLengthFunction();
    UFunction* GetLessThanOrEqualArrayLengthFunction();

public:
    BPCHandler_BaseSortArray(FKismetCompilerContext& InCompilerContext);

    virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override;

    UFunction* FindFunction(FKismetFunctionContext& Context, UEdGraphNode* Node) const;
    FBPTerminal* FindFunctionContext(FKismetFunctionContext& Context, UEdGraphNode* Node) const;

    // Get the name of the function to call from the node
    FString GetFunctionNameFromNode(UEdGraphNode* Node) const;

    UClass* GetCallingContext(FKismetFunctionContext& Context, UEdGraphNode* Node) const;
    
    virtual void Compile(FKismetFunctionContext& Context, UEdGraphNode* Node) override;
};
