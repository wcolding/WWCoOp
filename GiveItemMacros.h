#pragma once
#include "DolphinHook.h"

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

