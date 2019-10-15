#include <iostream>
#include <Windows.h>
#include <string>

#include "Inventory.h"

#define BASE_OFFSET 0x7FFF0000

WWInventory GetInventoryFromProcess(HANDLE h)
{
	WWInventory temp;
	const size_t captureLength = 21;
	__int8 buffer[captureLength];
	__int8 equipBuffer[3];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + ItemInfoStart), &buffer, captureLength, 0);
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), 0);

	int i;
	int c;
	int numStates;
	__int8 curState;
	for (i = 0; i < 21; i++)
	{
		//cout << hex << "0x" << (int)buffer[i] << " ";
		numStates = InventoryMap[i].states.capacity();
		for (c = 0; c < numStates; c++)
		{
			curState = InventoryMap[i].states[c].item;
			if (buffer[i] == curState)
			{
				temp.itemStates[i] = c;
			}
		}
	}
	//cout << endl;
	/*
	temp.Songs = buffer[WWItemSlot::SongsSlot - ItemInfoStart - 1];
	temp.Triforce = buffer[WWItemSlot::TriforceSlot - ItemInfoStart - 1];
	temp.BowMaxAmmo = buffer[WWItemSlot::BowMaxAmmo - ItemInfoStart - 1];
	temp.BombsMaxAmmo = buffer[WWItemSlot::BombsMaxAmmo - ItemInfoStart - 1];
	*/
	temp.XButtonEquip = equipBuffer[0];
	temp.YButtonEquip = equipBuffer[1];
	temp.ZButtonEquip = equipBuffer[2];

	return temp;
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

	
	
	WWInventory inv_master, inv_swap;

	inv_swap = GetInventoryFromProcess(handle);
	
	
	int i;
	for (i = 0; i < sizeof(inv_swap.itemStates); i++)
	{
		cout << (int)inv_swap.itemStates[i] << " ";
	}
	cout << endl;
	

	while (!running)
	{
		//ReadProcessMemory(handle, (LPVOID)(baseOffset + INV_OFFSET), &inv_swap, 21, 0);


		Sleep(1000);
	};
	
	//handle

}