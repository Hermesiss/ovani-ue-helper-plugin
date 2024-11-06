// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "MetasoundSource.h"


class FOvaniSoundModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterMenus();

	static void ModifyMetaSoundAsset(UMetaSoundSource* MetaSoundAsset, TArray<TObjectPtr<USoundBase>> Sounds);
	static UMetaSoundSource* DuplicateMetaSoundAsset(const FString& TemplatePath, const FString& NewAssetName,
	                                                 const FString& NewAssetPath);
};
