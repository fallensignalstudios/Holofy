#include "HolofyMaterialBuilder.h"

#include "Containers/Ticker.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "HolofyEditor"

class FHolofyEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FHolofyEditorModule::RegisterMenus));

		if (!IsRunningCommandlet() && !FApp::IsUnattended())
		{
			// Give the editor a moment to finish booting, then offer to generate if needed.
			CheckTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateRaw(this, &FHolofyEditorModule::CheckForMaterials), 3.0f);
		}
	}

	virtual void ShutdownModule() override
	{
		FTSTicker::GetCoreTicker().RemoveTicker(CheckTickerHandle);
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

private:
	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);

		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = Menu->FindOrAddSection("Holofy", LOCTEXT("HolofySection", "Holofy"));
		Section.AddMenuEntry(
			"HolofyGenerateMaterials",
			LOCTEXT("GenerateLabel", "Generate Hologram Materials"),
			LOCTEXT("GenerateTooltip", "(Re)build the Holofy master materials and preset instances in /Holofy/Materials."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FHolofyEditorModule::Generate)));
	}

	bool CheckForMaterials(float)
	{
		if (!HolofyMaterialBuilder::AreMaterialsGenerated())
		{
			FNotificationInfo Info(LOCTEXT("MissingTitle", "Holofy: hologram materials haven't been generated yet."));
			Info.SubText = LOCTEXT("MissingSub", "Generate them now? (You can also do it later from Tools > Holofy.)");
			Info.bFireAndForget = false;
			Info.ExpireDuration = 0.f;
			Info.ButtonDetails.Add(FNotificationButtonInfo(
				LOCTEXT("GenerateButton", "Generate"),
				LOCTEXT("GenerateButtonTip", "Build M_Holofy_* and MI_Holofy_* assets"),
				FSimpleDelegate::CreateRaw(this, &FHolofyEditorModule::OnPromptGenerate),
				SNotificationItem::CS_None));
			Info.ButtonDetails.Add(FNotificationButtonInfo(
				LOCTEXT("LaterButton", "Later"),
				FText::GetEmpty(),
				FSimpleDelegate::CreateRaw(this, &FHolofyEditorModule::DismissPrompt),
				SNotificationItem::CS_None));
			Prompt = FSlateNotificationManager::Get().AddNotification(Info);
		}
		return false; // one-shot
	}

	void OnPromptGenerate()
	{
		DismissPrompt();
		Generate();
	}

	void DismissPrompt()
	{
		if (TSharedPtr<SNotificationItem> Item = Prompt.Pin())
		{
			Item->SetExpireDuration(0.f);
			Item->ExpireAndFadeout();
		}
		Prompt.Reset();
	}

	void Generate()
	{
		const bool bOk = HolofyMaterialBuilder::GenerateAll();

		FNotificationInfo Info(bOk
			? LOCTEXT("GeneratedOk", "Holofy materials generated in /Holofy/Materials")
			: LOCTEXT("GeneratedFail", "Holofy material generation had problems - check the Output Log"));
		Info.ExpireDuration = 5.f;
		TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
		if (Item.IsValid())
		{
			Item->SetCompletionState(bOk ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}

	FTSTicker::FDelegateHandle CheckTickerHandle;
	TWeakPtr<SNotificationItem> Prompt;
};

IMPLEMENT_MODULE(FHolofyEditorModule, HolofyEditor)

#undef LOCTEXT_NAMESPACE
