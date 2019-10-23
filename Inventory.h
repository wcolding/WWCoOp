#pragma once
#include "Items.h"

vector<WWItemInfo> InventoryMap =
{
	{INV_OFFSET, {{WWItem::NoItem, ""}, {WWItem::Telecope, "Telescope"}}},
	{INV_OFFSET + 1, {{WWItem::NoItem, ""}, {WWItem::Sail, "Sail"}}},
	{INV_OFFSET + 2, {{WWItem::NoItem, ""}, {WWItem::WW, "Wind Waker"}}},
	{INV_OFFSET + 3, {{WWItem::NoItem, ""}, {WWItem::GrapplingHook, "Grappling Hook"}}},
	{INV_OFFSET + 4, {{WWItem::NoItem, ""}, {WWItem::SpoilsBag, "Spoils Bag"}}},
	{INV_OFFSET + 5, {{WWItem::NoItem, ""}, {WWItem::Boomerang, "Boomerang"}}},
	{INV_OFFSET + 6, {{WWItem::NoItem, ""}, {WWItem::DekuLeaf, "Deku Leaf"}}},

	{INV_OFFSET + 7, {{WWItem::NoItem, ""}, {WWItem::TingleTuner, "Tingle Tuner"}}},
	{INV_OFFSET + 8, {{WWItem::NoItem, ""}, {WWItem::PictoBox1, "Picto Box"}, {WWItem::PictoBox2, "Deluxe Picto Box"}}},
	{INV_OFFSET + 9, {{WWItem::NoItem, ""}, {WWItem::Boots, "Iron Boots"}}},
	{INV_OFFSET + 10, {{WWItem::NoItem, ""}, {WWItem::MagicArmor, "Magic Armor"}}},
	{INV_OFFSET + 11, {{WWItem::NoItem, ""}, {WWItem::BaitBag, "Bait Bag"}}},
	{INV_OFFSET + 12, {{WWItem::NoItem, ""}, {WWItem::Bow1, "Bow"}, {WWItem::Bow2, "Bow (Fire & Ice Arrows)"}, {WWItem::Bow3, "Bow (Light Arrows)"}}},
	{INV_OFFSET + 13, {{WWItem::NoItem, ""}, {WWItem::Bombs, "Bombs"}}},

	// We won't sync bottle contents to start
	{INV_OFFSET + 14, {{WWItem::NoItem, ""}, {WWItem::Bottle, "Bottle 1 (empty)"}}},
	{INV_OFFSET + 15, {{WWItem::NoItem, ""}, {WWItem::Bottle, "Bottle 2 (empty)"}}},
	{INV_OFFSET + 16, {{WWItem::NoItem, ""}, {WWItem::Bottle, "Bottle 3 (empty)"}}},
	{INV_OFFSET + 17, {{WWItem::NoItem, ""}, {WWItem::Bottle, "Bottle 4 (empty)"}}},
	{INV_OFFSET + 18, {{WWItem::NoItem, ""}, {WWItem::MailBag, "Mail Bag"}}},
	{INV_OFFSET + 19, {{WWItem::NoItem, ""}, {WWItem::Hookshot, "Hookshot"}}},
	{INV_OFFSET + 20, {{WWItem::NoItem, ""}, {WWItem::Hammer, "Skull Hammer"}}},

	// Swords, shields and bracelet must have their icon address written to in addition to the item
	{WWItemSlot::SwordSlot, {{WWItem::NoItem, ""}, {WWItem::Sword1, "Hero's Sword"}, {WWItem::Sword2, "Master Sword"}, {WWItem::Sword3, "Master Sword (Half-Charged)"}, {WWItem::Sword4, "Master Sword (Fully-Charged)"}}},
	{WWItemSlot::SwordIconSlot, {{0x00, ""}, {0x01, ""}, {0x02, ""}, {0x04, ""}, {0x08, ""}}},
	{WWItemSlot::ShieldSlot, {{WWItem::NoItem, ""}, {WWItem::Shield1, "Hero's Shield"}, {WWItem::Shield2, "Mirror Shield"}}},
	{WWItemSlot::ShieldIconSlot, {{0x00, ""}, {0x01, ""}, {0x02, ""}}},
	{WWItemSlot::BraceletSlot, {{WWItem::NoItem, ""}, {WWItem::Bracelet, "Power Bracelet"}}},
	{WWItemSlot::BraceletIconSlot, {{0x00, ""}, {0x01, ""}}},

	// We won't sync the equip state of hero's charm but we'll note it
	{WWItemSlot::HerosCharmSlot, {{0x00, ""}, {0x01, "Hero's Charm"}, {0x03, "Hero's Charm (equipped)"}}},

	{WWItemSlot::WalletSlot, {{0x00, "Wallet (200)"}, {0x01, "Wallet (1000)"}, {0x02, "Wallet (5000)"}}},
	{WWItemSlot::MagicSlot, {{0x00, ""}, {0x10, "Magic"}, {0x20, "Double Magic"}}},

	{WWItemSlot::BowMaxAmmo, {{0x1E, "Quiver (30)"}, {0x3C, "Quiver (60)"}, {0x63, "Quiver (99)"}}},
	{WWItemSlot::BombsMaxAmmo, {{0x1E, "Bomb Bag (30)"}, {0x3C, "Bomb Bag (60)"}, {0x63, "Bomb Bag (99)"} }}

};

