//=================================================================================================
// Creates a hidden secure virtual desktop
//=================================================================================================

#define WIN32_LEAN_AND_MEAN		// cut the overhead
#include <windows.h>
#include <iostream>
#include <thread>

// desktop name
#define DESKNAME "SideDesk"

// hotkeys for switching betwwen desktop
#define HOTKEY_COMMAND MOD_CONTROL|MOD_ALT|MOD_NOREPEAT
#define HOTKEY_EXIT 1
#define HOTKEY_ENTER 2

#define CERR(e){std::cerr << "[-] Error " << GetLastError() <<": "<< e << std::endl;}

#define PRINT_CONTROLS {std::cout << "\
[*] Controls:\n \
 - Ctrl + Alt + Left : Original Desktop\n\
 - Ctrl + Alt + Right : Secure Desktop\n\
 - Ctrl + C : Exit program" << std::endl;}



void unregister_hotkeys() {
	UnregisterHotKey(NULL, HOTKEY_EXIT);
	UnregisterHotKey(NULL, HOTKEY_ENTER);
}


bool register_hotkeys() {
	// CTRL+ALT+left to exit secure desktop
	bool success = true;
	if (!RegisterHotKey(NULL, HOTKEY_EXIT, HOTKEY_COMMAND, VK_LEFT)) {
		success=false;
		goto exit;
	}
	if (!RegisterHotKey(NULL, HOTKEY_ENTER, HOTKEY_COMMAND, VK_RIGHT)) {
		success=false;
		goto exit;
	}

	exit:
		if (!success)
			unregister_hotkeys();
		return success;
}


void secure_desk_thread_routine(HDESK hSecureDesk) {
	// thread routine to be ran by the hidden desktop
	STARTUPINFO sinfo = { 0 };
	PROCESS_INFORMATION pinfo = { 0 };
	char deskname[] = DESKNAME;
	char explorer_path[MAX_PATH];

	if (!SetThreadDesktop(hSecureDesk)) {
		CERR("Cannot set current thread to secure desktop");
		CloseDesktop(hSecureDesk);
		return;
	}

	// initialize graphical UI on desktop
	sinfo.cb = sizeof(STARTUPINFO);
	sinfo.lpDesktop = deskname;
	ExpandEnvironmentStrings("%windir%\\explorer.exe", explorer_path, MAX_PATH - 1);
	if (! CreateProcess(explorer_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)) {
		CERR("Cannot create process");
		CloseHandle(pinfo.hProcess);
		return;
	}

	WaitForSingleObject(pinfo.hProcess, INFINITE);
	CloseHandle(pinfo.hProcess);
	return ;
}


int main() {
	HDESK hOrigDesk, hSecureDesk;
	std::thread secure_desk_thread;
	MSG msg = {0};
	
	hOrigDesk = GetThreadDesktop(GetCurrentThreadId());
	if (!hOrigDesk) {
		CERR("Cannot get handle to current desktop");
		return 1;
	}

	hSecureDesk = OpenDesktop(DESKNAME, NULL, FALSE, GENERIC_ALL);
	if (! hSecureDesk) {
		hSecureDesk = CreateDesktop(DESKNAME, NULL, NULL, 0, GENERIC_ALL, NULL);
		if (!hSecureDesk) {
			CERR("Cannot create new desktop");
			return 1;
		}
	}

	if (!register_hotkeys()) {
		CERR("Cannot register hotkeys");
		CloseDesktop(hSecureDesk);
		return 1;
	}

	secure_desk_thread = std::thread(secure_desk_thread_routine, hSecureDesk);
	PRINT_CONTROLS;

	// active waiting for hotkey controls
	while (GetMessage(&msg, NULL, 0, 0) != 0) {	// while not WM_QUIT
		if (msg.message != WM_HOTKEY) continue;
		
		switch (msg.wParam) {
		case HOTKEY_EXIT:
			std::cout << "Switching to original desktop" << std::endl;
			if (!SwitchDesktop(hOrigDesk)) {
				CERR("Cannot switch to original desktop");
				CloseDesktop(hSecureDesk);
				return 1;
			}
			break; 
		case HOTKEY_ENTER:
			std::cout << "Switching to secure desktop" << std::endl;
			if (!SwitchDesktop(hSecureDesk)) {
				CERR("Cannot switch to secure desktop");
				CloseDesktop(hSecureDesk);
				return 1;
			}
			break;
		}
	}

	secure_desk_thread.join();

	// cleanup
	CloseDesktop(hSecureDesk);
	CloseDesktop(hOrigDesk);
	unregister_hotkeys();

	return 0;
}