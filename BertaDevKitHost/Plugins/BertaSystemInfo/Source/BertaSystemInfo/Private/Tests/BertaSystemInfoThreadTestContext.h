#pragma once

#include "HAL/ThreadSafeCounter.h"
#include "UObject/Object.h"
#include "BertaSystemInfoThreadTestContext.generated.h"

UCLASS()
class UBertaSystemInfoThreadTestContext final : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override
	{
		WorldLookupCount.Increment();
		return nullptr;
	}

	mutable FThreadSafeCounter WorldLookupCount;
};
