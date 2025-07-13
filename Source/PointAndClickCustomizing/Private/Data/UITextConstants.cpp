// Copyright 2025 Devhanghae All Rights Reserved.


#include "Data/UITextConstants.h"

#define LOCTEXT_NAMESPACE "UITextConstants"

FText UUITextConstants::GetWaitingForPlayers_Title()
{
	return LOCTEXT("WaitingTitle", "MATCHING");
}

FText UUITextConstants::GetWaitingForPlayers_Description()
{
	return LOCTEXT("WaitingDesc", "Waiting for other players to get ready...");
}

FText UUITextConstants::GetConfirmReady_Title()
{
	return LOCTEXT("ConfirmReadyTitle", "CONFIRM");
}

FText UUITextConstants::GetConfirmReady_Description()
{
	return LOCTEXT("ConfirmReadyDesc", "Are you ready to start the game?");
}


FText UUITextConstants::GetHowToUse_Title()
{
	return LOCTEXT("HowToUseTitle", "HOW TO USE");
}

FText UUITextConstants::GetHowToUse_Description()
{
	return LOCTEXT("HowToUseDesc",
		"1. Press the GUI button to spawn a part.\n\n"
		"2. Right-click to cancel spawning.\n\n"
		"3. Move the spawned part close to a character bone.\n\n"
		"4. Left-click when snapped to attach the part to the character.\n\n"
		"5. Click an attached part to enter Focus mode.\n\n"
		"6. In Focus mode, press the Delete key to remove the part, or drag to rotate it.\n\n"
		"7. Right-click to exit Focus mode."
	);
}

#undef LOCTEXT_NAMESPACE