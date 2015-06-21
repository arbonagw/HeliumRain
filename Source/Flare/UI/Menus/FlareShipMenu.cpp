
#include "../../Flare.h"
#include "FlareShipMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"
#include "../Widgets/FlarePartInfo.h"


#define LOCTEXT_NAMESPACE "FlareShipMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	TSharedPtr<SFlareButton> BackButton;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)

		// UI
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			SNew(SBorder)
			.Padding(FMargin(0))
			.BorderImage(&Theme.BackgroundBrush)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// Menu title
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage).Image(this, &SFlareShipMenu::GetIconBrush)
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SFlareShipMenu::GetTitleText)
							.TextStyle(&Theme.TitleFont)
						]
					]

					// Action box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ObjectActionMenu, SFlareTargetActions)
						.Player(PC)
						.NoInspect(true)
					]

					// Object name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(20))
					[
						SAssignNew(ObjectName, STextBlock)
						.TextStyle(&Theme.SubTitleFont)
					]

					// Object description
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SAssignNew(ObjectDescription, STextBlock)
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(600)
					]

					// Ship part characteristics
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SAssignNew(PartCharacteristicBox, SHorizontalBox)
					]

					// Ship customization panel
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ShipCustomizationBox, SVerticalBox)

						// Section title
						+ SVerticalBox::Slot()
						.Padding(FMargin(20))
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ShipParts", "COMPONENTS"))
							.TextStyle(&Theme.SubTitleFont)
						]

						// Engine group
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(EngineButton, SFlareButton)
								.ButtonStyle(FFlareStyleSet::Get(), "/Style/PartButtonMinimized")
								.OnClicked(this, &SFlareShipMenu::ShowEngines)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(RCSButton, SFlareButton)
								.ButtonStyle(FFlareStyleSet::Get(), "/Style/PartButtonMinimized")
								.OnClicked(this, &SFlareShipMenu::ShowRCSs)
							]

							// Weapon group
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(WeaponButtonBox, SHorizontalBox)
							]
						]

					]

					// Ship part customization panel
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ShipPartCustomizationBox, SVerticalBox)

						// Section title
						+ SVerticalBox::Slot()
						.Padding(FMargin(10))
						.AutoHeight()
						[
							SAssignNew(ShipPartPickerTitle, STextBlock)
							.Text(LOCTEXT("ShipParts", "AVAILABLE COMPONENTS"))
							.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
						]

						// Ship part picker
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(PartList, SListView< TSharedPtr<FInterfaceContainer> >)
							.ListItemsSource(&PartListDataShared)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SFlareShipMenu::GeneratePartInfo)
							.OnSelectionChanged(this, &SFlareShipMenu::OnPartPicked)
						]

						// Button box
						+ SVerticalBox::Slot()
						.Padding(FMargin(0, 10, 0, 0))
						.AutoHeight()
						[
							SAssignNew(BuyConfirmation, SFlareConfirmationBox)
							.ConfirmText(LOCTEXT("Confirm", "CONFIRM TRANSACTION"))
							.CancelText(LOCTEXT("BackTopShip", "BACK TO SHIP"))
							.OnConfirmed(this, &SFlareShipMenu::OnPartConfirmed)
							.OnCancelled(this, &SFlareShipMenu::OnPartCancelled)
						]
					]
				]
			]
		]

		// Dashboard button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SAssignNew(BackButton, SFlareButton)
			.ButtonStyle(FFlareStyleSet::Get(), "/Style/BackToDashboardButton")
			.OnClicked(this, &SFlareShipMenu::OnDashboardClicked)
		]
	];

	// Dashboard button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareShipMenu::Enter(IFlareSpacecraftInterface* Target, bool IsEditable)
{
	FLOG("SFlareShipMenu::Enter");
	SetEnabled(true);

	CanEdit = IsEditable;
	CurrentShipTarget = Target;
	CurrentShipData = Target->Save();
	LoadTargetShip();

	// Move the viewer to the right
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.1, 1.0);
	}

	SetVisibility(EVisibility::Visible);
}

void SFlareShipMenu::Exit()
{
	SetEnabled(false);
	ObjectActionMenu->Hide();
	PartListData.Empty();
	PartList->RequestListRefresh();
	SetVisibility(EVisibility::Hidden);
}

