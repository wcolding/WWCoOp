#include "Inventory.h"

#define BASE_OFFSET 0x7FFF0000

WWInventory GetInventoryFromProcess(HANDLE h)
{
	WWInventory temp;
	__int8 p1Buffer[21];
	__int8 p2Buffer[177];
	__int8 equipBuffer[3];
	__int8 mailBuffer[8];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + ItemInfoStart), &p1Buffer, sizeof(p1Buffer), nullptr);
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::SwordSlot), &p2Buffer, sizeof(p2Buffer), nullptr);
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), nullptr);
	memcpy(&mailBuffer, &p2Buffer[WWItemSlot::MailBagStart - WWItemSlot::SwordSlot], sizeof(mailBuffer));

	int i;
	int c;
	for (i = 0; i < 21; i++)
		temp.itemStates[i] = GetItemState(i, p1Buffer[i]);

	// Mail bag is arranged in order of collection.
	// To avoid dealing with this we'll force storage to conform to the InventoryMap order
	for (i = 0; i < sizeof(mailBuffer); i++)
	{
		if (mailBuffer[i] != WWItem::NoItem)
		{
			for (c = 21; c < 26; c++)
			{
				if (InventoryMap[c].states[1].item == mailBuffer[i])
					temp.itemStates[c] = 1; // Item state 1 is the piece of mail
			}
		}
	}

	temp.itemStates[26] = GetItemState(26, p2Buffer[0]); // Sword
	temp.itemStates[27] = GetItemState(27, p2Buffer[WWItemSlot::SwordIconSlot - WWItemSlot::SwordSlot]); // Sword Icon
	temp.itemStates[28] = GetItemState(28, p2Buffer[WWItemSlot::ShieldSlot - WWItemSlot::SwordSlot]); // Shield
	temp.itemStates[29] = GetItemState(29, p2Buffer[WWItemSlot::ShieldIconSlot - WWItemSlot::SwordSlot]); // Shield Icon
	temp.itemStates[30] = GetItemState(30, p2Buffer[WWItemSlot::BraceletSlot - WWItemSlot::SwordSlot]); // Bracelet
	temp.itemStates[31] = GetItemState(31, p2Buffer[WWItemSlot::BraceletIconSlot - WWItemSlot::SwordSlot]); // Bracelet Icon
	temp.itemStates[32] = GetItemState(32, p2Buffer[WWItemSlot::HerosCharmSlot - WWItemSlot::SwordSlot]); // Hero's Charm

	temp.itemStates[33] = GetItemState(33, p2Buffer[WWItemSlot::WalletSlot - WWItemSlot::SwordSlot]); // Wallet
	temp.itemStates[34] = GetItemState(34, p2Buffer[WWItemSlot::MagicSlot - WWItemSlot::SwordSlot]); // Magic
	temp.itemStates[35] = GetItemState(35, p2Buffer[WWItemSlot::BowMaxAmmo - WWItemSlot::SwordSlot]); // Bow Capacity
	temp.itemStates[36] = GetItemState(36, p2Buffer[WWItemSlot::BombsMaxAmmo - WWItemSlot::SwordSlot]); // Bomb Capacity

	temp.Songs = p2Buffer[WWItemSlot::SongsSlot - WWItemSlot::SwordSlot];
	temp.Triforce = p2Buffer[WWItemSlot::TriforceSlot - WWItemSlot::SwordSlot];
	/*temp.BowMaxAmmo = buffer[WWItemSlot::BowMaxAmmo - ItemInfoStart - 1];
	temp.BombsMaxAmmo = buffer[WWItemSlot::BombsMaxAmmo - ItemInfoStart - 1];
	*/
	temp.XButtonEquip = equipBuffer[0];
	temp.YButtonEquip = equipBuffer[1];
	temp.ZButtonEquip = equipBuffer[2];

	return temp;
}

void WriteMappedState(HANDLE h, int index, int state)
{
	WriteProcessMemory(h, (LPVOID)(BASE_OFFSET + InventoryMap[index].address), &InventoryMap[index].states[state].item, 1, nullptr);
}

