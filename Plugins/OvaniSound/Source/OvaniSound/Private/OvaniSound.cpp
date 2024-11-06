// Copyright Epic Games, Inc. All Rights Reserved.

#include "OvaniSound.h"

#include "ContentBrowserMenuContexts.h"
#include "EditorAssetLibrary.h"

#define LOCTEXT_NAMESPACE "FOvaniSoundModule"

void FOvaniSoundModule::StartupModule()
{
	if (!IsRunningCommandlet() && !IsRunningGame() && FSlateApplication::IsInitialized())
	{
		UToolMenus::Get()->RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOvaniSoundModule::RegisterMenus));
	}
}

void FOvaniSoundModule::ShutdownModule()
{
	UToolMenus::Get()->UnRegisterStartupCallback(this);
}

void FOvaniSoundModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu");
		FToolMenuSection& Section = Menu->AddSection("Ovani sound", LOCTEXT("OvaniSound", "Ovani Sound"),
		                                             FToolMenuInsert("PathViewFolderOptions",
		                                                             EToolMenuInsertType::After));
		Section.AddMenuEntry(
			"Create Sound Source",
			LOCTEXT("CreateSoundSourceTabTitle", "Create Sound Source"),
			LOCTEXT("CreateSoundSourceTooltipText", "Create sound source from audio files in this folder."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Task.PlaySound.Icon"),
			FToolMenuExecuteAction::CreateLambda([this](const FToolMenuContext& InContext)
			{
				if (const UContentBrowserFolderContext* Context = InContext.FindContext<UContentBrowserFolderContext>())
				{
					const TArray<FString>& SelectedPaths = Context->GetSelectedPackagePaths();

					FString FormattedSelectedPaths;
					for (int32 i = 0; i < SelectedPaths.Num(); ++i)
					{
						TObjectPtr<USoundBase> MainSound = nullptr;
						TObjectPtr<USoundBase> In1Sound = nullptr;
						TObjectPtr<USoundBase> In2Sound = nullptr;
						TObjectPtr<USoundBase> Cut30Sound = nullptr;
						TObjectPtr<USoundBase> Cut60Sound = nullptr;

						TArray<FString> Files = UEditorAssetLibrary::ListAssets(SelectedPaths[i], false);
						
						for (int32 j = 0; j < Files.Num(); ++j)
						{
							const auto Sound = UEditorAssetLibrary::LoadAsset(FPackageName::ObjectPathToPackageName(Files[j]));
							if (Sound)
							{
								const auto WaveAsset = Cast<USoundWave>(Sound);
								if (!WaveAsset)
								{
									continue;
								}
								
								if (Sound->GetName().Contains("Main"))
								{
									MainSound = WaveAsset;
								}
								else if (Sound->GetName().Contains("Intensity_1"))
								{
									In1Sound = WaveAsset;
								}
								else if (Sound->GetName().Contains("Intensity_2"))
								{
									In2Sound = WaveAsset;
								}
								else if (Sound->GetName().Contains("Cut_30"))
								{
									Cut30Sound = WaveAsset;
								}
								else if (Sound->GetName().Contains("Cut_60"))
								{
									Cut60Sound = WaveAsset;
								}
							}
						}

						if (!MainSound || !In1Sound || !In2Sound || !Cut30Sound || !Cut60Sound)
						{
							UE_LOG(LogTemp, Warning, TEXT("Not all sounds found"));
							continue;
						}

						const auto SoundArray = TArray{In1Sound, In2Sound, MainSound, Cut30Sound, Cut60Sound};

						auto Template = FPackageName::ObjectPathToPackageName((FString("/Game/OvaniSound/MetaSounds/TEMPLATE_SONG_SRC.TEMPLATE_SONG_SRC")));

						auto FolderName = FPaths::GetBaseFilename(SelectedPaths[i]);
						auto NewAssetName = FString::Printf(TEXT("%s_SongSrc"), *FolderName);
						const auto MetaSound = DuplicateMetaSoundAsset(
							Template, NewAssetName, SelectedPaths[i]);
						if (MetaSound)
						{
							ModifyMetaSoundAsset(MetaSound, SoundArray);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("Failed to create sound source"));
						}
						FormattedSelectedPaths.Append(SelectedPaths[i]);
						if (i < SelectedPaths.Num() - 1)
						{
							FormattedSelectedPaths.Append(LINE_TERMINATOR);
						}
					}
				}
			})
		);
	}
}

void FOvaniSoundModule::ModifyMetaSoundAsset(UMetaSoundSource* MetaSoundAsset, TArray<TObjectPtr<USoundBase>> Sounds)
{
	if (!MetaSoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("MetaSoundAsset is null"));
		return;
	}

	int i = 0;

	MetaSoundAsset->Modify();
	const auto Graph = MetaSoundAsset->GetGraph();
	if (!Graph)
	{
		UE_LOG(LogTemp, Warning, TEXT("Graph is null"));
		return;
	}


	auto Nodes = Graph->Nodes;
	Graph->Modify();
	for (UEdGraphNode* EdGraphNode : Nodes)
	{
		const auto WaveAssetPin = EdGraphNode->FindPin(FName("Wave Asset"));
		if (WaveAssetPin)
		{
			EdGraphNode->Modify();
			WaveAssetPin->Modify();
			WaveAssetPin->LinkedTo.Empty();
			WaveAssetPin->GetSchema()->TrySetDefaultObject(*WaveAssetPin, Sounds[i]);
			EdGraphNode->PostEditChange();

			i++;
		}
	}
	Graph->PostEditChange();

	MetaSoundAsset->PostEditChange();	
}

UMetaSoundSource* FOvaniSoundModule::DuplicateMetaSoundAsset(const FString& TemplatePath, const FString& NewAssetName,
												const FString& NewAssetPath)
{

	const bool TemplateExists = UEditorAssetLibrary::DoesAssetExist(TemplatePath);
	if (!TemplateExists)
	{
		UE_LOG(LogTemp, Error, TEXT("Template asset not found: %s"), *TemplatePath);
		return nullptr;
	}
	
	const FString NewAssetFullPath = FPackageName::ObjectPathToPackageName(NewAssetPath + "/" + NewAssetName);

	const auto NewMetaSound = UEditorAssetLibrary::DuplicateAsset(TemplatePath, NewAssetFullPath);
	if (!NewMetaSound)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to duplicate asset: %s"), *NewAssetFullPath);
		return nullptr;
	}

	return Cast<UMetaSoundSource>(NewMetaSound);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOvaniSoundModule, OvaniSound)
