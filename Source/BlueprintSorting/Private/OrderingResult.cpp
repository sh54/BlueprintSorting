#include "CoreMinimal.h"

#include "OrderingResult.h"
#include "BPNode_BaseSortArray.h"


UOrderingResult::UOrderingResult( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
}


UOrderingResult* UOrderingResult::MakeOrderingResult()
{
    UOrderingResult* MyObject = NewObject<UOrderingResult>();
    MyObject->Result = ESortOrdering::Equal;
    return MyObject;
}

bool UOrderingResult::IsFirstAfterSecond(UOrderingResult* OrderingResult, ESortDirection SortDirection)
{
    if (SortDirection == ESortDirection::Ascending)
    {
        return OrderingResult->Result == ESortOrdering::GreaterThan;
    }
    return OrderingResult->Result == ESortOrdering::LessThan;
}

void UOrderingResult::SetForInt(int32 R)
{
    if (R > 0)
    {
        Result = ESortOrdering::GreaterThan;
    }
    else if (R < 0)
    {
        Result = ESortOrdering::LessThan;
    }
    else
    {
        Result = ESortOrdering::Equal;
    }
}

void UOrderingResult::SetForFloat(float R, float Epsilon)
{
    if (R - Epsilon > 0)
    {
        Result = ESortOrdering::GreaterThan;
    }
    else if (R + Epsilon < 0)
    {
        Result = ESortOrdering::LessThan;
    }
    else
    {
        Result = ESortOrdering::Equal;
    }
}

void UOrderingResult::SetForInts(int32 A, int32 B)
{
    if(A > B)
    {
        Result = ESortOrdering::GreaterThan;
    }
    else if (A < B)
    {
        Result = ESortOrdering::LessThan;
    }
    else
    {
        Result = ESortOrdering::Equal;
    }
}

void UOrderingResult::SetForStrings(const FString& A, const FString& B)
{
    return SetForInt(FCString::Strcmp(*A, *B) > 0);
}

void UOrderingResult::SetForTexts(const FText& A, const FText& B)
{
    return SetForInt(FCString::Strcmp(*A.ToString(), *B.ToString()) > 0);
}
