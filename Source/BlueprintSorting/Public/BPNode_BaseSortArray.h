#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Object.h"
#include "K2Node.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "BPNode_BaseSortArray.generated.h"

struct FKismetFunctionContext;
class FBlueprintActionDatabaseRegistrar;
class UEdGraphSchema_K2;
class UBlueprint;
class UEdGraph;
class UEdGraphPin;

UENUM(BlueprintType)
enum class ESortDirection : uint8
{
    Ascending  UMETA(DisplayName = "Ascending"),
    Descending UMETA(DisplayName = "Descending")
};

UCLASS(abstract)
class BLUEPRINTSORTING_API UBPNode_BaseSortArray : public UK2Node
{
    GENERATED_UCLASS_BODY()

public:
    // UK2Node interface
    virtual bool IsNodePure() const override { return false; }
    virtual ERedirectType DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const override;
    virtual bool HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const override;
    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    virtual FText GetMenuCategory() const override;
    // End of UK2Node interface

    // UEdGraphNode interface
    virtual void AllocateDefaultPins() override;
    virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
    virtual void PostReconstructNode() override;
    virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
    virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
    virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
    // End of UEdGraphNode interface

    UEdGraphPin* GetDelegatePin() const;
    UEdGraphPin* GetTargetArrayPin() const;
    UEdGraphPin* GetThenPin() const;
    UEdGraphPin* GetExecutePin() const;
    UEdGraphPin* GetSortDirectionPin() const;

    UFunction* GetFunction(FEdGraphPinType& PinType) const;
    UFunction* GetSignatureFunction(FEdGraphPinType& PinType) const;
    UFunction* GetSignatureFunction(FEdGraphPinType& PinType, TSubclassOf<class UObject> ClassToSearch) const;
    static bool AreSameSignatures(const UEdGraphSchema_K2* K2Schema, UFunction* SigA, UFunction* SigB);
    UFunction* GetDelegateSignature(bool bForceNotFromSkelClass = false) const;

protected:
    void OnArrayPinChanged();
};
