#pragma once

#include "CoreMinimal.h"

enum class EBertaProjectSetupStatus : uint8 { Match, NeedsChange, PluginMissing, PendingRestart, Unavailable, Failed };

struct FBertaProjectSetupItem
{
	FString Name;
	FString CurrentValue;
	FString ExpectedValue;
	FString Detail;
	EBertaProjectSetupStatus Status = EBertaProjectSetupStatus::Unavailable;
	bool bRequiresRestart = false;
};

struct FBertaProjectSetupReport
{
	TArray<FBertaProjectSetupItem> Items;
	int32 Count(EBertaProjectSetupStatus Status) const;
	bool RequiresRestart() const;
};

/** Explicit, allowlisted application of BertaDevKit's Project Setup V1 preferences. */
class FBertaProjectSetup
{
public:
	static FBertaProjectSetupReport Audit();
	static void AuditAndReport();
	static void ApplyWithConfirmation();
};
