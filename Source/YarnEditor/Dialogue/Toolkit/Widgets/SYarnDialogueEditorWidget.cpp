#include "SYarnDialogueEditorWidget.h"

#include "SDialogueBranchWidget.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

void SYarnDialogueEditorWidget::Construct(const FArguments& InArgs)
{
	Asset = InArgs._YarnAsset;

	ChildSlot
	[
		SNew(SScrollBox).Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SAssignNew(BranchesBox, SVerticalBox)
			]

			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.MaxHeight(25.f)
			.MinHeight(25.f)
			.Padding(10, 0, 10, 50)
			[
				SNew(SButton)
				.Text(FText::FromString("Add Branch"))
				.OnClicked_Lambda([this]()
				{
					OnAddBranch();
					return FReply::Handled();
				})
			]
		]
	];

	RebuildUI();
}

void SYarnDialogueEditorWidget::RebuildUI()
{
	BranchesBox->ClearChildren();

	for (const FDialogueBranch& Branch : Asset->Branches)
	{
		BranchesBox->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SDialogueBranchWidget)
				.BranchID(Branch.BranchID)
				.Asset(Asset)
				[
					Branch.BranchID != FName(UYarnDialogueAsset::GetMainBranchID())
						? SNew(SButton)
						.OnClicked_Lambda([this, BranchID = Branch.BranchID]()
						{
							OnRemoveBranch(BranchID);
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.WidthOverride(16.f)
							.HeightOverride(16.f)
							.VAlign(VAlign_Top)
							.HAlign(HAlign_Center)
							[
								SNew(SImage)
								.Image(FSlateIcon(FName("EditorStyle"), "MyBlueprint.DeleteEntry").GetIcon())
							]
						]
						: SNullWidget::NullWidget
				]
			]
		];
	}
}


void SYarnDialogueEditorWidget::OnAddBranch()
{
	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "AddBranch", "Add Dialogue Branch"));
	Asset->Modify();

	FName NewID;
	int32 Suffix = 1;
	do
	{
		NewID = FName(*FString::Printf(TEXT("Branch_%d"), Suffix++));
	}
	while (Asset->GetBranchByID(NewID) != nullptr);
	Asset->Branches.Add(FDialogueBranch(NewID, NewID.ToString()));
	Asset->MarkPackageDirty();
	RebuildUI();
}

void SYarnDialogueEditorWidget::OnRemoveBranch(FName BranchID)
{
	if (BranchID == FName(UYarnDialogueAsset::GetMainBranchID()))
	{
		return;
	}

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "RemoveBranch", "Remove Dialogue Branch"));
	Asset->Modify();

	Asset->Branches.RemoveAll([&](const FDialogueBranch& Branch)
	{
		return Branch.BranchID == BranchID;
	});

	Asset->MarkPackageDirty();
	RebuildUI();
}
