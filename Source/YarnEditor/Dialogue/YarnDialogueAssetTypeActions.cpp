#include "YarnDialogueAssetTypeActions.h"

#include "YarnEditorModule.h"
#include "Toolkit/YarnDialogueAssetEditorToolkit.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

FYarnDialogueAssetTypeActions::FYarnDialogueAssetTypeActions(EAssetTypeCategories::Type InCategory): CustomCategory(InCategory)
{}

FText FYarnDialogueAssetTypeActions::GetName() const
{
	return INVTEXT("Yarn Dialogue Asset");
}

UClass* FYarnDialogueAssetTypeActions::GetSupportedClass() const
{
	return UYarnDialogueAsset::StaticClass();
}

FColor FYarnDialogueAssetTypeActions::GetTypeColor() const
{
	return FColor::Yellow;
}

uint32 FYarnDialogueAssetTypeActions::GetCategories()
{
	return CustomCategory;
}

void FYarnDialogueAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Obj : InObjects)
	{
		if (UYarnDialogueAsset* YarnAsset = Cast<UYarnDialogueAsset>(Obj))
		{
			TSharedRef<FYarnDialogueAssetEditorToolkit> EditorToolkit = MakeShared<FYarnDialogueAssetEditorToolkit>();
			EditorToolkit->InitEditor(EditWithinLevelEditor, YarnAsset);
		}
	}
}
