#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "YarnDialogueAssetFactory.generated.h"

/**
 * 
 */
UCLASS()
class YARNEDITOR_API UYarnDialogueAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UYarnDialogueAssetFactory();
	virtual FText GetDisplayName() const override;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ShouldShowInNewMenu() const override;
};
