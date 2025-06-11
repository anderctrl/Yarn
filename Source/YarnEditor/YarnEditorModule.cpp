// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "YarnEditorModule.h"

#include "EdGraphUtilities.h"
#include "IAssetTools.h"
#include "Dialogue/YarnDialogueAssetTypeActions.h"
#include "Dialogue/Toolkit/DialogueRedirectLog.h"
#include "Dialogue/Toolkit/Graph/YarnDialogueNodeFactory.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyleRegistry.h"

IMPLEMENT_MODULE(FYarnEditorModule, YarnEditor)
DEFINE_LOG_CATEGORY(LogYarnDialogueEditor);

TSharedPtr<FSlateStyleSet> FYarnEditorModule::StyleSet = nullptr;

void FYarnEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	const EAssetTypeCategories::Type YarnCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName("Yarn"), FText::FromString("Yarn"));

	YarnDialogueAssetTypeActions = MakeShareable(new FYarnDialogueAssetTypeActions(YarnCategory));

	AssetTools.RegisterAssetTypeActions(YarnDialogueAssetTypeActions.ToSharedRef());

	YarnNodeFactory = MakeShared<FYarnDialogueNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(YarnNodeFactory);

	InitializeStyle();

	GLog->AddOutputDevice(&FDialogueRedirectLog::Get());
}

void FYarnEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && YarnDialogueAssetTypeActions.IsValid())
	{
		const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().UnregisterAssetTypeActions(YarnDialogueAssetTypeActions.ToSharedRef());
		YarnDialogueAssetTypeActions.Reset();
	}

	if (YarnNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(YarnNodeFactory);
		YarnNodeFactory.Reset();
	}

	GLog->RemoveOutputDevice(&FDialogueRedirectLog::Get());

	ShutdownStyle();
}

TSharedPtr<FSlateStyleSet> FYarnEditorModule::GetStyleSet()
{
	return StyleSet;
}

void FYarnEditorModule::InitializeStyle()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShared<FSlateStyleSet>("YarnEditorStyle");

	FSlateFontInfo LogFont = FCoreStyle::Get().GetFontStyle("Log.Font");

	constexpr FLinearColor MutedWhite = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	constexpr FLinearColor MutedYellow = FLinearColor(1.0f, 0.85f, 0.3f, 1.0f);
	constexpr FLinearColor MutedRed = FLinearColor(0.85f, 0.3f, 0.3f, 1.0f);
	constexpr FLinearColor MutedGreen = FLinearColor(0.3f, 0.85f, 0.3f, 1.0f);


	const FTextBlockStyle InfoStyle = FTextBlockStyle().SetFont(LogFont).SetColorAndOpacity(MutedWhite);

	FTextBlockStyle WarningStyle = InfoStyle;
	WarningStyle.SetColorAndOpacity(MutedYellow);

	FTextBlockStyle ErrorStyle = InfoStyle;
	ErrorStyle.SetColorAndOpacity(MutedRed);

	FTextBlockStyle SuccessStyle = InfoStyle;
	SuccessStyle.SetColorAndOpacity(MutedGreen);

	StyleSet->Set("Log.Info", InfoStyle);
	StyleSet->Set("Log.Warning", WarningStyle);
	StyleSet->Set("Log.Error", ErrorStyle);
	StyleSet->Set("Log.Success", SuccessStyle);

	StyleSet->SetContentRoot(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Yarn/Resources")));

	StyleSet->Set("ClassThumbnail.YarnDialogueAsset", new FSlateImageBrush(
		              StyleSet->RootToContentDir(TEXT("Icons/DialogueIcon"), TEXT(".png")),
		              FVector2D(64.f, 64.f)));

	StyleSet->Set("ClassIcon.YarnDialogueAsset", new FSlateImageBrush(
		              StyleSet->RootToContentDir(TEXT("Icons/DialogueIcon"), TEXT(".png")),
		              FVector2D(16.f, 16.f)));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FYarnEditorModule::ShutdownStyle()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		StyleSet.Reset();
	}
}