void SFlareShipMenu::LoadTargetShip()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

		// Get the description data
		const FFlareSpacecraftDescription* ShipDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(CurrentShipData->Identifier);
		if (ShipDesc)
		{
			ObjectName->SetText(FText::FromString(ShipDesc->Name.ToString()));
			ObjectDescription->SetText(ShipDesc->Description);
			PC->GetMenuPawn()->ShowShip(ShipDesc, CurrentShipData);
		}

		// Make the right box visible
		if (!CanEdit)
		{
			ObjectActionMenu->SetSpacecraft(CurrentShipTarget);
			ObjectActionMenu->Show();
		}
		ShipPartCustomizationBox->SetVisibility(EVisibility::Collapsed);
		PartCharacteristicBox->SetVisibility(EVisibility::Collapsed);
		ShipCustomizationBox->SetVisibility(EVisibility::Visible);

		// Add weapon slots buttons
		int32 WeaponCount = 0;
		TSharedPtr<SFlareButton> Temp;
		WeaponButtonBox->ClearChildren();
		FName OrbitalEngineIdentifier;
		FName RCSIdentifier;
		
		for (int32 i = 0; i < CurrentShipData->Components.Num(); i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(CurrentShipData->Components[i].ComponentIdentifier);
			if (ComponentDescription->Type == EFlarePartType::Weapon)
			{
				WeaponButtonBox->AddSlot()
					.AutoWidth()
					[
						SAssignNew(Temp, SFlareButton)
						.ButtonStyle(FFlareStyleSet::Get(), "/Style/PartButtonMinimized")
						.OnClicked(this, &SFlareShipMenu::ShowWeapons, TSharedPtr<int32>(new int32(WeaponCount)))
					];

				Temp->GetContainer()->SetContent(SNew(SFlarePartInfo)
					.IsOwned(true)
					.IsMinimized(true)
					.Description(Catalog->Get(CurrentShipData->Components[i].ComponentIdentifier)));
				WeaponCount++;
			} 
			else if (ComponentDescription->Type == EFlarePartType::RCS)
			{
				RCSIdentifier = ComponentDescription->Identifier;
			}
			else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				OrbitalEngineIdentifier = ComponentDescription->Identifier;
			}
		}

		WeaponButtonBox->SetVisibility(WeaponCount > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		
		// Add orbital engine button
		EngineButton->GetContainer()->SetContent(SNew(SFlarePartInfo)
			.IsOwned(true)
			.IsMinimized(true)
			.Description(Catalog->Get(OrbitalEngineIdentifier)));

		// Add RCS button
		RCSButton->GetContainer()->SetContent(SNew(SFlarePartInfo)
			.IsOwned(true)
			.IsMinimized(true)
			.Description(Catalog->Get(RCSIdentifier)));

	}

}

void SFlareShipMenu::LoadPart(FName InternalName)
{
	// Spawn the part
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get(InternalName);
		if (PartDesc)
		{
			// Show part
			ObjectName->SetText(PartDesc->Name);
			ObjectDescription->SetText(PartDesc->Description);
			PC->GetMenuPawn()->ShowPart(PartDesc);

			// Build info
			SFlarePartInfo::BuildInfoBlock(PartCharacteristicBox, PartDesc);
		}
	}

	// Make the right box visible
	ObjectActionMenu->Hide();
	ObjectDescription->SetVisibility(EVisibility::Visible);
	ShipPartCustomizationBox->SetVisibility(EVisibility::Visible);
	PartCharacteristicBox->SetVisibility(EVisibility::Visible);
	ShipCustomizationBox->SetVisibility(EVisibility::Collapsed);
}

void SFlareShipMenu::UpdatePartList(FFlareSpacecraftComponentDescription* SelectItem)
{
	ShipPartPickerTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartList->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartListDataShared.Empty();
	PartList->RequestListRefresh();

	if (CanEdit)
	{
		int32 Index = PartListData.Find(SelectItem);
		ShipPartIndex = Index;
		CurrentPartIndex = Index;
		CurrentEquippedPartIndex = Index;

		// Copy items
		for (int32 i = 0; i < PartListData.Num(); i++)
		{
			PartListDataShared.AddUnique(FInterfaceContainer::New(PartListData[i]));
		}

		// Update list
		PartList->RequestListRefresh();
		PartList->SetSelection(PartListDataShared[Index]);
	}

	LoadPart(SelectItem->Identifier);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareShipMenu::GetIconBrush() const
{
	if (CanEdit)
	{
		return AFlareHUD::GetMenuIcon(EFlareMenu::MENU_ShipConfig);
	}
	else
	{
		return AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Ship);
	}
}

