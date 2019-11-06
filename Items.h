#pragma once
#include "WWCore.h"

typedef enum : __int8
{
	GreenRupee	=	0x01, // 1
	BlueRupee	=	0x02, // 5
	YellowRupee	=	0x03, // 10
	RedRupee	=	0x04, // 20
	PurpleRupee	=	0x05, // 50
	OrangeRupee	=	0x06, // 100
	SilverRupee	=	0x0F, // 200
	
	PieceOfHeart	=	0x07, 
	HeartContainer	=	0x08,
	
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
	i_CommandMelody		=	0x6F,
	i_EarthGodsLyric	=	0x70,
	i_WindGodsAria		=	0x71,
	i_SongofPassing		=	0x72,

	FathersLetter	=	0x98,
	NotetoMom		=	0x99,
	MaggiesLetter	=	0x9A,
	MoblinsLetter	=	0x9B,
	CabanaDeed		=	0x9C,
	ComplimentaryID	=	0x9D,
	FillupCoupon	=	0x9E,

	NoItem			=	-1

} WWItem;


#define INV_OFFSET 0x003C4C44
#define MAP_OFFSET 0x003C9D48

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
	PearlSlot		=	0x003C4CC7,
	ChartSlot		=	0x003C4CDC,
	OpenedChartSlot	=	0x003C4CEC,
	
	WalletSlot		=	0x003C4C1A,
	MagicSlot		=	0x003C4C1B,

	MailBagStart	=	0x003C4C8E
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

typedef enum : __int64
{
	TreasureChart10	= 1,
	TreasureChart14	= 1 << 1,
	TinglesChart	= 1 << 2,
	GhostShipChart	= 1 << 3,
	TreasureChart9	= 1 << 4,
	TreasureChart22	= 1 << 5,
	TreasureChart36	= 1 << 6,
	TreasureChart17	= 1 << 7,
	
	TreasureChart25	= 1 << 8,
	TreasureChart37	= 1 << 9,
	TreasureChart8	= 1 << 10,
	TreasureChart26	= 1 << 11,
	TreasureChart41	= 1 << 12,
	TreasureChart19	= 1 << 13,
	TreasureChart32	= 1 << 14,
	TreasureChart13	= 1 << 15,
	
	TreasureChart21	= 1 << 16,
	TreasureChart27	= 1 << 17,
	TreasureChart7	= 1 << 18,
	IncredibleChart	= 1 << 19,
	OctoChart		= 1 << 20,
	GreatFairyChart	= 1 << 21,
	IsleHeartsChart = 1 << 22,
	SeaHeartsChart	= 1 << 23,
	
	SecretCaveChart	= 1 << 24,
	LightRingChart	= 1 << 25,
	PlatformChart	= 1 << 26,
	BeedlesChart	= 1 << 27,
	SubmarineChart	= 1 << 28,
	// 29-31 are skipped
	
	TriforceChart1 = 1 << 32,
	TriforceChart2 = 1 << 33,
	TriforceChart3 = 1 << 34,
	TriforceChart4 = 1 << 35,
	TriforceChart5 = 1 << 36,
	TriforceChart6 = 1 << 37,
	TriforceChart7 = 1 << 38,
	TriforceChart8 = 1 << 39,
	
	TreasureChart11	= 1 << 40,
	TreasureChart15	= 1 << 41,
	TreasureChart30	= 1 << 42,
	TreasureChart20	= 1 << 43,
	TreasureChart5	= 1 << 44,
	TreasureChart23	= 1 << 45,
	TreasureChart31	= 1 << 46,
	TreasureChart33	= 1 << 47,
	
	TreasureChart2	= 1 << 48,
	TreasureChart38	= 1 << 49,
	TreasureChart39	= 1 << 50,
	TreasureChart24	= 1 << 51,
	TreasureChart6	= 1 << 52,
	TreasureChart12	= 1 << 53,
	TreasureChart35	= 1 << 54,
	TreasureChart1	= 1 << 55,
	
	TreasureChart29	= 1 << 56,
	TreasureChart34	= 1 << 57,
	TreasureChart18	= 1 << 58,
	TreasureChart16	= 1 << 59,
	TreasureChart28	= 1 << 60,
	TreasureChart4	= 1 << 61,
	TreasureChart3	= 1 << 62,
	TreasureChart40	= 1 << 63,
} WWChartMask;

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
