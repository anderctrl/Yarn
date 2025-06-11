#include "YarnDialogueAssetEditorToolkit.h"

#include "FileHelpers.h"
#include "YarnEditorModule.h"
#include "Dialogue/YarnDialogueEditorHelpers.h"
#include "Graph/YarnDialogueGraph.h"
#include "Graph/YarnDialogueGraphSchema.h"
#include "Widgets/SDialogueLogWindow.h"
#include "Widgets/SYarnDialogueEditorWidget.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

const FName FYarnDialogueAssetEditorToolkit::YarnEditorTabId(TEXT("YarnDialogueEditor_Main"));
const FName FYarnDialogueAssetEditorToolkit::YarnGraphTabId(TEXT("YarnDialogueEditor_Graph"));
const FName FYarnDialogueAssetEditorToolkit::YarnLogTabId(TEXT("YarnDialogueEditor_Log"));

FYarnDialogueAssetEditorToolkit::FYarnDialogueAssetEditorToolkit()
{
	ToolkitCommands = MakeShareable(new FUICommandList());
}

void FYarnDialogueAssetEditorToolkit::InitEditor(TSharedPtr<IToolkitHost> InitToolkitHost, UYarnDialogueAsset* InAsset)
{
	check(InAsset);
	YarnAsset = InAsset;

	DialogueGraph = NewObject<UYarnDialogueGraph>(UYarnDialogueGraph::StaticClass());
	DialogueGraph->Schema = UYarnDialogueGraphSchema::StaticClass();

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("YarnDialogueEditorLayout_V4")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->Split(
					FTabManager::NewStack()
					->AddTab(YarnGraphTabId, ETabState::OpenedTab)
					->SetHideTabWell(false)
					->SetSizeCoefficient(0.4f)
				)
				->Split(
					FTabManager::NewStack()
					->AddTab(YarnEditorTabId, ETabState::OpenedTab)
					->SetHideTabWell(false)
					->SetSizeCoefficient(0.6f)
				)
			)
			->Split(
				FTabManager::NewStack()
				->AddTab(YarnLogTabId, ETabState::OpenedTab)
				->SetHideTabWell(false)
				->SetSizeCoefficient(0.3f)
			)
		);


	const EToolkitMode::Type Mode = InitToolkitHost.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	InitAssetEditor(
		Mode,
		InitToolkitHost,
		FName("YarnDialogueEditorApp"),
		Layout,
		true,
		true,
		{InAsset}
	);

	if (FYarnDialogueEditorHelpers::TestYarnAssetValidity(YarnAsset.Get()))
	{
		DialogueGraph->Initialize(YarnAsset.Get());
	}

	GEditor->RegisterForUndo(this);
}


void FYarnDialogueAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("Yarn Dialogue"));
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(YarnEditorTabId,
	                                 FOnSpawnTab::CreateSP(this, &FYarnDialogueAssetEditorToolkit::SpawnEditorTab))
	            .SetDisplayName(INVTEXT("Dialogue Script"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Comment"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(YarnGraphTabId,
	                                 FOnSpawnTab::CreateSP(this, &FYarnDialogueAssetEditorToolkit::SpawnGraphTab))
	            .SetDisplayName(INVTEXT("Dialogue Graph"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Blueprint"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
	InTabManager->RegisterTabSpawner(YarnLogTabId,
	                                 FOnSpawnTab::CreateSP(this, &FYarnDialogueAssetEditorToolkit::SpawnLogTab))
	            .SetDisplayName(INVTEXT("Dialogue Log"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Audit"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FYarnDialogueAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(YarnEditorTabId);
}


TSharedRef<SDockTab> FYarnDialogueAssetEditorToolkit::SpawnEditorTab(const FSpawnTabArgs& Args)
{
	FSlateIcon Icon = FSlateIcon(FAppStyle::Get().GetStyleSetName(), "BlueprintEditor.TabIcon");

	return SNew(SDockTab)
		.TabRole(NomadTab)
		.OnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FYarnDialogueAssetEditorToolkit::OnTabClosed))
		[
			SAssignNew(EditorWidget, SYarnDialogueEditorWidget)
			.YarnAsset(YarnAsset.Get())
		];
}

TSharedRef<SDockTab> FYarnDialogueAssetEditorToolkit::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SGraphEditor> GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(DialogueGraph)
		.IsEditable(false);

	return SNew(SDockTab)
		.Label(INVTEXT("Dialogue Graph"))
		.TabRole(PanelTab)
		[
			GraphEditor
		];
}

TSharedRef<SDockTab> FYarnDialogueAssetEditorToolkit::SpawnLogTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		.OnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FYarnDialogueAssetEditorToolkit::OnTabClosed))
		[
			SAssignNew(LogWindowWidget, SDialogueLogWindow)
		];
}

void FYarnDialogueAssetEditorToolkit::OnTabClosed(TSharedRef<SDockTab> ClosedTab)
{
	if (EditorWidget.IsValid())
	{
		EditorWidget.Reset();
		UE_LOG(LogTemp, Log, TEXT("EditorWidget has been reset on tab close"));
	}

	GEditor->UnregisterForUndo(this);
}

void FYarnDialogueAssetEditorToolkit::SaveAsset_Execute()
{
	if (YarnAsset.IsValid())
	{
		YarnAsset->Modify();
		YarnAsset->MarkPackageDirty();
		YarnAsset->GetPackage()->MarkPackageDirty();

		EditorWidget->RebuildUI();

		bool bSuccessfulValidation = FYarnDialogueEditorHelpers::TestYarnAssetValidity(YarnAsset.Get());

		if (bSuccessfulValidation)
		{
			DialogueGraph->Initialize(YarnAsset.Get());
		}
		UE_LOG(LogYarnDialogueEditor, Log, TEXT("%s saved."), *YarnAsset->GetName());

		const TArray<UPackage*> PackagesToSave = {YarnAsset->GetPackage()};
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	}
}

void FYarnDialogueAssetEditorToolkit::PostUndo(bool bSuccess)
{
	if (EditorWidget.IsValid())
	{
		EditorWidget->RebuildUI();
	}
}

void FYarnDialogueAssetEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

FName FYarnDialogueAssetEditorToolkit::GetToolkitFName() const
{
	return FName("YarnDialogueAssetEditor");
}

FText FYarnDialogueAssetEditorToolkit::GetBaseToolkitName() const
{
	return INVTEXT("Yarn Dialogue Editor");
}

FString FYarnDialogueAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("YarnDialogueEditor");
}

FLinearColor FYarnDialogueAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
