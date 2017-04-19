#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Object.h"
#include "K2Node.h"
#include "BPNode_BaseSortArray.h"
#include "BPNode_SortArray.generated.h"


UCLASS(BlueprintType, Blueprintable)
class BLUEPRINTSORTING_API UBPNode_SortArray : public UBPNode_BaseSortArray
{
    GENERATED_UCLASS_BODY()
public:
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
};
