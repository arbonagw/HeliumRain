
#include "../../Flare.h"

#include "FlareSimulatedSpacecraftNavigationSystem.h"

#define LOCTEXT_NAMESPACE "FlareSimulatedSpacecraftNavigationSystem"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSpacecraftNavigationSystem::UFlareSimulatedSpacecraftNavigationSystem(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, Spacecraft(NULL)
{

}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/


void UFlareSimulatedSpacecraftNavigationSystem::Initialize(UFlareSimulatedSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData)
{
	Spacecraft = OwnerSpacecraft;
	Description = Spacecraft->GetDescription();
	Data = OwnerData;
}


/*----------------------------------------------------
	System API
----------------------------------------------------*/

bool UFlareSimulatedSpacecraftNavigationSystem::IsAutoPilot()
{
	// No autopilot in simulation
	return false;
}

bool UFlareSimulatedSpacecraftNavigationSystem::IsDocked()
{
	return (Data->DockedTo != NAME_None);
}

bool UFlareSimulatedSpacecraftNavigationSystem::Undock()
{
	if (IsDocked())
	{
		Data->DockedTo = NAME_None;
		Data->DockedAt = -1;
		return true;
	}
	else
	{
		return false;
	}
}

FFlareShipCommandData UFlareSimulatedSpacecraftNavigationSystem::GetCurrentCommand()
{
	// No commands in simulation
	FFlareShipCommandData CurrentCommand;
	CurrentCommand.Type = EFlareCommandDataType::CDT_None;
	return CurrentCommand;
}

#undef LOCTEXT_NAMESPACE
