#include <Windows.h>
#include <map>
#include <dinput.h>
#include <chrono>
#include <thread>
#include <future>
#include "Log.h"
#include "Keybinds.h"

std::unique_ptr<std::thread> WorkerThread = std::make_unique<std::thread>();
bool g_IsWorkerRunning = false;

void SendKey(SHORT key) {
	UINT mappedkey;
	INPUT input = { 0 };

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

	if (hOriginalWindow) {
		SetForegroundWindow(hOriginalWindow);
	}
}

void InitialTurrets() {
	HWND hWindow = FindWindow(NULL, L"Guild Wars 2");
	HWND hOriginalWindow = GetForegroundWindow();

	if (!hWindow) {
		Log("Failed to find window.");
		return;
	}
	Log("Placing primary turrets.");

	SetForegroundWindow(hWindow);
	Sleep(250);

	SendKey(HEALING_TURRET);
	Sleep(750);
	SendKey(TURRET_1);
	Sleep(500);
	SendKey(TURRET_2);
	Sleep(500);
	SendKey(THUMPER_TURRET);
	Sleep(50);

	if (hOriginalWindow) { SetForegroundWindow(hOriginalWindow); }
}

// Every five minutes place turrets using a seperate thread:
void Worker() {
	HWND hWindow = FindWindow(NULL, L"Guild Wars 2");
	HWND hOriginalWindow = GetForegroundWindow();

	if (!hWindow) {
		Log("Failed to find window.");
		return;
	}

	Log("Worker started on thread ", WorkerThread->get_id());

	do {
		g_IsWorkerRunning = true;
		
		Log("Sleeping for 5 minutes.");
		std::this_thread::sleep_for(std::chrono::minutes(5) + std::chrono::seconds(20));

		Log("Placing primary turrets.");

		SetForegroundWindow(hWindow);
		Sleep(250);

		SendKey(HEALING_TURRET);
		Sleep(1000);
		SendKey(TURRET_1);

		if (hOriginalWindow) {SetForegroundWindow(hOriginalWindow);}
		Sleep(10000);
		SetForegroundWindow(hWindow);
		Sleep(250);
		SendKey(TURRET_2);

		if (hOriginalWindow) { SetForegroundWindow(hOriginalWindow); }
		Sleep(10000);
		SetForegroundWindow(hWindow);
		Sleep(250);
		SendKey(THUMPER_TURRET);
		Sleep(150);

		if (hOriginalWindow) { SetForegroundWindow(hOriginalWindow); }		
	} while (true);
	g_IsWorkerRunning = false;
}

int main() {
	Log("F8 - START");
	Log("F9 - STOP");
	Log("F10 - Pickup turrets.");

	do {
		if (GetAsyncKeyState(VK_F8) & 1) {
			if (WorkerThread->native_handle()) {
				Log("Thread alreaty created.");
				continue;
			}
			else if (g_IsWorkerRunning) {
				Log("Worker already running.");
				continue;
			}
			InitialTurrets();
			*WorkerThread = std::thread(Worker);
		}		
		else if (GetAsyncKeyState(VK_F10) & 1) {
			PickupTurrets();
		}
	} while (!GetAsyncKeyState(VK_F9));
	
	Log("Bot Finished");
	
	return 0;
}