#pragma once

#include "CoreMinimal.h"
#include "BPCHandler_BaseSortArray.h"

class BPCHandler_BubbleSortArray : public BPCHandler_BaseSortArray
{
protected:
    virtual void CompileSort(FKismetFunctionContext& Context, UEdGraphNode* Node) override;

public:
    BPCHandler_BubbleSortArray(FKismetCompilerContext& InCompilerContext);
};