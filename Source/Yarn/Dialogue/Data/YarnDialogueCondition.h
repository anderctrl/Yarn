#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YarnDialogueCondition.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class YARN_API UYarnDialogueCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool EvaluateCondition(AActor* Actor);

	virtual bool EvaluateCondition_Implementation(AActor* Actor)
	{
		return false;
	}
};
