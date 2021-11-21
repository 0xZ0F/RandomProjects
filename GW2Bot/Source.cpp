/*

AFK Farming bot for GW2 engineer class. Summons turrets automatically. Better to use in a VM since changing the active window is required to send input.
Keybinds are set in Keybinds.h.
There are 2 types of turrets used. Two have 20 second cooldowns upon expiration (5 minutes), the other 2 have 40 second cooldowns.
Two threads actively manage 2 turrets each because of the different cooldowns. However, each thread uses the same function.

*/

#include <Windows.h>
#include <map>
#include <dinput.h>
#include <chrono>
#include <thread>
#include <future>
#include "Log.h"
#include "Keybinds.h"

std::unique_ptr<std::thread> WorkerTwentySeconds = std::make_unique<std::thread>();
std::unique_ptr<std::thread> WorkerFourtySeconds = std::make_unique<std::thread>();
bool IsWorkerTwentySeconds = false;

void SendKey(SHORT key) {
	UINT mappedkey;
	INPUT input = { 0 };

	// Have to map key for DirectX games.
	mappedkey = MapVirtualKey(LOBYTE(key), 0);
	input.type = INPUT_KEYBOARD;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	input.ki.wScan = mappedkey;

	SendInput(1, &input, sizeof(input));
	Sleep(50);
	input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(input));
}

void PickupTurrets() {
	HWND hWindow = FindWindow(NULL, L"Guild Wars 2");
	HWND hOriginalWindow = GetForegroundWindow();

	if (!hWindow) {
		Log("Failed to find window.");
		return;
	}

	Log("Picking up turrets.");

	SetForegroundWindow(hWindow);
	Sleep(150);

	SendKey(VK_OEM_6);
	Sleep(500);
	SendKey(VK_OEM_6);
	Sleep(500);
	SendKey(VK_OEM_6);
	Sleep(500);
	SendKey(VK_OEM_6);
	Sleep(500);

	if (hOriginalWindow) {
		SetForegroundWindow(hOriginalWindow);
	}
}

// Every five minutes + n seconds place turrets specified in the array:
void Worker(int additionalSeconds, int turrets[], int size) {
	HWND hWindow = FindWindow(NULL, L"Guild Wars 2");
	HWND hOriginalWindow = GetForegroundWindow();

	if (!hWindow) {
		Log("Failed to find window.");
		return;
	}

	Log("Worker ", std::this_thread::get_id(), " started.");

	do {
		// Prevent the two threads from placing turrets at the same time:
		if (WorkerTwentySeconds->get_id() == std::this_thread::get_id()) {
			IsWorkerTwentySeconds = true;
		}else {
			while (IsWorkerTwentySeconds) {
				Sleep(100);
			}
		}

		Log("Worker ", std::this_thread::get_id(), " placing turrets (delayed by ", additionalSeconds, " seconds).");

		SetForegroundWindow(hWindow);
		Sleep(250);

		for (size_t x = 0; x < size; x++) {
			SendKey(turrets[x]);
			Sleep(1000);
		}

		Sleep(150);

		if (hOriginalWindow) { SetForegroundWindow(hOriginalWindow); }

		Log("Worker ", std::this_thread::get_id(), " sleeping for 5 minutes ", additionalSeconds, " seconds.");
		IsWorkerTwentySeconds = false;
		std::this_thread::sleep_for(std::chrono::minutes(5) + std::chrono::seconds(additionalSeconds));
	} while (true);
}

int main() {
	Log("F8 - START");
	Log("F9 - STOP");
	Log("F10 - Pickup turrets.");
	int TwentySecondTurrets[2] = {0};
	int FourtySecondTurrets[2] = {0};

	do {
		if (GetAsyncKeyState(VK_F8) & 1) {
			if (!WorkerTwentySeconds->native_handle()) {
				TwentySecondTurrets[0] = HEALING_TURRET;
				TwentySecondTurrets[1] = RIFLE_TURRET;
				*WorkerTwentySeconds = std::thread(Worker, 20, TwentySecondTurrets, 2);
				Sleep(100);
			}
			if (!WorkerFourtySeconds->native_handle()) {
				FourtySecondTurrets[0] = ROCKET_TURRET;
				FourtySecondTurrets[1] = THUMPER_TURRET;
				*WorkerFourtySeconds = std::thread(Worker, 40, FourtySecondTurrets, 2);
			}				
		}		
		else if (GetAsyncKeyState(VK_F10) & 1) {
			PickupTurrets();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!GetAsyncKeyState(VK_F9));
	
	Log("Bot Finished");
	
	return 0;
}