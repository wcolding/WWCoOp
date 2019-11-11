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

	TownFlower		=	0x8C,
	SeaFlower		=	0x8D,
	ExoticFlower	=	0x8E,
	HerosFlag		=	0x8F,
	BigCatchFlag	=	0x90,
	BigSaleFlag		=	0x91,
	Pinwheel		=	0x92,
	SickleMoonFlag	=	0x93,
	SkullTowerIdol	=	0x94,
	FountainIdol	=	0x95,
	PostmanStatue	=	0x96,
	ShopGuruStatue	=	0x97,

	// Unsure where/how these are stored
	HurricaneSpin	=	0xAA,
	TingleStatue1	=	0xA3, // Dragon
	TingleStatue2	=	0xA4, // Forbidden
	TingleStatue3	=	0xA5, // Goddess
	TingleStatue4	=	0xA6, // Earth
	TingleStatue5	=	0xA7, // Wind

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

	MailBagStart	=	0x003C4C8E,

	HeartContainers =	0x003C4C09,
	CurrentHearts	=	0x003C4C0B
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

typedef enum : unsigned int
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
} WWChartMaskA;

typedef enum : unsigned int
{
	TriforceChart1 = 1,
	TriforceChart2 = 1 << 1,
	TriforceChart3 = 1 << 2,
	TriforceChart4 = 1 << 3,
	TriforceChart5 = 1 << 4,
	TriforceChart6 = 1 << 5,
	TriforceChart7 = 1 << 6,
	TriforceChart8 = 1 << 7,

	TreasureChart11 = 1 << 8,
	TreasureChart15 = 1 << 9,
	TreasureChart30 = 1 << 10,
	TreasureChart20 = 1 << 11,
	TreasureChart5	= 1 << 12,
	TreasureChart23 = 1 << 13,
	TreasureChart31 = 1 << 14,
	TreasureChart33 = 1 << 15,

	TreasureChart2	= 1 << 16,
	TreasureChart38 = 1 << 17,
	TreasureChart39 = 1 << 18,
	TreasureChart24 = 1 << 19,
	TreasureChart6	= 1 << 20,
	TreasureChart12 = 1 << 21,
	TreasureChart35 = 1 << 22,
	TreasureChart1	= 1 << 23,

	TreasureChart29 = 1 << 24,
	TreasureChart34 = 1 << 25,
	TreasureChart18 = 1 << 26,
	TreasureChart16 = 1 << 27,
	TreasureChart28 = 1 << 28,
	TreasureChart4	= 1 << 29,
	TreasureChart3	= 1 << 30,
	TreasureChart40 = 1 << 31
} WWChartMaskB;

struct WWChartState
{
	WWChartMaskA a = (WWChartMaskA)0;
	WWChartMaskB b = (WWChartMaskB)0;

	__int64 GetState()
	{
		return ((__int64)b << 32) + ((__int64)a);
	}

	void SetState(__int64 mask)
	{
		int _a = (int)mask;
		int _b = (int)(mask >> 32);
		a = (WWChartMaskA)_a;
		b = (WWChartMaskB)_b;
	}

	void AddChart(WWChartMaskA chart)
	{
		a = (WWChartMaskA)(a | chart);
	}

	void AddChart(WWChartMaskB chart)
	{
		b = (WWChartMaskB)(b | chart);
	}

	bool HasChart(WWChartMaskA chart)
	{
		return ((a & chart) != 0);
	}

	bool HasChart(WWChartMaskB chart)
	{
		return ((b & chart) != 0);
	}
};

WWChartState GetChartsFromBuffer(char(&buffer)[8])
{
	WWChartState cs;
	char flipped[8];
	memset(&flipped, 0, 8);
	for (int i = 0; i < 8; i++)
	{
		flipped[i] = buffer[7 - i];
	}
	__int64 mask;
	memcpy(&mask, &flipped, sizeof(mask));
	cs.SetState(mask);
	return cs;
}

void SetBufferFromChartState(char(&buffer)[8], WWChartState cs)
{
	char flipped[8];
	memset(&buffer, 0, 8);
	memset(&flipped, 0, 8);
	__int64 mask = cs.GetState();
	memcpy(&flipped, &mask, 8);
	for (int i = 0; i < 8; i++)
	{
		buffer[i] = flipped[7 - i];
	}
}

struct WWBagState
{
	__int8 slots[8];

	WWBagState()
	{
		memset(&slots, 0xFF, sizeof(slots));
	}

	void AddItem(WWItem item)
	{
		for (int i = 0; i < sizeof(slots); i++)
		{
			if (slots[i] == 0xFF)
				slots[i] == item;
		}
	}

	bool HasItem(WWItem item)
	{
		for (int i = 0; i < sizeof(slots); i++)
		{
			if (slots[i] == item)
				return true;
		}

		return false;
	}
};

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
