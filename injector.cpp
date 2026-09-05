#include <iostream>
#include <string>

#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcessIdByName(const wchar_t* processName) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(pe);

	DWORD pid = 0;

	if (Process32FirstW(snapshot, &pe)) {
		do {
			if (_wcsicmp(pe.szExeFile, processName) == 0) {
				pid = pe.th32ProcessID;
				break;
			}
		} while (Process32NextW(snapshot, &pe));
	}

	CloseHandle(snapshot);
	return pid;
}

int main() {

	DWORD pid = GetProcessIdByName(L"notepad.exe");

	if (pid == 0) {
		std::cout << "Процесс не найден." << std::endl;
		return 1;
	}

	// Открываем процесс со всеми правами
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (!hProcess) {
		std::cout << "Не удалось открыть процесс." << std::endl;
		return 1;
	}

	// Записываем путь к нашей DLL
	std::string buffer = "D:\\MyTest\\Test.dll";
	SIZE_T size = buffer.size() + 1;

	// Выделяем память в чужом процессе
	LPVOID addr = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// Пишем данные в память другого процесса
	WriteProcessMemory(hProcess, addr, buffer.c_str(), size, NULL);

	// Находим адрес LoadLibraryA
	HMODULE hModule = GetModuleHandleA("kernel32.dll");
	FARPROC func = GetProcAddress(hModule, "LoadLibraryA");

	// Запускаем LoadLibraryA с путем DLL
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)func, addr, 0, NULL);

	if (!hThread) {
		std::cout << "Не удалось создать поток" << std::endl;
	}
	else {
		std::cout << "DLL успешно загружена" << std::endl;
		CloseHandle(hThread);
	}

	CloseHandle(hProcess);
	return 0;
}