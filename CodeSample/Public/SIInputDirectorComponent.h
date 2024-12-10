// Copyright Vsevolod Khlebnikov. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/InputSettings.h"

#include "SIInputDirectorComponent.generated.h"

class ASIVisualPuzzleBase;

/**
 * Holding name of puzzle event and action
 */
USTRUCT(BlueprintType)
struct FPuzzleAction
{
	GENERATED_BODY()

	/** Name of puzzle action */
	UPROPERTY(EditAnywhere, Category = "Puzzle Action")
	FString ActionName;

	/** Edge Name of puzzle, where action supposed to be called from */
	UPROPERTY(EditAnywhere, Category = "Puzzle Action")
	FString EdgeName;

	/** Action for binding */
	UPROPERTY(EditAnywhere, Category = "Puzzle Action")
	FInputActionKeyMapping Action;
};

/**
 * Using as object to execute the action of receiver
 */
UCLASS()
class SOLVEIT_API USIActionExecuter : public UObject
{
	GENERATED_BODY()

private:
	/** Name of action */
	FString ActionName;

	/** Pointer to receiver */
	UPROPERTY(Transient)
	UActorComponent* Receiver;

public:
	/** Initialize executor */
	void Initialize(const FString& InActionName, UActorComponent* InReceiver);

	/** Execute current puzzle action */
	void ExecuteAction();
};

/**
 * Implemented ability to bind player inputs to puzzle component events, including the use of key combinations
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOLVEIT_API USIInputDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	/** Puzzle edge actions, such as edge turns */
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TArray<FPuzzleAction> PuzzleEdgeActions;

	/** Cached action executors */
	UPROPERTY(Transient)
	TArray<USIActionExecuter*> ActionExecutors;

	/** Should bind inputs for puzzle or not */
	UPROPERTY(EditAnywhere, Category = "Inputs")
	bool bBindInputs;

	/** Check if action is already exist */
	void RemoveDuplicateActions(FPuzzleAction& PuzzleAction);

	/** Bind puzzle action */
	void BindPuzzleAction(const ASIVisualPuzzleBase* Puzzle, FPuzzleAction& PuzzleAction, UInputComponent* InputComponent);

	/** Making unique action name */
	void SetActionName(const ASIVisualPuzzleBase* Puzzle, FPuzzleAction& PuzzleAction);

	/** Mark executor objects to destroy */
	void DestroyExecutors();

public:
	// Sets default values for this component's properties
	USIInputDirectorComponent();

	/** Initialize Component */
	UFUNCTION(BlueprintCallable, Category = "Input Director Component")
	void BindPuzzleActions(UInputComponent* InputComponent);
};
