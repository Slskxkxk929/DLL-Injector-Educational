#include <iostream>   // Для вывода в консоль
#include <string>     // Для std::string
#include <Windows.h>  // WinAPI
#include <TlHelp32.h> // Для снапшотов процессов

#include "NTApi.h"    // NTApi

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

	std::cout << "———————————————————————————————————"      << std::endl;
	std::cout << "< Выбери режим инжектирования DLL >"      << std::endl;
	std::cout << "———————————————————————————————————"      << std::endl;
	std::cout << "1. LoadLibraryA - WinApi функция которая" << std::endl;
	std::cout << "загружает DLL в процесс, работает на"     << std::endl;
	std::cout << "«поверхностном» режиме."                  << std::endl;
	std::cout << "———————————————————————————————————"      << std::endl;
	std::cout << "2. NTDLLInjection - Это способ загрузки"  << std::endl;
	std::cout << "DLL в процесс через NT API а не через"    << std::endl;
	std::cout << "стандартные WinApi функции. Работает"     << std::endl;
	std::cout << "на более «низкоуровневом» режиме"         << std::endl;
	std::cout << "чем LoadLibraryA."                        << std::endl;
	std::cout << "———————————————————————————————————"      << std::endl;

	int choice;

	std::cout << "Выбери режим (1 / 2) >> ";
	std::cin >> choice;
	std::cout << "———————————————————————————————————" << std::endl;

	// DLL режим LoadLibraryA 

	if (choice == 1) 
	{
		std::cout << "Запускаю LoadLibraryA.." << std::endl;
		std::cout << "———————————————————————————————————" << std::endl;

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

		/*
		/ Эта функция используется и в LoadLibraryA и в NTDLLInjection так как она не от WinApi и работает через kernel32.dll
	   */
		LPVOID addr = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (!addr) {
			std::cout << "Не удалось выделить память." << std::endl;
			CloseHandle(hProcess);
			std::cin.get();
			return 1;
		}

		std::cout << " Записываю путь к DLL... (4/6)" << std::endl;

		WriteProcessMemory(hProcess, addr, dllPath.c_str(), size, NULL);

		std::cout << " Получаю адрес и запускаю LoadLibraryA... (5/6)" << std::endl;

		HMODULE hModule = GetModuleHandleA("kernel32.dll");
		FARPROC func = GetProcAddress(hModule, "LoadLibraryA");

		std::cout << " Создаю поток... (6/6)" << std::endl << std::endl;

		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)func, addr, 0, NULL);

		if (!hThread) {
			std::cout << "Не удалось создать поток." << std::endl;
		}
		else {
			std::cout << "DLL успешно загружена!" << std::endl;
			CloseHandle(hThread);
		}

		CloseHandle(hProcess);
	}



	// DLL режим NTDLLInjection

	else if (choice == 2)
	{
		std::cout << "Запускаю NTDLLInjection.." << std::endl;
		std::cout << "———————————————————————————————————" << std::endl;

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

		HANDLE hProcess = NULL;

		CLIENT_ID clientId;
		clientId.UniqueProcess = (HANDLE)pid;
		clientId.UniqueThread = NULL;

		OBJECT_ATTRIBUTES oa;
		oa.Length = sizeof(oa);
		oa.RootDirectory = NULL;
		oa.ObjectName = NULL;
		oa.Attributes = 0;
		oa.SecurityDescriptor = NULL;
		oa.SecurityQualityOfService = NULL;

		NTSTATUS status = NtOpenProcess(
			&hProcess,
			PROCESS_ALL_ACCESS,
			&oa,
			&clientId
		);

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

		/*
		/ Эта функция используется и в LoadLibraryA и в NTDLLInjection так как она не от WinApi и работает через kernel32.dll
	   */
		LPVOID addr = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (!addr) {
			std::cout << "Не удалось выделить память." << std::endl;
			CloseHandle(hProcess);
			std::cin.get();
			return 1;
		}

		std::cout << " Записываю путь к DLL... (4/6)" << std::endl;

		NtWriteVirtualMemory(
			hProcess,
			addr,
			(PVOID)dllPath.c_str(),
			size,
			NULL
		);

		std::cout << " Получаю адрес LoadLibraryA... (5/6)" << std::endl;

		HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
		FARPROC func = GetProcAddress(hKernel32, "LoadLibraryA");

		std::cout << " Создаю поток... (6/6)" << std::endl << std::endl;

		HANDLE hThread = NULL;

		NTSTATUS status2 = NtCreateThreadEx(
			&hThread,
			THREAD_ALL_ACCESS,
			NULL,
			hProcess,
			(PVOID)func,
			addr,
			0,
			0,
			0,
			0,
			NULL
		);

		if (!hThread) {
			std::cout << "Не удалось создать поток." << std::endl;
		}
		else {
			std::cout << "DLL успешно загружена!" << std::endl;
			CloseHandle(hThread);
		}

		CloseHandle(hProcess);
	}

	else
	{
		std::cout << "Неверный выбор." << std::endl;
		std::cout << "———————————————————————————————————" << std::endl;
	}

	std::cout << std::endl << "Нажми Enter для выхода..." << std::endl;
	std::cin.get();
	return 0;
}
