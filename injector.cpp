#include <iostream>   // Для вывода в консоль
#include <string>     // Для std::string
#include <Windows.h>  // WinAPI
#include <TlHelp32.h> // Для снапшотов процессов

// Функция: найти PID процесса по имени
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
	std::string process;

	std::cout << "Введи имя процесса (Диспетчер задач -> Подробности) >> ";
	std::cin >> process;
	std::cout << std::endl;

	std::wstring wprocess(process.begin(), process.end());

	std::cout << " Получаю PID процесса... (1/6)" << std::endl;

	DWORD pid = GetProcessIdByName(wprocess.c_str());

	if (pid == 0) {
		std::cout << "Процесс не найден." << std::endl;
		std::cin.ignore();
		std::cin.get();
		return 1;
	}

	std::cout << " Открываю процесс... (2/6)" << std::endl << std::endl;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (!hProcess) {
		std::cout << "Не удалось открыть процесс." << std::endl;
		std::cin.ignore();
		std::cin.get();
		return 1;
	}

	std::string dllPath;

	std::cout << "Введи путь до DLL (используй / вместо \\) >> ";
	std::cin.ignore();
	std::getline(std::cin, dllPath);
	std::cout << std::endl << std::endl;

	SIZE_T size = dllPath.size() + 1;

	std::cout << " Выделяю память в процессе... (3/6)" << std::endl;

	LPVOID addr = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!addr) {
		std::cout << "Не удалось выделить память." << std::endl;
		CloseHandle(hProcess);
		std::cin.get();
		return 1;
	}

	std::cout << " Записываю путь к DLL... (4/6)" << std::endl;

	WriteProcessMemory(hProcess, addr, dllPath.c_str(), size, NULL);

	std::cout << " Получаю адрес LoadLibraryA... (5/6)" << std::endl;

	HMODULE hModule = GetModuleHandleA("kernel32.dll");
	FARPROC func = GetProcAddress(hModule, "LoadLibraryA");

	std::cout << " Запускаю LoadLibraryA... (6/6)" << std::endl << std::endl;

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)func, addr, 0, NULL);

	if (!hThread) {
		std::cout << "Не удалось создать поток." << std::endl;
	}
	else {
		std::cout << "DLL успешно загружена!" << std::endl;
		CloseHandle(hThread);
	}

	CloseHandle(hProcess);

	std::cout << std::endl << "Нажми Enter для выхода..." << std::endl;
	std::cin.get();
	return 0;
}
