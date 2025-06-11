#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLogMessageReceived, const FString&, ELogVerbosity::Type, FName);

class YARNEDITOR_API FDialogueRedirectLog : public FOutputDevice
{
public:
	static FDialogueRedirectLog& Get();

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	const TArray<FString>& GetLogs() const { return Logs; }

	FOnLogMessageReceived OnLogMessageReceived;

private:
	FDialogueRedirectLog() = default;

	TArray<FString> Logs;
};
