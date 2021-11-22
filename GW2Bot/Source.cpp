/*

AFK Farming bot for GW2 engineer class. Summons turrets automatically. Better to use in a VM since changing the active window is required to send input.
Keybinds are set in Keybinds.h.
There are 2 types of turrets used. Two have 20 second cooldowns upon expiration (5 minutes), the other 2 have 40 second cooldowns.
Two threads actively manage 2 turrets each because of the different cooldowns. However, each thread uses the same function.
Supports multiboxing.
*/

#include <Windows.h>
#include <map>
#include <dinput.h>
#include <chrono>
#include <thread>
#include <future>
#include <vector>
#include "Log.h"
#include "Keybinds.h"

static LPCSTR g_WindowName = "Guild Wars 2";
static std::unique_ptr<std::thread> g_WorkerTwentySeconds = std::make_unique<std::thread>();
static std::unique_ptr<std::thread> g_WorkerFourtySeconds = std::make_unique<std::thread>();
static bool g_IsWorkerTwentySeconds = false;
static std::vector<HWND> g_WindowHandles;

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

BOOL CALLBACK EnumWindowTitles(HWND hwnd, LPARAM lParam) {
	char str[1024] = { 0 };
	GetWindowTextA(hwnd, str, 1024);
	if (strncmp(str, g_WindowName, 1024) == 0) {
		g_WindowHandles.push_back(hwnd);
	}
	return true;
}

// Every five minutes + n seconds place turrets specified in the array:
void Worker(int additionalSeconds, int turrets[], int size) {
	HWND hOriginalWindow = GetForegroundWindow();

	Log("Worker ", std::this_thread::get_id(), " started.");

	do {
		// Prevent the two threads from placing turrets at the same time:
		if (g_WorkerTwentySeconds->get_id() == std::this_thread::get_id()) {
			g_IsWorkerTwentySeconds = true;
		}else {
			while (g_IsWorkerTwentySeconds) {
				Sleep(100);
			}
		}

		for (unsigned short x = 0; x < g_WindowHandles.size(); x++) {
			Log("Worker ", std::this_thread::get_id(), " placing turrets (delayed by ", additionalSeconds, " seconds) for ", g_WindowHandles[x]);

			SetForegroundWindow(g_WindowHandles[x]);
			Sleep(200);

			for (size_t x = 0; x < size; x++) {
				SendKey(turrets[x]);
				Sleep(1000);
			}
		}

		if (hOriginalWindow) { SetForegroundWindow(hOriginalWindow); }

		Log("Worker ", std::this_thread::get_id(), " sleeping for 5 minutes ", additionalSeconds, " seconds.");
		g_IsWorkerTwentySeconds = false;
		std::this_thread::sleep_for(std::chrono::minutes(5) - std::chrono::seconds(size) + std::chrono::seconds(additionalSeconds));
	} while (true);
}

int main() {
	Log("F8 - START");
	Log("F9 - STOP");
	int TwentySecondTurrets[2] = {0};
	int FourtySecondTurrets[2] = {0};

	do {
		if (GetAsyncKeyState(VK_F8) & 1) {
			EnumWindows(EnumWindowTitles, NULL);
			if (g_WindowHandles.size() < 1) {
				Log("Failed to find a window.");
				g_WindowHandles.clear();
				continue;
			}

			if (!g_WorkerTwentySeconds->native_handle()) {
				TwentySecondTurrets[0] = HEALING_TURRET;
				TwentySecondTurrets[1] = RIFLE_TURRET;
				*g_WorkerTwentySeconds = std::thread(Worker, 20, TwentySecondTurrets, 2);
				Sleep(100);
			}
			if (!g_WorkerFourtySeconds->native_handle()) {
				FourtySecondTurrets[0] = ROCKET_TURRET;
				FourtySecondTurrets[1] = THUMPER_TURRET;
				*g_WorkerFourtySeconds = std::thread(Worker, 40, FourtySecondTurrets, 2);
			}				
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!GetAsyncKeyState(VK_F9));
	
	Log("Bot Finished");
	
	return 0;
}