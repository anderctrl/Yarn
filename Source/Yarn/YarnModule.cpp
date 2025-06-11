#pragma once

#include "YarnModule.h"

#include "ISettingsModule.h"
#include "YarnSettings.h"

IMPLEMENT_MODULE(FYarnModule, Yarn)
DEFINE_LOG_CATEGORY(LogYarnRuntime);

void FYarnModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", "Plugins", "Yarn",
			FText::FromString("Yarn"),
			FText::FromString("Settings for the Yarn plugin."),
			GetMutableDefault<UYarnSettings>()
		);
	}
}

void FYarnModule::ShutdownModule()
{
}
