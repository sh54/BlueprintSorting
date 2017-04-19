#include "CoreMinimal.h"

#include "BPNode_SortArray.h"
#include "BPCHandler_BubbleSortArray.h"

#define LOCTEXT_NAMESPACE "BlueprintSorting"

UBPNode_SortArray::UBPNode_SortArray(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FNodeHandlingFunctor* UBPNode_SortArray::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
    // This uses bubble sort for now...
    return new BPCHandler_BubbleSortArray(CompilerContext);
}

FText UBPNode_SortArray::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("UBPNode_SortArray.NodeTitle", "Sort Array");
}

#undef LOCTEXT_NAMESPACE
