#pragma once

#include "CoreMinimal.h"

class SDialogueLogWindow;
class UYarnDialogueGraph;
class SYarnDialogueEditorWidget;
class UYarnDialogueAsset;


class YARNEDITOR_API FYarnDialogueAssetEditorToolkit : public FAssetEditorToolkit, public FEditorUndoClient
{
public:
	FYarnDialogueAssetEditorToolkit();
	
	void InitEditor(TSharedPtr<IToolkitHost> InitToolkitHost, UYarnDialogueAsset* InAsset);

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

protected:
	
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	
	TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnLogTab(const FSpawnTabArgs& Args);
	
	void OnTabClosed(TSharedRef<SDockTab> ClosedTab);

	virtual void SaveAsset_Execute() override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;


	static const FName YarnEditorTabId;
	static const FName YarnGraphTabId;
	static const FName YarnLogTabId;

	TWeakObjectPtr<UYarnDialogueAsset> YarnAsset;
	TSharedPtr<SYarnDialogueEditorWidget> EditorWidget;
	TSharedPtr<SDialogueLogWindow> LogWindowWidget;
	UYarnDialogueGraph* DialogueGraph = nullptr;
};
