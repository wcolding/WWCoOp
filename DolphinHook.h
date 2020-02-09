#pragma once
#include "Inventory.h"

#define BASE_OFFSET (unsigned int)0x7FFF0000

HANDLE DolphinHandle = NULL;

void DolphinWrite8(unsigned int offset, __int8 value)
{
	if (DolphinHandle == NULL)
		return;

	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + offset), &value, 1, nullptr);
}

void DolphinWrite16(unsigned int offset, __int16 value)
{
	if (DolphinHandle == NULL)
		return;

	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + offset), &value, 2, nullptr);
}

__int8 DolphinRead8(unsigned int offset)
{
	if (DolphinHandle == NULL)
		return 0;

	__int8 value;
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + offset), &value, 1, nullptr);
	return value;
}

WWInventory GetInventoryFromProcess()
{
	WWInventory temp;

	if (DolphinHandle == NULL)
	{
		std::cout << "Need to call HookDolphinProcess() first!" << std::endl;
		return temp;
	}
	
	__int8 p1Buffer[21];
	__int8 p2Buffer[WWItemSlot::ChartSlot + 8 - WWItemSlot::HeartContainers];
	__int8 equipBuffer[3];
	__int8 mailBuffer[8];
	__int8 statueBuffer;
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + ItemInfoStart), &p1Buffer, sizeof(p1Buffer), nullptr);
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::HeartContainers), &p2Buffer, sizeof(p2Buffer), nullptr);
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), nullptr);
	memcpy(&mailBuffer, &p2Buffer[WWItemSlot::MailBagStart - WWItemSlot::HeartContainers], sizeof(mailBuffer));
	statueBuffer = DolphinRead8(WWItemSlot::StatuesSlot);

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

	temp.itemStates[26] = GetItemState(26, p2Buffer[WWItemSlot::SwordSlot - WWItemSlot::HeartContainers]); // Sword
	temp.itemStates[27] = GetItemState(27, p2Buffer[WWItemSlot::SwordIconSlot - WWItemSlot::HeartContainers]); // Sword Icon
	temp.itemStates[28] = GetItemState(28, p2Buffer[WWItemSlot::ShieldSlot - WWItemSlot::HeartContainers]); // Shield
	temp.itemStates[29] = GetItemState(29, p2Buffer[WWItemSlot::ShieldIconSlot - WWItemSlot::HeartContainers]); // Shield Icon
	temp.itemStates[30] = GetItemState(30, p2Buffer[WWItemSlot::BraceletSlot - WWItemSlot::HeartContainers]); // Bracelet
	temp.itemStates[31] = GetItemState(31, p2Buffer[WWItemSlot::BraceletIconSlot - WWItemSlot::HeartContainers]); // Bracelet Icon
	temp.itemStates[32] = GetItemState(32, p2Buffer[WWItemSlot::HerosCharmSlot - WWItemSlot::HeartContainers]); // Hero's Charm

	temp.Wallet = p2Buffer[WWItemSlot::WalletSlot - WWItemSlot::HeartContainers];
	temp.Magic = p2Buffer[WWItemSlot::MagicSlot - WWItemSlot::HeartContainers];
	temp.Quiver = p2Buffer[WWItemSlot::BowMaxAmmo - WWItemSlot::HeartContainers];
	temp.BombBag = p2Buffer[WWItemSlot::BombsMaxAmmo - WWItemSlot::HeartContainers];

	temp.Hearts = p2Buffer[0];
	temp.Songs = p2Buffer[WWItemSlot::SongsSlot - WWItemSlot::HeartContainers];
	temp.Triforce = p2Buffer[WWItemSlot::TriforceSlot - WWItemSlot::HeartContainers];
	temp.Pearls = p2Buffer[WWItemSlot::PearlSlot - WWItemSlot::HeartContainers];
	temp.Statues = statueBuffer;
	
	char chartBuffer[8];
	memcpy(&chartBuffer, &p2Buffer[WWItemSlot::ChartSlot - WWItemSlot::HeartContainers], sizeof(chartBuffer));
	temp.Charts = GetChartsFromBuffer(chartBuffer);

	temp.XButtonEquip = equipBuffer[0];
	temp.YButtonEquip = equipBuffer[1];
	temp.ZButtonEquip = equipBuffer[2];

	return temp;
}

void WriteMappedState(int index, int state)
{
	if (DolphinHandle == NULL)
	{
		std::cout << "Need to call HookDolphinProcess() first!" << std::endl;
		return;
	}

	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + InventoryMap[index].address), &InventoryMap[index].states[state].item, 1, nullptr);
}

