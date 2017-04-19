#include "CoreMinimal.h"

#include "BPCHandler_BubbleSortArray.h"
#include "BPCHandler_BaseSortArray.h"
#include "BPNode_BaseSortArray.h"
#include "Engine/Blueprint.h"
#include "Engine/MemberReference.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "EdGraphUtilities.h"
#include "EditorCategoryUtils.h"
#include "BlueprintNodeSpawner.h"
#include "OrderingResult.h"

#define LOCTEXT_NAMESPACE "BlueprintSorting"

BPCHandler_BubbleSortArray::BPCHandler_BubbleSortArray(FKismetCompilerContext& InCompilerContext)
    : BPCHandler_BaseSortArray(InCompilerContext)
{
}

// Add all the statements necessary for the bubble sort algorithm
void BPCHandler_BubbleSortArray::CompileSort(FKismetFunctionContext& Context, UEdGraphNode* Node)
{
    UBPNode_BaseSortArray* ArrayNode = CastChecked<UBPNode_BaseSortArray>(Node);

    FBPTerminal* OrderingResultTerm = OrderingResultTermMap.FindRef(Node);
    FBPTerminal* ArrayItem1Term = ArrayItem1TermMap.FindRef(Node);
    FBPTerminal* ArrayIndex1Term = ArrayIndex1TermMap.FindRef(Node);
    FBPTerminal* ArrayItem2Term = ArrayItem2TermMap.FindRef(Node);
    FBPTerminal* ArrayIndex2Term = ArrayIndex2TermMap.FindRef(Node);
    FBPTerminal* BoolTerm = BoolTermMap.FindRef(Node);
    FBPTerminal* BoolNoAlterationsTerm = BoolSomeAlterationTermMap.FindRef(Node);

    UEdGraphPin* DirectionPin = Context.FindRequiredPinByName(Node, "Sort Direction", EGPD_Input);
    UEdGraphPin* PinToTry = FEdGraphUtilities::GetNetFromPin(DirectionPin);
    FBPTerminal** DirectionTerm = Context.NetMap.Find(PinToTry);

    FEdGraphPinType ExpectedExecPinType;
    ExpectedExecPinType.PinCategory = UEdGraphSchema_K2::PC_Exec;

    // Make sure that the input pin is connected and valid for this block
    UEdGraphPin* ExecTriggeringPin = Context.FindRequiredPinByName(ArrayNode, UEdGraphSchema_K2::PN_Execute, EGPD_Input);
    if ((ExecTriggeringPin == NULL) || !Context.ValidatePinType(ExecTriggeringPin, ExpectedExecPinType))
    {
        CompilerContext.MessageLog.Error(*FString::Printf(*LOCTEXT("NoValidExecutionPinForSortArray_Error", "@@ must have a valid execution pin @@").ToString()), ArrayNode, ExecTriggeringPin);
        return;
    }

    UFunction* OrderingFunction = FindFunction(Context, Node);
    FBPTerminal* OrderingFunctionContext = FindFunctionContext(Context, Node);
    if(!OrderingFunction)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(*LOCTEXT("NoValidFunctionForSortArray_Error", "Unable to find a vaild sort function to call for @@").ToString()), ArrayNode);
        return;
    }

    auto MakeOrderingResult = GetMakeOrderingResultFunction();
    auto IsFirstAfterSecond = GetIsFirstAfterSecondFunction();
    auto SwapItems = GetSwapItemsFunction();
    auto Increment = GetIncrementFunction();
    auto GreaterThanOrEqualArrayLength = GetGreaterThanOrEqualArrayLengthFunction();
    auto LessThanOrEqualArrayLength = GetLessThanOrEqualArrayLengthFunction();

    UEdGraphPin* ArrayPinNet = FEdGraphUtilities::GetNetFromPin(ArrayNode->GetTargetArrayPin());

    FBPTerminal** ArrayTerm = Context.NetMap.Find(ArrayPinNet);

    // Pointers
    FBlueprintCompiledStatement* JumpToEndPtr;
    FBlueprintCompiledStatement* JumpOverSwapPtr;
    FBlueprintCompiledStatement* InnerLoopPtr;
    FBlueprintCompiledStatement* MainLoopPtr;


    // If array length is 0 or 1 then no sorting is needed to goto the end
    {
        // Assign literal int 2 to some local value
        {
            FBPTerminal* LiteralIndexTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
            LiteralIndexTerm->bIsLiteral = true;
            LiteralIndexTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Int;
            LiteralIndexTerm->Name = TEXT("2");

            FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
            AssignStatement.Type = KCST_Assignment;
            AssignStatement.LHS = ArrayIndex1Term;
            AssignStatement.RHS.Add(LiteralIndexTerm);
        }

        // Check against array length
        {
            FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
            Statement.Type = KCST_CallFunction;
            Statement.FunctionToCall = LessThanOrEqualArrayLength;
            //Statement.FunctionContext = FuncContext;
            Statement.bIsParentContext = false;

            Statement.LHS = BoolTerm;
            Statement.RHS.Add(*ArrayTerm);
            Statement.RHS.Add(ArrayIndex1Term);
        }

        // Jump to end if less than 2 items in array
        {
            FBlueprintCompiledStatement& JumpToEnd = Context.AppendStatementForNode(Node);
            JumpToEndPtr = &JumpToEnd;
            JumpToEnd.Type = KCST_GotoIfNot;
            JumpToEnd.LHS = BoolTerm;

            // JumpToEnd.TargetLabel gets assigned later on
        }
    }

    // Create Ordering Result
    // Only one is needed since it will just get reused.
    {
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
        Statement.Type = KCST_CallFunction;
        Statement.FunctionToCall = MakeOrderingResult;
        Statement.bIsParentContext = false;
        Statement.LHS = OrderingResultTerm;
    }

    // Make a label
    // This is the main loop
    // If any swaps occur then return back here and loop again
    {
        FBlueprintCompiledStatement& MainLoop = Context.AppendStatementForNode(ArrayNode);
        MainLoop.Type = KCST_Nop;
        MainLoop.bIsJumpTarget = true;
        MainLoopPtr = &MainLoop;
    }

    // Assign inital no swaps monitor variable to true
    {
        FBPTerminal* LiteralTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
        LiteralTerm->bIsLiteral = true;
        LiteralTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Boolean;
        LiteralTerm->Name = TEXT("true");

        FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
        AssignStatement.Type = KCST_Assignment;
        AssignStatement.LHS = BoolNoAlterationsTerm;
        AssignStatement.RHS.Add(LiteralTerm);
    }

    // Assign Array lookup Index A to 0
    {
        FBPTerminal* LiteralIndexTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
        LiteralIndexTerm->bIsLiteral = true;
        LiteralIndexTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Int;
        LiteralIndexTerm->Name = TEXT("0");

        FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
        AssignStatement.Type = KCST_Assignment;
        AssignStatement.LHS = ArrayIndex1Term;
        AssignStatement.RHS.Add(LiteralIndexTerm);
    }

    // Assign Array lookup Index B to 1
    {
        FBPTerminal* LiteralIndexTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
        LiteralIndexTerm->bIsLiteral = true;
        LiteralIndexTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Int;
        LiteralIndexTerm->Name = TEXT("1");

        FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
        AssignStatement.Type = KCST_Assignment;
        AssignStatement.LHS = ArrayIndex2Term;
        AssignStatement.RHS.Add(LiteralIndexTerm);
    }

    // Inner loop
    // For Array[1..n] we loop through pairs (1, 2), (2, 3), ..., (n-1, n) and compare them
    // If the first is greater than the second then we swap the array elements
    {
        FBlueprintCompiledStatement& InnerLoop = Context.AppendStatementForNode(ArrayNode);
        InnerLoop.Type = KCST_Nop;
        InnerLoop.bIsJumpTarget = true;
        InnerLoopPtr = &InnerLoop;
    }

    // Get item from array at index A
    {
        FBlueprintCompiledStatement* StatementPtr = new FBlueprintCompiledStatement();
        FBlueprintCompiledStatement& Statement = *StatementPtr;
        Context.AllGeneratedStatements.Add(StatementPtr);
        Statement.Type = KCST_ArrayGetByRef;
        Statement.RHS.Add(*ArrayTerm);
        Statement.RHS.Add(ArrayIndex1Term);
        ArrayItem1Term->InlineGeneratedParameter = &Statement;
    }

    // Get item from array at index B
    {
        FBlueprintCompiledStatement* StatementPtr = new FBlueprintCompiledStatement();
        FBlueprintCompiledStatement& Statement = *StatementPtr;
        Context.AllGeneratedStatements.Add(StatementPtr);
        Statement.Type = KCST_ArrayGetByRef;
        Statement.RHS.Add(*ArrayTerm);
        Statement.RHS.Add(ArrayIndex2Term);
        ArrayItem2Term->InlineGeneratedParameter = &Statement;
    }

    // Call ordering function
    {
        // See FKCHandler_CallFunction::CreateFunctionCallStatement for more advanced stuff
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(Node);
        Statement.FunctionToCall = OrderingFunction;
        Statement.FunctionContext = OrderingFunctionContext;
        Statement.bIsParentContext = false;
        Statement.Type = KCST_CallFunction;
        Statement.RHS.Add(ArrayItem1Term);
        Statement.RHS.Add(ArrayItem2Term);
        Statement.RHS.Add(OrderingResultTerm);
    }

    // See if the first item should go after the second item
    {
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
        Statement.Type = KCST_CallFunction;
        Statement.FunctionToCall = IsFirstAfterSecond;
        //Statement.FunctionContext = FuncContext;
        Statement.bIsParentContext = false;

        Statement.LHS = BoolTerm;
        Statement.RHS.Add(OrderingResultTerm);
        Statement.RHS.Add(*DirectionTerm);
    }

    // If if the first item should go after the second item then we will swap the elements and mark that a swap has been made
    // Otherwise jump this
    {
        FBlueprintCompiledStatement& JumpOverSwap = Context.AppendStatementForNode(Node);
        JumpOverSwap.Type = KCST_GotoIfNot;
        JumpOverSwap.LHS = BoolTerm;
        JumpOverSwapPtr = &JumpOverSwap;
    }

    // Set no swaps to false
    {
        FBPTerminal* LiteralTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
        LiteralTerm->bIsLiteral = true;
        LiteralTerm->Type.PinCategory = CompilerContext.GetSchema()->PC_Boolean;
        LiteralTerm->Name = TEXT("false");

        FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
        AssignStatement.Type = KCST_Assignment;
        AssignStatement.LHS = BoolNoAlterationsTerm;
        AssignStatement.RHS.Add(LiteralTerm);
    }

    // Swap the items in the array
    {
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
        Statement.Type = KCST_CallFunction;
        Statement.FunctionToCall = SwapItems;
        Statement.bIsParentContext = false;

        Statement.RHS.Add(*ArrayTerm);
        Statement.RHS.Add(ArrayIndex1Term);
        Statement.RHS.Add(ArrayIndex2Term);
    }

    // Label to jump to if no swaps are to be done
    {
        FBlueprintCompiledStatement& AfterSwap = Context.AppendStatementForNode(ArrayNode);
        AfterSwap.Type = KCST_Nop;
        AfterSwap.bIsJumpTarget = true;

        JumpOverSwapPtr->TargetLabel = &AfterSwap;
    }

    // Increment index A
    {
        FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
        AssignStatement.Type = KCST_Assignment;
        AssignStatement.LHS = ArrayIndex1Term;
        AssignStatement.RHS.Add(ArrayIndex2Term);
    }

    // Increment index B
    {
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
        Statement.Type = KCST_CallFunction;
        Statement.FunctionToCall = Increment;
        Statement.bIsParentContext = false;

        Statement.LHS = ArrayIndex2Term;
        Statement.RHS.Add(ArrayIndex1Term);
    }

    // See if all pairs have been checked
    // Do this by checking index B against array length
    {
        FBlueprintCompiledStatement& Statement = Context.AppendStatementForNode(ArrayNode);
        Statement.Type = KCST_CallFunction;
        Statement.FunctionToCall = GreaterThanOrEqualArrayLength;
        Statement.bIsParentContext = false;

        Statement.LHS = BoolTerm;
        Statement.RHS.Add(*ArrayTerm);
        Statement.RHS.Add(ArrayIndex2Term);
    }

    // If there are more adjacent pairs to test then jump back to the relevant point
    {
        FBlueprintCompiledStatement& JumpInnerLoopStart = Context.AppendStatementForNode(Node);
        JumpInnerLoopStart.Type = KCST_GotoIfNot;
        JumpInnerLoopStart.LHS = BoolTerm;
        JumpInnerLoopStart.TargetLabel = InnerLoopPtr;
    }

    // If any swaps occured then jump back and check orderings of all adjacent pairs
    {
        FBlueprintCompiledStatement& JumpInnerLoopStart = Context.AppendStatementForNode(Node);
        JumpInnerLoopStart.Type = KCST_GotoIfNot;
        JumpInnerLoopStart.LHS = BoolNoAlterationsTerm;
        JumpInnerLoopStart.TargetLabel = MainLoopPtr;
    }

    // Label for the end
    {
        FBlueprintCompiledStatement& End = Context.AppendStatementForNode(ArrayNode);
        End.Type = KCST_Nop;
        End.bIsJumpTarget = true;

        // Adding a link
        JumpToEndPtr->TargetLabel = &End;
    }
}

#undef LOCTEXT_NAMESPACE
