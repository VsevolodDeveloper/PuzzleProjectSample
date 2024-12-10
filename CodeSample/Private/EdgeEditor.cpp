// Copyright Vsevolod Khlebnikov. All Rights Reserved.

#include "EdgeEditor.h"

#include "EdgeEditorStyle.h"
#include "EdgeEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Kismet/GameplayStatics.h"
#include "Puzzles/SIPuzzleBase.h"
#include "Components/SIPuzzlePieceComponent.h"
#include "Engine/Selection.h"

static const FName EdgeEditorTabName("EdgeEditor");

#define LOCTEXT_NAMESPACE "FEdgeEditorModule"
DEFINE_LOG_CATEGORY(LogEdgeEditor);

void FEdgeEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FEdgeEditorStyle::Initialize();
	FEdgeEditorStyle::ReloadTextures();
	FEdgeEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FEdgeEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FEdgeEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEdgeEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(EdgeEditorTabName, FOnSpawnTab::CreateRaw(this, &FEdgeEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FEdgeEditorTabTitle", "EdgeEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FEdgeEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FEdgeEditorStyle::Shutdown();
	FEdgeEditorCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EdgeEditorTabName);
}

TSharedRef<SDockTab> FEdgeEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const FText ButtonText = FText::FromName(FName("Add Moveset"));
	OnAddMovesetButtonClicked.BindRaw(this, &FEdgeEditorModule::AddEdgeMoveset);
	OnEdgeNameChanged.BindRaw(this, &FEdgeEditorModule::SetEdgeName);

	UE_LOG(LogEdgeEditor, Log, TEXT("Plugin tab created"));

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox) 
			+SVerticalBox::Slot()
			.AutoHeight()
			[
		       SNew(SButton)
				.OnClicked(OnAddMovesetButtonClicked)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(ButtonText)
				] 
			] 
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SEditableTextBox)
				 .OnTextChanged(OnEdgeNameChanged)
				 .HintText(LOCTEXT("EditableTextBox", "Edge Name"))
			]
		];
}

void FEdgeEditorModule::SetEdgeName(const FText& Text)
{
	const FString EdgeString = Text.ToString();
	EdgeName = FName(*EdgeString);
}

FReply FEdgeEditorModule::AddEdgeMoveset()
{
	USelection* ComponentsSelection = GEditor->GetSelectedComponents();
	TArray<UObject*> Objects;
	ComponentsSelection->GetSelectedObjects(Objects);

	AActor* PuzzleActor = nullptr;
	FEdgeMoveset EdgeMoveset = FEdgeMoveset();
	EdgeMoveset.Order.Empty();
	int64 StartElementID = -1;

	for (UObject* Object : Objects)
	{
		AddEdgeMovesetElement(Object, PuzzleActor, EdgeMoveset, StartElementID);
	}

	if (StartElementID == -1 || !PuzzleActor)
	{
		return FReply::Handled();
		UE_LOG(LogEdgeEditor, Warning, TEXT("PuzzleActor or selected slots not found"));
	}
	else
	{
		EdgeMoveset.Order.Add(StartElementID);
	}

	ASIPuzzleBase* Puzzle = Cast<ASIPuzzleBase>(PuzzleActor);
	USIPuzzleEdgeComponent* Edge = Puzzle->GetEdgeByName(EdgeName);

	if (!Edge)
	{
		return FReply::Handled();
		UE_LOG(LogEdgeEditor, Warning, TEXT("Edge not found"));
	}

	Edge->GetBehavior()->AddMoveset(EdgeMoveset);

	const FString EdgeString = EdgeName.ToString();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, EdgeString);
	UE_LOG(LogEdgeEditor, Log, TEXT("Moveset added for %s Edge"), *EdgeString);

	return FReply::Handled();
}

void FEdgeEditorModule::AddEdgeMovesetElement(UObject* Object, AActor*& PuzzleActor, FEdgeMoveset& EdgeMoveset, int64& FirstPieceId)
{
	USIPuzzlePieceComponent* Piece = Cast<USIPuzzlePieceComponent>(Object);
	if (Piece)
	{
		PuzzleActor = Piece->GetOwner();
		if (Piece->IsSelectedInEditor())
		{
			EdgeMoveset.Order.Add(Piece->GetID());

			if (FirstPieceId == -1)
			{
				FirstPieceId = Piece->GetID();
			}

			const FString SlotString = FString::FromInt(Piece->GetID());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, SlotString);
			UE_LOG(LogEdgeEditor, Log, TEXT("Element with id: %s added to moveset"), *SlotString);
		}
	}
}

void FEdgeEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(EdgeEditorTabName);
}

void FEdgeEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FEdgeEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FEdgeEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEdgeEditorModule, EdgeEditor)