struct WWInventory
{
	__int8 itemStates[21];
	__int8 Songs;
	__int8 Triforce;
	__int8 BowMaxAmmo; // may be obsolete with state
	__int8 BombsMaxAmmo;
	__int8 Hearts; // still have to find this
	__int8 PiecesofHeart; // still have to find this
	__int8 XButtonEquip;
	__int8 YButtonEquip;
	__int8 ZButtonEquip;
	
	WWInventory()
	{
		memset(&itemStates, 0, sizeof(itemStates));
		Songs = 0;
		Triforce = 0;
		Hearts = 0;
		PiecesofHeart = 0;
		BowMaxAmmo = 30;
		BombsMaxAmmo = 30;
		XButtonEquip, YButtonEquip, ZButtonEquip = WWItem::NoItem;
	}	
};

// We will use a "patch" system to only write changes and not write the entire inventory all the time
WWInventory MakePatch(WWInventory oldInv, WWInventory newInv)
{
	WWInventory patch;
	int i;
	for (i = 0; i < sizeof(patch.itemStates); i++)
	{
		if (newInv.itemStates[i] > oldInv.itemStates[i])
		{
			patch.itemStates[i] = newInv.itemStates[i];

			// If the item being upgraded is on a button, update the equipped item too
			if (oldInv.XButtonEquip == oldInv.itemStates[i])
				patch.XButtonEquip = patch.itemStates[i];
			if (oldInv.YButtonEquip == oldInv.itemStates[i])
				patch.YButtonEquip = patch.itemStates[i];
			if (oldInv.ZButtonEquip == oldInv.itemStates[i])
				patch.ZButtonEquip = patch.itemStates[i];
		}
		else 
		{
			patch.itemStates[i] = 0;
		}
	}

	patch.Songs = oldInv.Songs ^ newInv.Songs;
	patch.Triforce = oldInv.Triforce ^ newInv.Triforce;

	return patch;
}

// This will be used to print synched items to the console
vector<string> GetInventoryStrings(WWInventory inv)
{
	vector<string> builder;
	int i;
	for (i = 0; i < sizeof(inv.itemStates); i++)
	{
		if (inv.itemStates[i] > 0)
			builder.push_back(InventoryMap[i].states[inv.itemStates[i]].name);
	}

	if ((inv.Songs & WWSongMask::WindsRequiem) != 0)
		builder.push_back("Wind's Requiem");
	if ((inv.Songs & WWSongMask::BalladofGales) != 0)
		builder.push_back("Ballad of Gales");
	if ((inv.Songs & WWSongMask::CommandMelody) != 0)
		builder.push_back("Command Melody");
	if ((inv.Songs & WWSongMask::EarthGodsLyric) != 0)
		builder.push_back("Earth God's Lyric");
	if ((inv.Songs & WWSongMask::WindGodsAria) != 0)
		builder.push_back("Wind God's Aria");
	if ((inv.Songs & WWSongMask::SongofPassing) != 0)
		builder.push_back("Song of Passing");

	if ((inv.Triforce & 0x01) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x02) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x04) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x08) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x10) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x20) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x40) != 0)
		builder.push_back("Triforce Shard");
	if ((inv.Triforce & 0x80) != 0)
		builder.push_back("Triforce Shard");

	return builder;
}

