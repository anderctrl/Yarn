#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YarnDialogueEvent.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class YARN_API UYarnDialogueEvent : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool Execute(AActor* Actor);

	virtual bool Execute_Implementation(AActor* Actor)
	{
		return false;
	}
};
