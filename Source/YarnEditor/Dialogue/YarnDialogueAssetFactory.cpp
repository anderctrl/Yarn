#include "YarnDialogueAssetFactory.h"

#include "AssetTypeCategories.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

UYarnDialogueAssetFactory::UYarnDialogueAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYarnDialogueAsset::StaticClass();
}

FText UYarnDialogueAssetFactory::GetDisplayName() const
{
	return INVTEXT("Yarn Dialogue");
}

UObject* UYarnDialogueAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                     EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UYarnDialogueAsset* NewAsset = NewObject<UYarnDialogueAsset>(InParent, InClass, InName, Flags | RF_Transactional);
	return NewAsset;
}

bool UYarnDialogueAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}
