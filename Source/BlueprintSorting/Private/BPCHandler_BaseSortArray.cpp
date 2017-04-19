#include "CoreMinimal.h"

#include "BPCHandler_BaseSortArray.h"
#include "BPNode_BaseSortArray.h"
#include "BlueprintSortingHelperLibrary.h"
#include "Engine/Blueprint.h"
#include "Engine/MemberReference.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_CreateDelegate.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "KismetCompiler.h"
#include "OrderingResult.h"

#define LOCTEXT_NAMESPACE "BlueprintSorting"

BPCHandler_BaseSortArray::BPCHandler_BaseSortArray(FKismetCompilerContext& InCompilerContext)
    : FNodeHandlingFunctor(InCompilerContext)
{
}

UFunction* BPCHandler_BaseSortArray::GetMakeOrderingResultFunction()
{
    return UOrderingResult::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UOrderingResult, MakeOrderingResult));
}

UFunction* BPCHandler_BaseSortArray::GetIsFirstAfterSecondFunction()
{
    return UOrderingResult::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UOrderingResult, IsFirstAfterSecond));
}

UFunction* BPCHandler_BaseSortArray::GetSwapItemsFunction()
{
    return UBlueprintSortingHelperLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UBlueprintSortingHelperLibrary, SwapItems));
}

UFunction* BPCHandler_BaseSortArray::GetIncrementFunction()
{
    return UBlueprintSortingHelperLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UBlueprintSortingHelperLibrary, Increment));
}

UFunction* BPCHandler_BaseSortArray::GetGreaterThanOrEqualArrayLengthFunction()
{
    return UBlueprintSortingHelperLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UBlueprintSortingHelperLibrary, GreaterThanOrEqualArrayLength));
}

UFunction* BPCHandler_BaseSortArray::GetLessThanOrEqualArrayLengthFunction()
{
    return UBlueprintSortingHelperLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UBlueprintSortingHelperLibrary, LessThanOrEqualArrayLength));
}

void BPCHandler_BaseSortArray::RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node)
{
    UBPNode_BaseSortArray* ArrayNode = CastChecked<UBPNode_BaseSortArray>(Node);
    FNodeHandlingFunctor::RegisterNets(Context, Node);

    auto DelegatePin = ArrayNode->GetDelegatePin();
    auto ArrayPin = ArrayNode->GetTargetArrayPin();

    if(ArrayPin->LinkedTo.Num() == 0)
    {
        CompilerContext.MessageLog.Error(*FString(*LOCTEXT("BaseSortArray_NoArrayLink", "Array pin is not connected @@").ToString()), ArrayNode);
    }

    if(DelegatePin)
    {
        if (DelegatePin->LinkedTo.Num() == 0)
        {
            CompilerContext.MessageLog.Error(*FString(*LOCTEXT("BaseSortArray_NoDelegateLink", "Sort Event pin is not connected @@").ToString()), ArrayNode);
        }

        //auto BlueprintClass = ArrayNode->GetBlueprintClassFromNode();
        //auto NewSimpleRef = DelegatePin->PinType.PinSubCategoryMemberReference;
        // TODO: see if this is actually needed... It will mess up delegate signatures stored in other classes
        //NewSimpleRef.MemberParent = BlueprintClass;
    }
    else
    {
        CompilerContext.MessageLog.Error(*FString(*LOCTEXT("BaseSortArray_NoSortDelegate", "Cannot determine what signature to use for the sort event @@").ToString()), ArrayNode);
    }

    // Create a term to determine if the compare was successful or not
    FBPTerminal* OrderingResultTerm = Context.CreateLocalTerminal();
    OrderingResultTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Object;
    OrderingResultTerm->Source = Node;
    OrderingResultTerm->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_OrderingResult");
    OrderingResultTermMap.Add(Node, OrderingResultTerm);

    FBPTerminal* BoolTerm = Context.CreateLocalTerminal();
    BoolTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Boolean;
    BoolTerm->Source = Node;
    BoolTerm->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_CmpSuccess");
    BoolTermMap.Add(Node, BoolTerm);

    FBPTerminal* BoolSomeAlterationTerm = Context.CreateLocalTerminal();
    BoolSomeAlterationTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Boolean;
    BoolSomeAlterationTerm->Source = Node;
    BoolSomeAlterationTerm->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_SomeAlteration");
    BoolSomeAlterationTermMap.Add(Node, BoolSomeAlterationTerm);

    FBPTerminal* ArrayItem1Term = Context.CreateLocalTerminal();
    ArrayItem1Term->Type.PinCategory = CompilerContext.GetSchema()->PC_Object; // TODO... base on array pin type...
    ArrayItem1Term->Source = Node;
    ArrayItem1Term->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_Obj1");
    ArrayItem1TermMap.Add(Node, ArrayItem1Term);

    FBPTerminal* ArrayItem2Term = Context.CreateLocalTerminal();
    ArrayItem2Term->Type.PinCategory = CompilerContext.GetSchema()->PC_Object; // TODO... base on array pin type...
    ArrayItem2Term->Source = Node;
    ArrayItem2Term->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_Obj2");
    ArrayItem2TermMap.Add(Node, ArrayItem2Term);

    FBPTerminal* ArrayIndex1Term = Context.CreateLocalTerminal();
    ArrayIndex1Term->Type.PinCategory = CompilerContext.GetSchema()->PC_Int;
    ArrayIndex1Term->Source = Node;
    ArrayIndex1Term->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_Index1");
    ArrayIndex1TermMap.Add(Node, ArrayIndex1Term);

    FBPTerminal* ArrayIndex2Term = Context.CreateLocalTerminal();
    ArrayIndex2Term->Type.PinCategory = CompilerContext.GetSchema()->PC_Int;
    ArrayIndex2Term->Source = Node;
    ArrayIndex2Term->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_Index2");
    ArrayIndex2TermMap.Add(Node, ArrayIndex2Term);
}

