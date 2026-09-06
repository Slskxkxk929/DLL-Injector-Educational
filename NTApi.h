#pragma once
#include <Windows.h>

// Структуры
typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PVOID ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;
	PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _PS_ATTRIBUTE_LIST {
	SIZE_T TotalLength;
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// Типы функций
typedef NTSTATUS(NTAPI* pNtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);

typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(HANDLE, PVOID, PVOID, ULONG, PULONG);

typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(HANDLE, PVOID, PVOID, ULONG, PULONG);

typedef NTSTATUS(NTAPI* pNtCreateThreadEx)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PPS_ATTRIBUTE_LIST);

typedef NTSTATUS(NTAPI* pLdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);

// Объявления переменных
extern pNtOpenProcess NtOpenProcess;

extern pNtWriteVirtualMemory NtWriteVirtualMemory;

extern pNtReadVirtualMemory NtReadVirtualMemory;

extern pNtCreateThreadEx NtCreateThreadEx;

extern pLdrLoadDll LdrLoadDll;