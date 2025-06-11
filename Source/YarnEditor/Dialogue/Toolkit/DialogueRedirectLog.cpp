#include "DialogueRedirectLog.h"

FDialogueRedirectLog& FDialogueRedirectLog::Get()
{
	static FDialogueRedirectLog Instance;
	return Instance;
}

void FDialogueRedirectLog::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (Category != "LogYarnDialogueEditor")
	{
		return;
	}

	FString VerbosityName;
	switch (Verbosity)
	{
	case ELogVerbosity::Error:
		VerbosityName = TEXT("Error");
		break;
	case ELogVerbosity::Warning:
		VerbosityName = TEXT("Warning");
		break;
	case ELogVerbosity::Display:
		VerbosityName = TEXT("Display");
		break;
	default:
		VerbosityName = TEXT("Log");
		break;
	}

	const FString Formatted = FString::Printf(TEXT("%s: %s"), *VerbosityName, V);

	Logs.Add(Formatted);
	AsyncTask(ENamedThreads::GameThread, [this, Formatted, Verbosity, Category]()
	{
		OnLogMessageReceived.Broadcast(Formatted, Verbosity, Category);
	});
}
