#include "Inventory.h"

#define BASE_OFFSET 0x7FFF0000

WWInventory GetInventoryFromProcess(HANDLE h)
{
	WWInventory temp;
	const size_t captureLength = 21;
	__int8 p1Buffer[captureLength];
	__int8 p2Buffer[177];
	__int8 equipBuffer[3];
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + ItemInfoStart), &p1Buffer, sizeof(p1Buffer), nullptr);
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWItemSlot::SwordSlot), &p2Buffer, sizeof(p2Buffer), nullptr);
	ReadProcessMemory(h, (LPVOID)(BASE_OFFSET + WWEquipSlot::X_BUTTON), &equipBuffer, sizeof(equipBuffer), nullptr);

	int i;
	int c;
	int numStates;
	__int8 curState;
	for (i = 0; i < 21; i++)
	{
		numStates = InventoryMap[i].states.size();
		for (c = 0; c < numStates; c++)
		{
			curState = InventoryMap[i].states[c].item;
			if (p1Buffer[i] == curState)
			{
				temp.itemStates[i] = c;
			}
		}
	}

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

	while (running)
	{
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
	};
	
	

}