#include "NTApi.h"

pNtOpenProcess NtOpenProcess = (pNtOpenProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenProcess");

pNtWriteVirtualMemory NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWriteVirtualMemory");

pNtReadVirtualMemory NtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadVirtualMemory");

pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtCreateThreadEx");

pLdrLoadDll LdrLoadDll = (pLdrLoadDll)GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll");