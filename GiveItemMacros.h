#pragma once
#include "DolphinHook.h"

void GiveHookshot()
{
	DolphinWrite8(INV_OFFSET + 19, WWItem::Hookshot);
}

void SetSword(int swordLvl)
{
	switch (swordLvl)
	{
	case 0:
		DolphinWrite8(WWItemSlot::SwordSlot, WWItem::NoItem);
		DolphinWrite8(WWItemSlot::SwordIconSlot, WWItem::NoItem);
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