void StoreInventoryToProcess(HANDLE h, WWInventory patch)
{
	int i, c;
	__int8 equipBuffer[3]; 
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), nullptr);

	for (i = 0; i < 21; i++)
	{
		// Only write changed values
		if (patch.itemStates[i] != 0) 
		{
			WriteMappedState(h, i, patch.itemStates[i]);


			if (patch.itemStates[i] > 1)
			{
				for (c = 0; c < 3; c++)
				{
					// Update equip buttons if this is an upgrade of an equipped item
					if (equipBuffer[c] == InventoryMap[i].states[patch.itemStates[i] - 1].item)
					{
						WriteProcessMemory(h, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON + c), &InventoryMap[i].states[patch.itemStates[i]].item, 1, nullptr);
					}
				}
			}
		}
			
	}

	// Only write new mail to empty slots
	__int8 mailBuffer[8];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::MailBagStart), &mailBuffer, sizeof(mailBuffer), nullptr);
	
	for (i = 21; i < 26; i++)
	{
		if (patch.itemStates[i] != 0)
		{
			for (c = 0; c < sizeof(mailBuffer); c++)
			{
				if (mailBuffer[c] == WWItem::NoItem)
				{
					WriteProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::MailBagStart + c), &InventoryMap[i].states[1].item, 1, nullptr);
					break;
				}
			}
		}
	}

	for (i = 26; i < 37; i++)
	{
		if (patch.itemStates[i] != 0)
			WriteMappedState(h, i, patch.itemStates[i]);
	}

	// Reread triforce and songs and OR them with patch value
	__int8 bitMaskBuffer[2];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::SongsSlot), &bitMaskBuffer, sizeof(bitMaskBuffer), nullptr);
	bitMaskBuffer[0] = bitMaskBuffer[0] | patch.Songs;
	bitMaskBuffer[1] = bitMaskBuffer[1] | patch.Triforce;
	WriteProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::SongsSlot), &bitMaskBuffer, sizeof(bitMaskBuffer), nullptr);


}

string GetCurrentMap(HANDLE h)
{
	char buffer[8];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + MAP_OFFSET), &buffer, sizeof(buffer), nullptr);
	int i;
	string s;
	for (i = 0; i < sizeof(buffer); i++)
	{
		if (buffer[i] != 0)
			s.push_back(buffer[i]);
	}
	return s;
}

int main()
{
	HWND window = FindWindowA(NULL, "Dolphin 5.0");
	if (window == NULL)
	{
		cout << "Unable to get Dolphin window." << endl;
		return -1;
	}

	DWORD process;
	GetWindowThreadProcessId(window, &process);
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process);
	if (handle == NULL)
	{
		cout << "Unable to open Dolphin process." << endl;
		return -2;
	}

	cout << "Opened Dolphin process." << endl;

	bool running = true;

	
	
	WWInventory inv_master, inv_swap, inv_patch;

	inv_swap = GetInventoryFromProcess(handle);
	inv_master = inv_swap;
	string oldMap, curMap;
	curMap = GetCurrentMap(handle);
	oldMap = curMap;

	inv_patch.itemStates[26] = 4;
	inv_patch.itemStates[27] = 4;
	inv_patch.itemStates[28] = 2;
	inv_patch.itemStates[29] = 2;

	inv_patch.Songs = WWSongMask::CommandMelody;
	inv_patch.Triforce = 4;


	StoreInventoryToProcess(handle, inv_patch);

	/*
	while (running)
	{
		curMap = GetCurrentMap(handle);
		
		if (curMap != oldMap)
		{
			cout << "Map change: " << curMap << endl;
			oldMap = curMap;
		}

		inv_swap = GetInventoryFromProcess(handle);
		if (InvChanged(inv_master, inv_swap))
		{
			inv_patch = MakePatch(inv_master, inv_swap);
			inv_master = inv_swap;
			vector<string> log = GetInventoryStrings(inv_patch);
			int i;
			for (i = 0; i < log.size(); i++)
			{
				cout << log[i] << endl;
			}
		}

		Sleep(1000);
	};*/
	
	

}