void StoreInventoryToProcess(WWInventory patch)
{
	if (DolphinHandle == NULL)
	{
		std::cout << "Need to call HookDolphinProcess() first!" << std::endl;
		return;
	}

	int i, c;
	__int8 equipBuffer[3];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), nullptr);

	for (i = 0; i < 21; i++)
	{
		// Only write changed values
		if (patch.itemStates[i] != 0)
		{
			// Don't overwrite acquired bottles with blanks
			if (13 < i < 18)
			{
				__int8 bottleValue = 0xFF;
				ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + InventoryMap[i].address), &bottleValue, 1, nullptr);
				if (bottleValue == WWItem::NoItem)
					WriteMappedState(i, patch.itemStates[i]);
			}
			else
			{
				WriteMappedState(i, patch.itemStates[i]);

				if (patch.itemStates[i] > 1)
				{
					for (c = 0; c < 3; c++)
					{
						// Update equip buttons if this is an upgrade of an equipped item
						if (equipBuffer[c] == InventoryMap[i].states[patch.itemStates[i] - 1].item) // only works if progression is 1 state higher. normal arrows -> light arrows does not trigger
						{
							WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON + c), &InventoryMap[i].states[patch.itemStates[i]].item, 1, nullptr);
						}
					}
				}
			}
		}

	}

	// Only write new mail to empty slots
	__int8 mailBuffer[8];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::MailBagStart), &mailBuffer, sizeof(mailBuffer), nullptr);

	for (i = 21; i < 26; i++)
	{
		if (patch.itemStates[i] != 0)
		{
			for (c = 0; c < sizeof(mailBuffer); c++)
			{
				if (mailBuffer[c] == WWItem::NoItem)
				{
					WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::MailBagStart + c), &InventoryMap[i].states[1].item, 1, nullptr);
					break;
				}
			}
		}
	}

	for (i = 26; i < sizeof(patch.itemStates); i++)
	{
		if (patch.itemStates[i] != 0)
			WriteMappedState(i, patch.itemStates[i]);
	}

	if (patch.Wallet > 0)
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::WalletSlot), &patch.Wallet, 1, nullptr);
	if (patch.Magic > 0)
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::MagicSlot), &patch.Magic, 1, nullptr);
	if (patch.Quiver > 0)
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::BowMaxAmmo), &patch.Quiver, 1, nullptr);
	if (patch.BombBag > 0)
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::BombsMaxAmmo), &patch.BombBag, 1, nullptr);

	// Hearts?

	if (patch.itemStates[sizeof(patch.itemStates) - 1] > 0)
		WriteMappedState(sizeof(patch.itemStates) - 1, 1); // Hero's Charm will only exist in state 0 or 1 (as state 2 is when it is equipped)

	// Reread triforce and songs and OR them with patch value
	__int8 bitMaskBuffer[3];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::SongsSlot), &bitMaskBuffer, sizeof(bitMaskBuffer), nullptr);
	bitMaskBuffer[0] = bitMaskBuffer[0] | patch.Songs;
	bitMaskBuffer[1] = bitMaskBuffer[1] | patch.Triforce;
	bitMaskBuffer[2] = bitMaskBuffer[2] | patch.Pearls;
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::SongsSlot), &bitMaskBuffer, sizeof(bitMaskBuffer), nullptr);
	__int8 statueBuffer;
	statueBuffer = DolphinRead8(WWItemSlot::StatuesSlot);
	statueBuffer |= patch.Statues;
	DolphinWrite8(WWItemSlot::StatuesSlot, statueBuffer);
	
	// Chart malarky
	__int8 chartBuffer[8];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::ChartSlot), &chartBuffer, sizeof(chartBuffer), nullptr);
	WWChartState currentChartState = GetChartsFromBuffer(chartBuffer);
	__int64 chartMask = currentChartState.GetState() | patch.Charts.GetState();
	currentChartState.SetState(chartMask);
	SetBufferFromChartState(chartBuffer, currentChartState);
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::ChartSlot), &chartBuffer, sizeof(chartBuffer), nullptr);
}

string GetCurrentStage()
{
	string s;
	if (DolphinHandle == NULL)
	{
		std::cout << "Need to call HookDolphinProcess() first!" << std::endl;
		return s;
	}

	char buffer[8];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + MAP_OFFSET), &buffer, sizeof(buffer), nullptr);
	int i;
	
	for (i = 0; i < sizeof(buffer); i++)
	{
		if (buffer[i] != 0)
			s.push_back(buffer[i]);
		else
			return s;
	}
	return s;
}

string GetPlayerName()
{
	string s;
	if (DolphinHandle == NULL)
	{
		std::cout << "Need to call HookDolphinProcess() first!" << std::endl;
		return s;
	}

	char buffer[8];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + NAME_OFFSET), &buffer, sizeof(buffer), nullptr);
	int i;

	for (i = 0; i < sizeof(buffer); i++)
	{
		if (buffer[i] != 0)
			s.push_back(buffer[i]);
		else
			return s;
	}
	return s;
}

int HookDolphinProcess()
{
	HWND window = FindWindowA(NULL, "Dolphin 5.0");
	if (window == NULL)
	{
		std::cout << "Unable to get Dolphin window." << std::endl;
		return -1;
	}
	
	DWORD process;
	GetWindowThreadProcessId(window, &process);
	DolphinHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process);
	if (DolphinHandle == NULL)
	{
		std::cout << "Unable to open Dolphin process." << std::endl;
		return -2;
	}
}