FText SFlareShipMenu::GetTitleText() const
{
	if (CanEdit)
	{
		return LOCTEXT("ShipConfigMenuTitle", "SHIP UPGRADE");
	}
	else
	{
		return LOCTEXT("ShipMenuTitle", "SHIP");
	}
}

void SFlareShipMenu::ShowRCSs()
{
	PartListData.Empty();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(CurrentShipTarget);
		PC->GetGame()->GetShipPartsCatalog()->GetRCSList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetRCSDescription());
	}
}

void SFlareShipMenu::ShowEngines()
{
	PartListData.Empty();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(CurrentShipTarget);
		PC->GetGame()->GetShipPartsCatalog()->GetEngineList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetOrbitalEngineDescription());
	}
}

void SFlareShipMenu::ShowWeapons(TSharedPtr<int32> WeaponIndex)
{
	PartListData.Empty();
	CurrentWeaponIndex = *WeaponIndex;

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(CurrentShipTarget);
		PC->GetGame()->GetShipPartsCatalog()->GetWeaponList(PartListData, Ship->GetDescription()->Size);
		UpdatePartList(Ship->GetWeaponsSystem()->GetWeaponDescription(CurrentWeaponIndex));
	}
}

TSharedRef<ITableRow> SFlareShipMenu::GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<SFlarePartInfo> Temp;
	TSharedPtr<SFlareListItem> TempWidget;

	// Create the row
	TSharedRef<ITableRow> res = SAssignNew(TempWidget, SFlareListItem, OwnerTable)
		.ButtonStyle(&FFlareStyleSet::Get(), "/Style/PartButton")
		.Content()
		[
			SAssignNew(Temp, SFlarePartInfo)
			.Description(Item->PartDescription)
			.ShowOwnershipInfo(true)
		];

	// Update the selection to force select the first item
	int32 Index = PartListData.Find(Item->PartDescription);
	if (Index == CurrentEquippedPartIndex)
	{
		Temp->SetOwned(true);
		TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(TempWidget);
		if (ItemWidget.IsValid())
		{
			ItemWidget->SetSelected(true);
			PreviousSelection = ItemWidget;
		}
	}

	return res;
}

void SFlareShipMenu::OnPartPicked(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	int32 Index = PartListData.Find(Item->PartDescription);
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	if (PC && Item->PartDescription && Index != CurrentPartIndex)
	{
		AFlareMenuPawn* Viewer = PC->GetMenuPawn();

		// Load the part
		if (Viewer)
		{
			Viewer->SetSlideDirection(Index > CurrentPartIndex);
			LoadPart(Item->PartDescription->Identifier);
		}
		CurrentPartIndex = Index;

		// Show the confirmation dialog
		if (CurrentPartIndex != ShipPartIndex)
		{
			BuyConfirmation->Show(PartListData[CurrentEquippedPartIndex]->Cost - Item->PartDescription->Cost);
		}
		else
		{
			BuyConfirmation->Hide();
		}
	}

	// De-select old
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}

	// Re-select new
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(PartList->WidgetFromItem(Item));
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}

void SFlareShipMenu::OnPartConfirmed()
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		CurrentEquippedPartIndex = CurrentPartIndex;

		// Edit the correct save data property
		FFlareSpacecraftComponentDescription* PartDesc = PartListData[CurrentPartIndex];
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		int32 WeaponCount = 0;
		for (int32 i = 0; i < CurrentShipData->Components.Num(); i++)
		{
			bool UpdatePart = false;
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(CurrentShipData->Components[i].ComponentIdentifier);

			if (ComponentDescription->Type == PartDesc->Type)
			{
				if (ComponentDescription->Type == EFlarePartType::Weapon)
				{
					if (WeaponCount == CurrentWeaponIndex)
					{
						UpdatePart = true;
					}
					WeaponCount ++;
				}
				else
				{
					UpdatePart = true;

				}
			}

			if (UpdatePart)
			{
				CurrentShipData->Components[i].ComponentIdentifier = PartDesc->Identifier;
			}
		}
		
		// Update the world ship if it exists
		if (CurrentShipTarget)
		{
			CurrentShipTarget->Load(*CurrentShipData);
		}

		// Get back to the ship config
		BuyConfirmation->Hide();
		LoadTargetShip();
	}
}

void SFlareShipMenu::OnPartCancelled()
{
	BuyConfirmation->Hide();
	LoadTargetShip();
}

void SFlareShipMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}

#undef LOCTEXT_NAMESPACE