FBPTerminal* BPCHandler_BaseSortArray::FindFunctionContext(FKismetFunctionContext& Context, UEdGraphNode* Node) const
{
    UBPNode_BaseSortArray* SortNode = CastChecked<UBPNode_BaseSortArray>(Node);

    auto DelegatePin = SortNode->GetDelegatePin();
    if (DelegatePin)
    {
        if (DelegatePin->LinkedTo.Num() > 0)
        {
            auto Linked = DelegatePin->LinkedTo[0];

            auto CreateDelegateNode = Cast<UK2Node_CreateDelegate>(Linked->GetOwningNode());
            if (CreateDelegateNode)
            {
                auto Schema = CompilerContext.GetSchema();
                auto SelfPin = Schema->FindSelfPin(*CreateDelegateNode, EGPD_Input);
                if (SelfPin)
                {
                    if (SelfPin->LinkedTo.Num() > 0)
                    {
                        FBPTerminal** pContextTerm = Context.NetMap.Find(SelfPin->LinkedTo[0]);
                        return *pContextTerm;
                    }
                }
            }
        }
    }

    return nullptr;
}

UFunction* BPCHandler_BaseSortArray::FindFunction(FKismetFunctionContext& Context, UEdGraphNode* Node) const
{
    UClass* CallingContext = GetCallingContext(Context, Node);

    if (CallingContext)
    {
        FString FunctionName = GetFunctionNameFromNode(Node);

        return CallingContext->FindFunctionByName(*FunctionName);
    }

    return nullptr;
}

// Get the name of the function to call from the node
FString BPCHandler_BaseSortArray::GetFunctionNameFromNode(UEdGraphNode* Node) const
{
    const UBPNode_BaseSortArray* SortNode = CastChecked<UBPNode_BaseSortArray>(Node);
    FName FuncName;
    auto Link = SortNode->GetDelegatePin()->LinkedTo[0];
    auto LinkNode = Link->GetOwningNode();
    auto EventNode = Cast<UK2Node_Event>(LinkNode);
    if (EventNode)
    {
        return EventNode->CustomFunctionName.ToString();
    }
    else
    {
        auto CreateDelegateNode = Cast<UK2Node_CreateDelegate>(LinkNode);
        if (CreateDelegateNode)
        {
            return CreateDelegateNode->SelectedFunctionName.ToString();
        }
    }

    CompilerContext.MessageLog.Error(*NSLOCTEXT("KismetCompiler", "UnableResolveFunctionName_Error", "Unable to resolve function name for @@").ToString(), Node);
    return TEXT("");
}

UClass* BPCHandler_BaseSortArray::GetCallingContext(FKismetFunctionContext& Context, UEdGraphNode* Node) const
{
    UBPNode_BaseSortArray* SortNode = CastChecked<UBPNode_BaseSortArray>(Node);

    auto DelegatePin = SortNode->GetDelegatePin();
    if(DelegatePin)
    {
        if (DelegatePin->LinkedTo.Num() > 0)
        {
            auto Linked = DelegatePin->LinkedTo[0];

            // If the delegate pin is linked to a create delegate then a function in an object is going to get invoked 
            // which means that the context is going to be that object.
            auto CreateDelegateNode = Cast<UK2Node_CreateDelegate>(Linked->GetOwningNode());
            if(CreateDelegateNode)
            {
                auto Schema = CompilerContext.GetSchema();
                auto SelfPin = Schema->FindSelfPin(*CreateDelegateNode, EGPD_Input);
                if(SelfPin)
                {
                    if (SelfPin->LinkedTo.Num() > 0)
                    {
                        auto SelfPinkLinked = SelfPin->LinkedTo[0];
                        auto CreateDelegateSearchScope = Cast<UClass>(Context.GetScopeFromPinType(SelfPinkLinked->PinType, Context.NewClass));
                        return CreateDelegateSearchScope;
                    }
                }
            }
        }
    }

    // Find the calling scope
    UClass* SearchScope = Context.NewClass;
    return SearchScope;
}

void BPCHandler_BaseSortArray::Compile(FKismetFunctionContext& Context, UEdGraphNode* Node)
{
    UBPNode_BaseSortArray* ArrayNode = CastChecked<UBPNode_BaseSortArray>(Node);

    CompileSort(Context, Node);

    GenerateSimpleThenGoto(Context, *ArrayNode, ArrayNode->FindPin(CompilerContext.GetSchema()->PN_Then));
    FNodeHandlingFunctor::Compile(Context, ArrayNode);
}

#undef LOCTEXT_NAMESPACE
