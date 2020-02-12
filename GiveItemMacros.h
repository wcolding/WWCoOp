#pragma once
#include "DolphinHook.h"
#include "WorldFlags.h"

void GiveTelescope()
{
	DolphinWrite8(INV_OFFSET, WWItem::Telecope);
}

void GiveSail()
{

	DolphinWrite8(INV_OFFSET + 1, WWItem::Sail);
}

void GiveWindWaker()
{

	DolphinWrite8(INV_OFFSET + 2, WWItem::WW);
}

void GiveGrapplingHook()
{

	DolphinWrite8(INV_OFFSET + 3, WWItem::GrapplingHook);
}

void GiveSpoilsBag()
{

	DolphinWrite8(INV_OFFSET + 4, WWItem::SpoilsBag);
}

void GiveBoomerang()
{

	DolphinWrite8(INV_OFFSET + 5, WWItem::Boomerang);
}

void GiveDekuLeaf()
{

	DolphinWrite8(INV_OFFSET + 6, WWItem::DekuLeaf);
}

void GiveTingleTuner()
{

	DolphinWrite8(INV_OFFSET + 7, WWItem::TingleTuner);
}

void SetPictoBox(int camLvl)
{
	switch (camLvl)
	{
	case 0:
		DolphinWrite8(INV_OFFSET + 8, WWItem::NoItem);
		break;
	case 1:
		DolphinWrite8(INV_OFFSET + 8, WWItem::PictoBox1);
		break;
	case 2:
		DolphinWrite8(INV_OFFSET + 8, WWItem::PictoBox2);
		break;
	default:
		break;
	}
}

void GiveIronBoots()
{
	DolphinWrite8(INV_OFFSET + 9, WWItem::Boots);
}

void GiveMagicArmor()
{
	DolphinWrite8(INV_OFFSET + 10, WWItem::MagicArmor);
}

void GiveBaitBag()
{
	DolphinWrite8(INV_OFFSET + 11, WWItem::BaitBag);
}

void SetBow(int bowLvl)
{
	switch (bowLvl)
	{
	case 0:
		DolphinWrite8(INV_OFFSET + 12, WWItem::NoItem);
		DolphinWrite8(WWItemSlot::BowProgressionMask, 0);
		break;
	case 1:
		DolphinWrite8(INV_OFFSET + 12, WWItem::Bow1);
		DolphinWrite8(WWItemSlot::BowProgressionMask, 0b00000001);
		break;
	case 2:
		DolphinWrite8(INV_OFFSET + 12, WWItem::Bow2);
		DolphinWrite8(WWItemSlot::BowProgressionMask, 0b00000011);
		break;
	case 3:
		DolphinWrite8(INV_OFFSET + 12, WWItem::Bow3);
		DolphinWrite8(WWItemSlot::BowProgressionMask, 0b00000111);
		break;
	default:
		break;
	}
}

void GiveBombBag()
{
	DolphinWrite8(INV_OFFSET + 13, WWItem::Bombs);
}

void SetBottleSlot(int slot, WWBottleContents contents)
{
	switch (slot)
	{
	case 1:
		DolphinWrite8(INV_OFFSET + 14, contents);
		break;
	case 2:
		DolphinWrite8(INV_OFFSET + 15, contents);
		break;
	case 3:
		DolphinWrite8(INV_OFFSET + 16, contents);
		break;
	case 4:
		DolphinWrite8(INV_OFFSET + 17, contents);
		break;
	default:
		break;
	}
}

void GiveMailBag()
{
	DolphinWrite8(INV_OFFSET + 18, WWItem::MailBag);
}

void GiveHookshot()
{
	DolphinWrite8(INV_OFFSET + 19, WWItem::Hookshot);
}

void GiveSkullHammer()
{
	DolphinWrite8(INV_OFFSET + 20, WWItem::Hammer);
}

void SetSword(int swordLvl)
{
	switch (swordLvl)
	{
	case 0:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::NoItem);
		DolphinWrite8(WWItemSlot::SwordIconSlot, 0);
		break;
	case 1:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::Sword1);
		DolphinWrite8(WWItemSlot::SwordIconSlot, 0b00000001);
		break;
	case 2:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::Sword2);
		DolphinWrite8(WWItemSlot::SwordIconSlot, 0b00000011);
		break;
	case 3:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::Sword3);
		DolphinWrite8(WWItemSlot::SwordIconSlot, 0b00000111);
		break;
	case 4:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::Sword4);
		DolphinWrite8(WWItemSlot::SwordIconSlot, 0b00001111);
		break;
	default:
		break;
	}
}

