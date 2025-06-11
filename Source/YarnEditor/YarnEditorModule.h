// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Dialogue/YarnDialogueAssetTypeActions.h"
#include "Modules/ModuleManager.h"

struct FGraphPanelNodeFactory;
DECLARE_LOG_CATEGORY_EXTERN(LogYarnDialogueEditor, Verbose, All);

class FYarnEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	static TSharedPtr<FSlateStyleSet> GetStyleSet();

protected:
	TSharedPtr<FYarnDialogueAssetTypeActions> YarnDialogueAssetTypeActions;
	TSharedPtr<FGraphPanelNodeFactory> YarnNodeFactory;

private:
	static void InitializeStyle();
	static void ShutdownStyle();
	static TSharedPtr<FSlateStyleSet> StyleSet;
};
