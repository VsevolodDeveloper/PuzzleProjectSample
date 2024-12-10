// Copyright Vsevolod Khlebnikov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
struct FEdgeMoveset;

DECLARE_LOG_CATEGORY_EXTERN(LogEdgeEditor, Log, All);

/**
 * Editor module, to simplify work with the puzzle and adding movesets of puzzle edges
 */
class FEdgeEditorModule : public IModuleInterface
{
private:
	/** Called on spawn plugin tab */
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** Plugin commands */
	TSharedPtr<class FUICommandList> PluginCommands;

	/** Name of selected edge */
	FName EdgeName;

	/** Register menus */
	void RegisterMenus();

	/** Add edge moveset */
	FReply AddEdgeMoveset();

	/** Add moveset element */
	void AddEdgeMovesetElement(UObject* Object, AActor*& PuzzleActor, FEdgeMoveset& EdgeMoveset, int64& FirstPieceId);

	/** Set edge name */
	void SetEdgeName(const FText& Text);

public:
	// Called when "add moveset" button clicked */
	FOnClicked OnAddMovesetButtonClicked;

	// Called when edge name changed */
	FOnTextChanged OnEdgeNameChanged;

	//~ Begin IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface implementation

	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
};
