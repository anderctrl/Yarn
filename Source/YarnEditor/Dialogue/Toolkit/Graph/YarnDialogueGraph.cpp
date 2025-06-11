#include "YarnDialogueGraph.h"
#include "YarnDialogueGraphNode_Branch.h"
#include "YarnDialogueGraphNode_Line.h"

void UYarnDialogueGraph::Initialize(UYarnDialogueAsset* InDialogueAsset)
{
    DialogueAsset = InDialogueAsset;
    Nodes.Reset();

    if (!DialogueAsset)
        return;

    constexpr int32 BranchVerticalSpacing = 400;
    constexpr int32 BaseLineHorizontalSpacing = 100;
    constexpr int32 ChoiceHorizontalOffset = 300;
    constexpr int32 ChoiceVerticalSpacing = 150;
    constexpr float BaseNodeWidth = 100.f;
    constexpr float WidthPerCharacter = 6.f;
    constexpr float NodeHeight = 80.f;
    constexpr float MaxNodeWidthForSpacing = 250.f;

    BranchIDToNode.Reset();
    LineIDToNode.Reset();

    CreateBranches(BranchVerticalSpacing);
    CreateLinesAndChoices(BaseLineHorizontalSpacing, ChoiceHorizontalOffset, ChoiceVerticalSpacing, BaseNodeWidth, WidthPerCharacter, MaxNodeWidthForSpacing);
    ConnectChoicesToNextMainLine();
    ConnectJumpToBranches();
    CenterNodesVertically(NodeHeight);
}

void UYarnDialogueGraph::CreateBranches(int32 BranchVerticalSpacing)
{
    int32 BranchIndex = 0;

    for (const FDialogueBranch& Branch : DialogueAsset->Branches)
    {
        UYarnDialogueGraphNode_Branch* BranchNode = NewObject<UYarnDialogueGraphNode_Branch>(this);
        BranchNode->BranchID = Branch.BranchID;
        BranchNode->DialogueAsset = DialogueAsset;

        BranchNode->SetFlags(RF_Transactional);
        BranchNode->CreateNewGuid();
        BranchNode->PostPlacedNewNode();
        BranchNode->AllocateDefaultPins();

        BranchNode->NodePosX = 0;
        BranchNode->NodePosY = BranchIndex * BranchVerticalSpacing;

        AddNode(BranchNode);
        BranchIDToNode.Add(Branch.BranchID, BranchNode);

        BranchIndex++;
    }
}

void UYarnDialogueGraph::CreateLinesAndChoices(
    int32 BaseLineHorizontalSpacing,
    int32 ChoiceHorizontalOffset,
    int32 ChoiceVerticalSpacing,
    float BaseNodeWidth,
    float WidthPerCharacter,
    float MaxNodeWidthForSpacing)
{
    for (const FDialogueBranch& Branch : DialogueAsset->Branches)
    {
        int32 CurrentLineX = 300;
        int32 BranchY = BranchIDToNode[Branch.BranchID]->NodePosY;

        TArray<UYarnDialogueGraphNode_Line*> LineNodes;

        for (int32 i = 0; i < Branch.Lines.Num(); ++i)
        {
            const FDialogueLine& Line = Branch.Lines[i];

            UYarnDialogueGraphNode_Line* LineNode = NewObject<UYarnDialogueGraphNode_Line>(this);
            LineNode->ParentBranchID = Branch.BranchID;
            LineNode->DialogueAsset = DialogueAsset;
            LineNode->LineID = Line.LineID;

            LineNode->SetFlags(RF_Transactional);
            LineNode->CreateNewGuid();
            LineNode->PostPlacedNewNode();
            LineNode->AllocateDefaultPins();

            float LineNodeWidth = FMath::Min(BaseNodeWidth + (Line.Text.ToString().Len() * WidthPerCharacter), MaxNodeWidthForSpacing);

            LineNode->NodePosX = CurrentLineX;
            LineNode->NodePosY = BranchY;

            AddNode(LineNode);
            LineIDToNode.Add(Line.LineID, LineNode);
            LineNodes.Add(LineNode);

            if (i == 0)
            {
                BranchIDToNode[Branch.BranchID]->GetOutputPin()->MakeLinkTo(LineNode->GetInputPin());
            }
            else
            {
                const FDialogueLine& PrevLine = Branch.Lines[i - 1];
                if (PrevLine.PlayerChoiceIDs.Num() == 0)
                {
                    UYarnDialogueGraphNode_Line* PrevNode = LineNodes[i - 1];
                    PrevNode->GetOutputPin()->MakeLinkTo(LineNode->GetInputPin());
                }
            }

            float MaxChoiceWidth = 0.f;
            if (Line.PlayerChoiceIDs.Num() > 0)
            {
                float ChoiceStartY = BranchY - ((Line.PlayerChoiceIDs.Num() - 1) * ChoiceVerticalSpacing) / 2.f;

                for (int32 ChoiceIndex = 0; ChoiceIndex < Line.PlayerChoiceIDs.Num(); ++ChoiceIndex)
                {
                    const FName& ChoiceID = Line.PlayerChoiceIDs[ChoiceIndex];
                    const FDialogueLine* ChoiceLine = DialogueAsset->GetChoiceByID(ChoiceID);
                    if (!ChoiceLine)
                        continue;

                    UYarnDialogueGraphNode_Line* ChoiceNode = NewObject<UYarnDialogueGraphNode_Line>(this);
                    ChoiceNode->ParentBranchID = Branch.BranchID;
                    ChoiceNode->DialogueAsset = DialogueAsset;
                    ChoiceNode->LineID = ChoiceLine->LineID;

                    ChoiceNode->SetFlags(RF_Transactional);
                    ChoiceNode->CreateNewGuid();
                    ChoiceNode->PostPlacedNewNode();
                    ChoiceNode->AllocateDefaultPins();

                    float ChoiceNodeWidth = FMath::Min(BaseNodeWidth + (ChoiceLine->Text.ToString().Len() * WidthPerCharacter), MaxNodeWidthForSpacing);

                    ChoiceNode->NodePosX = CurrentLineX + ChoiceHorizontalOffset;
                    ChoiceNode->NodePosY = ChoiceStartY + ChoiceIndex * ChoiceVerticalSpacing;

                    AddNode(ChoiceNode);
                    LineIDToNode.Add(ChoiceLine->LineID, ChoiceNode);

                    LineNode->GetOutputPin()->MakeLinkTo(ChoiceNode->GetInputPin());

                    if (ChoiceNodeWidth > MaxChoiceWidth)
                        MaxChoiceWidth = ChoiceNodeWidth;
                }
            }

            float TotalNodeWidth = LineNodeWidth;
            if (Line.PlayerChoiceIDs.Num() > 0)
            {
                TotalNodeWidth = FMath::Max(LineNodeWidth, MaxChoiceWidth + ChoiceHorizontalOffset);
            }
            CurrentLineX += TotalNodeWidth + BaseLineHorizontalSpacing;
        }
    }
}