void SetShield(int shieldLvl)
{
	switch (shieldLvl)
	{
	case 0:
		DolphinWrite8(WWItemSlot::ShieldSlot, WWItem::NoItem);
		DolphinWrite8(WWItemSlot::ShieldIconSlot, 0);
		break;
	case 1:
		DolphinWrite8(WWItemSlot::ShieldSlot, WWItem::Shield1);
		DolphinWrite8(WWItemSlot::ShieldIconSlot, 0b00000001);
		break;
	case 2:
		DolphinWrite8(WWItemSlot::ShieldSlot, WWItem::Shield2);
		DolphinWrite8(WWItemSlot::ShieldIconSlot, 0b00000011);
		break;
	default:
		break;
	}
}

void SetSongs(__int8 mask)
{
	DolphinWrite8(WWItemSlot::SongsSlot, mask);
}

void SetTriforce(__int8 mask)
{
	DolphinWrite8(WWItemSlot::TriforceSlot, mask);
}

void SetPearls(__int8 mask)
{
	DolphinWrite8(WWItemSlot::PearlSlot, mask);
	__int8 currentSetPearls = DolphinRead8(WWItemSlot::SetPearlsSlot);

	// Randomizer automatically sets these flags when you acquire a pearl
	// This will replicate that functionality for pearls received from another player
	if ((mask & WWPearlMask::Din) != 0)
		currentSetPearls |= DinsPearlPlaced.flag;
	if ((mask & WWPearlMask::Farore) != 0)
		currentSetPearls |= FaroresPearlPlaced.flag;
	if ((mask & WWPearlMask::Nayru) != 0)
		currentSetPearls |= NayrusPearlPlaced.flag;

	// If all three pearls are collected, set Tower of the Gods as raised
	if (mask == (WWPearlMask::Din | WWPearlMask::Farore | WWPearlMask::Nayru))
	{
		__int8 totgRaised = DolphinRead8(TotGRaised.address);
		totgRaised |= TotGRaised.flag;
		DolphinWrite8(TotGRaised.address, totgRaised);
	}

	DolphinWrite8(WWItemSlot::SetPearlsSlot, currentSetPearls);
}

void SetStatues(__int8 mask)
{
	DolphinWrite8(WWItemSlot::StatuesSlot, mask);
}

void SetWallet(__int8 walletLvl)
{
	DolphinWrite8(WWItemSlot::WalletSlot, walletLvl);
}

void UpgradeWallet()
{
	__int8 wallet = DolphinRead8(WWItemSlot::WalletSlot);
	if (wallet < 2)
		wallet += 1;
	DolphinWrite8(WWItemSlot::WalletSlot, wallet);
}

void UpgradeMagic()
{
	__int8 magic = DolphinRead8(WWItemSlot::MagicSlot);
	if (magic < 0x20)
		magic += 0x10;
	DolphinWrite8(WWItemSlot::MagicSlot, magic);
}

void UpgradeQuiver()
{
	__int8 quiver = DolphinRead8(WWItemSlot::BowMaxAmmo);
	switch (quiver)
	{
	case 0:
		quiver = 30;
		break;
	case 30:
		quiver = 60;
		break;
	case 60:
		quiver = 99;
		
		break;
	default:
		return;
		break;
	}
	DolphinWrite8(WWItemSlot::BowMaxAmmo, quiver);
}

void UpgradeBombBag()
{
	__int8 bombBag = DolphinRead8(WWItemSlot::BombsMaxAmmo);
	switch (bombBag)
	{
	case 0:
		bombBag = 30;
		break;
	case 30:
		bombBag = 60;
		break;
	case 60:
		bombBag = 99;

		break;
	default:
		return;
		break;
	}
	DolphinWrite8(WWItemSlot::BombsMaxAmmo, bombBag);
}

void AddHeartContainer()
{
	__int8 hearts = DolphinRead8(WWItemSlot::HeartContainers);
	if (hearts < 80)
		hearts += 4;
	DolphinWrite8(WWItemSlot::HeartContainers, hearts);
}

void GiveHurricaneSpin()
{
	DolphinWrite8(WWItemSlot::HurricaneSpinSlot, 1);
}

// Adds an item to the first empty mailbag slot if the mailbag does not contain it already
void AddToMail(WWItem mail)
{
	__int8 bag[9];
	memset(&bag, WWItem::NoItem, 9);
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + MailBagStart), &bag, 9, nullptr);

	// Check to avoid duplicates
	for (int i = 0; i < 9; i++)
	{
		if (bag[i] == mail)
			return;
	}

	// Need to account for a full mailbag

	// Add to first empty slot
	for (int i = 0; i < 9; i++)
	{
		if (bag[i] == WWItem::NoItem)
		{
			DolphinWrite8(MailBagStart + i, mail);
			return;
		}

	}
}