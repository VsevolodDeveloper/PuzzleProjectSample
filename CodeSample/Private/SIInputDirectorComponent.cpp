// Copyright Vsevolod Khlebnikov. All Rights Reserved.

#include "Components/SIInputDirectorComponent.h"
#include "Interfaces/SIInputReceiverInterface.h"

#include "Puzzles/SIVisualPuzzleBase.h"

USIInputDirectorComponent::USIInputDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bBindInputs = true;
}

void USIInputDirectorComponent::BindPuzzleActions(UInputComponent* InputComponent)
{
	ASIVisualPuzzleBase* Puzzle = Cast<ASIVisualPuzzleBase>(GetOwner());

	if (Puzzle && bBindInputs)
	{
		DestroyExecutors();
		ActionExecutors.Empty();

		for (FPuzzleAction& PuzzleAction : PuzzleEdgeActions)
		{
			BindPuzzleAction(Puzzle, PuzzleAction, InputComponent);
		}
	}
}

void USIInputDirectorComponent::DestroyExecutors()
{
	for (USIActionExecuter* Executor : ActionExecutors)
	{
		Executor->MarkPendingKill();
	}
}

void USIInputDirectorComponent::BindPuzzleAction(const ASIVisualPuzzleBase* Puzzle, FPuzzleAction& PuzzleAction, UInputComponent* InputComponent)
{
	SetActionName(Puzzle, PuzzleAction);
	RemoveDuplicateActions(PuzzleAction);
	UInputSettings::GetInputSettings()->AddActionMapping(PuzzleAction.Action, true);

	TArray<UActorComponent*> Components = Puzzle->GetComponentsByInterface(USIInputReceiverInterface::StaticClass());
	for (UActorComponent* Receiver : Components)
	{
		const ISIInputReceiverInterface* ReceiverInterface = Cast<ISIInputReceiverInterface>(Receiver);
		const FName ReceiverName = ReceiverInterface->Execute_GetReceiverName(Receiver);
		if (ReceiverName == FName(PuzzleAction.EdgeName))
		{
			USIActionExecuter* ActionExecutor = NewObject<USIActionExecuter>();
			ActionExecutor->Initialize(PuzzleAction.ActionName, Receiver);
			InputComponent->BindAction(PuzzleAction.Action.ActionName, IE_Pressed, ActionExecutor, &USIActionExecuter::ExecuteAction);
			ActionExecutors.Add(ActionExecutor);
		}
	}
}

void USIInputDirectorComponent::SetActionName(const ASIVisualPuzzleBase* Puzzle, FPuzzleAction& PuzzleAction)
{
	FString ActionName = Puzzle->GetPuzzleName().ToString();
	const FName ActionSubName = FName(FString("_").Append(PuzzleAction.ActionName));
	ActionSubName.AppendString(ActionName);
	PuzzleAction.Action.ActionName = FName(ActionName);
}

void USIInputDirectorComponent::RemoveDuplicateActions(FPuzzleAction& PuzzleAction)
{
	TArray<FInputActionKeyMapping> OutMappings;
	UInputSettings::GetInputSettings()->GetActionMappingByName(PuzzleAction.Action.ActionName, OutMappings);

	for (FInputActionKeyMapping& DuplicatedMapping : OutMappings)
	{
		if (DuplicatedMapping.ActionName == PuzzleAction.Action.ActionName)
		{
			PuzzleAction.Action.Key = DuplicatedMapping.Key;
			UInputSettings::GetInputSettings()->RemoveActionMapping(DuplicatedMapping);
		}
	}
}

void USIActionExecuter::Initialize(const FString& InActionName, UActorComponent* InReceiver)
{
	ActionName = InActionName;
	Receiver = InReceiver;
}

void USIActionExecuter::ExecuteAction()
{
	const ISIInputReceiverInterface* ReceiverInterface = Cast<ISIInputReceiverInterface>(Receiver);
	ReceiverInterface->Execute_ReceiveInput(Receiver, ActionName);
}