void UYarnDialogueGraph::ConnectChoicesToNextMainLine()
{
    for (const FDialogueBranch& Branch : DialogueAsset->Branches)
    {
        const TArray<FDialogueLine>& Lines = Branch.Lines;

        for (int32 i = 0; i < Lines.Num() - 1; ++i)
        {
            const FDialogueLine& Line = Lines[i];
            const FDialogueLine& NextLine = Lines[i + 1];

            if (Line.PlayerChoiceIDs.Num() == 0)
                continue;

            for (const FName& ChoiceID : Line.PlayerChoiceIDs)
            {
                const FDialogueLine* Choice = DialogueAsset->GetChoiceByID(ChoiceID);
                if (!Choice)
                    continue;

                if (Choice->JumpToBranchID == NAME_None)
                {
                    UYarnDialogueGraphNode_Line* ChoiceNode = LineIDToNode.FindRef(Choice->LineID);
                    UYarnDialogueGraphNode_Line* NextLineNode = LineIDToNode.FindRef(NextLine.LineID);
                    if (ChoiceNode && NextLineNode)
                    {
                        ChoiceNode->GetOutputPin()->MakeLinkTo(NextLineNode->GetInputPin());
                    }
                }
            }
        }
    }
}

void UYarnDialogueGraph::ConnectJumpToBranches()
{
    for (auto& Pair : LineIDToNode)
    {
        const FName LineID = Pair.Key;
        UYarnDialogueGraphNode_Line* LineNode = Pair.Value;

        const FDialogueLine* Line = DialogueAsset->GetLineByID(LineID);
        const FDialogueLine* Choice = DialogueAsset->GetChoiceByID(LineID);

        const FDialogueLine* ActualLine = Line ? Line : Choice;

        if (ActualLine && ActualLine->JumpToBranchID != NAME_None)
        {
            if (UYarnDialogueGraphNode_Branch* TargetBranchNode = BranchIDToNode.FindRef(ActualLine->JumpToBranchID))
            {
                LineNode->GetOutputPin()->MakeLinkTo(TargetBranchNode->GetInputPin());
            }
        }
    }
}

void UYarnDialogueGraph::CenterNodesVertically(float NodeHeight)
{
    for (auto& Pair : LineIDToNode)
    {
        UYarnDialogueGraphNode_Line* Node = Pair.Value;
        Node->NodePosY += NodeHeight * 0.5f;
    }

    for (auto& Pair : BranchIDToNode)
    {
        UYarnDialogueGraphNode_Branch* Node = Pair.Value;
        Node->NodePosY += NodeHeight * 0.5f;
    }
}
