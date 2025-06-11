#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class YARNEDITOR_API FYarnDialogueAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FYarnDialogueAssetTypeActions(EAssetTypeCategories::Type InCategory);

	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:
	EAssetTypeCategories::Type CustomCategory;
};
