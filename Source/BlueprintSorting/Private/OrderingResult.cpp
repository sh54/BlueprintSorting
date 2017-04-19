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
