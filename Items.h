#pragma once
#include "WWCore.h"

typedef enum : __int8
{
	Sword1	=	0x38, // Hero's Sword
	Sword2	=	0x39, // Master Sword
	Sword3	=	0x3A, // Half charged
	Sword4	=	0x3E, // Fully charged

	Shield1	=	0x3B, // Hero's Shield
	Shield2 =	0x3C, // Mirror Shield
	
	Bow1	=	0x27, 
	Bow2	=	0x35, // Fire and Ice arrows
	Bow3	=	0x36, // Light arrows

	GrapplingHook	=	0x25,
	DekuLeaf		=	0x34,
	Boomerang		=	0x2D,
	Bombs			=	0x31, 
	Hammer			=	0x33,
	Hookshot		=	0x2F,

	Sail	=	0x78,
	WW		=	0x22,

	SpoilsBag	=	0x24, 
	BaitBag		=	0x2C,
	MailBag		=	0x30,

	Bracelet	=	0x28,
	Boots		=	0x29,

	MagicArmor	=	0x2A,
	HerosCharm	=	0x43, 
	TingleTuner	=	0x21, 
	Telecope	=	0x20,
	PictoBox1	=	0x23,
	PictoBox2	=	0x26,

	Bottle		=	0x50,

	i_WindsRequiem		=	0x6D,
	i_BalladofGales		=	0x6E,
	i_CommandMelody		=	0x70,
	i_EarthGodsLyric	=	0x71,
	i_WindGodsAria		=	0x72,
	i_SongofPassing		=	0x6F,

	FathersLetter	=	0x98,
	NotetoMom		=	0x99,
	MaggiesLetter	=	0x9A,
	MoblinsLetter	=	0x9B,
	CabanaDeed		=	0x9C,
	ComplimentaryID	=	0x9D,
	FillupCoupon	=	0x9E,

	PieceOfHeart	=	0x07, // or 3f?

	NoItem			=	-1

} WWItem;


#define INV_OFFSET 0x003C4C44

/* The order of the inventory is as follows, left to right, row by row:
	Starts at	0x003C4C44
	Ends at		0x003C4C58
	Row 1: Telescope, Sail, Wind Waker, Grappling Hook, Spoils Bag, Boomerang, Deku Leaf
	Row 2: Tingle Tuner, Camera, Iron Boots, Magic Armor, Bait Bag, Bow, Bombs
	Row 3: Bottles 1-4, Mail Bag, Hookshot, Skull Hammer

	2nd Page
	
	Icons: Sword, Shield, Bracelets, Key Bag, Mask (equip works)
*/
typedef enum
{
	SwordSlot		=	0x003C4C16,
	SwordIconSlot	=	0x003C4CBC,
	ShieldSlot		=	0x003C4C17,
	ShieldIconSlot	=	0x003C4CBD,
	BraceletSlot	=	0x003C4C18,
	BraceletIconSlot =	0x003C4CBD,
	HerosCharmSlot	=	0x003C4CC0,

	BowMaxAmmo		=	0x003C4C77,
	BombsMaxAmmo	=	0x003C4C78,

	SongsSlot		=	0x003C4CC5,
	TriforceSlot	=	0x003C4CC6,
	
	WalletSlot		=	0x003C4C1A,
	MagicSlot		=	0x003C4C1B,
} WWItemSlot;

typedef enum
{
	WindsRequiem	=	0x01,
	BalladofGales	=	0x02,
	CommandMelody	=	0x04,
	EarthGodsLyric	=	0x08,
	WindGodsAria	=	0x10,
	SongofPassing	=	0x20
} WWSongMask;

typedef enum
{
	X_BUTTON	=	0x003CA7DB,
	Y_BUTTON	=	0x003CA7DC,
	Z_BUTTON	=	0x003CA7DD,
} WWEquipSlot;

typedef enum
{
	Nayru	=	0x01,
	Din		=	0x02,
	Farore	=	0x04
} WWPearlMask;

struct WWItemState
{
	__int8 item;
	string name = "";
};

struct WWItemInfo
{
	DWORD address;
	vector<WWItemState> states;
};



const DWORD ItemInfoStart = INV_OFFSET;
const DWORD ItemInfoEnd = WWItemSlot::TriforceSlot;
